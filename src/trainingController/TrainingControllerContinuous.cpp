#include "TrainingControllerContinuous.h"

#include <filesystem>
#include <chrono>
#include <random>

#include "../TrainingParameters.h"
#include "../TrainingRewarder.h"
#include "../TrainingEncoder.h"
#include "../Environment.h"
#include "../util/Maths.h"
#include "../Models.h"
#include "../TrainingLogger.h"

using namespace PLANS;

//############################ TrainingControllerContinuous ############################

// Random engine for shuffling memory.
std::random_device rd;
std::mt19937 re(rd());

// PUBLIC

TrainingControllerContinuous::TrainingControllerContinuous(TrainingParameters* parameters, Environment* enviroment) : TrainingController(parameters, enviroment), stepsTillAction(), episodesTillCheckpoint() {}

bool TrainingControllerContinuous::onNextScenarioRequired(bool isInit) {
	consoleOut("TrainingControllerContinuous::onNextScenarioRequired");

	if(!isInit) {
		setTrainedEpisodes(getTrainedEpisodes() + 1);

		TrainingController::updateVMEpisodeCount(getTrainedEpisodes());

		// Check if a checkpoint should be created. 
		if(episodesTillCheckpoint != UINT32_MAX && --episodesTillCheckpoint == 0) {
			// Save agents as episode ended. 
			std::string checkpointFilePath;
			saveAgents(getTrainedEpisodes(), checkpointFilePath);
			// Log. 
			TrainingLogger::onCheckpointCreated(checkpointFilePath, getTrainedEpisodes());

			episodesTillCheckpoint = getTrainingParameters()->episodesPerCheckpoint;
		}
	}

	// Check if episode limit reached. If so, terminate training. 
	if(getTrainedEpisodes() >= getTrainingParameters()->maxEpisodes && getTrainingParameters()->maxEpisodes > 0) {
		return true;	// Terminate. Maximum episode count reached. 
	}

	// Reset episode values. 
	setStepsInThisEpisode(-1);
	stepsTillAction = getTrainingParameters()->policyStepLength - 1;
	TrainingController::cleanUpStateDatas();

	// Reset all agents. 
	for(Agent* agent : getAgents()) {
		resetAgentTrainingStep(agent);
	}

	return false;	// Don't terminate, there are episodes to do left. 
}

bool TrainingControllerContinuous::onActionRequired(AGENT_ID agentID, std::vector<torch::Tensor>& output) {
	// Check wether action is allowed (policy step). 
	if(stepsTillAction != 0) {
		return false;	// Not allowed in this step. 
	}

	// Get agent. 
	Agent* agent = getAgents()[agentID];

#if defined(_DEBUG) and defined(MEASURE_TIME)
	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
#endif

	// Get or create state data. 
	const StateData* stateData = TrainingController::getOrCreateStateData(agentID);

	// Save input tensor (state tensor). 
	agent->states.push_back(stateData->inputTensor);

	// Pass inputs into model to produce actor and critic outputs. 
	std::tuple<torch::Tensor, torch::Tensor> outputTuple = agent->model->get()->forward(stateData->inputTensorDevice, true);

#if defined(_DEBUG) and defined(MEASURE_TIME)
	auto finish = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	auto total = finish - start;
	std::cout << "[" + std::to_string(agentID) + "] ran from " + std::to_string(start) + " to " + std::to_string(finish) + "(total of " + std::to_string(total) + ")" << std::endl;
#endif

	// Extract actual output (vector index, sequence, batch). 
	torch::Tensor actorOutput = std::get<0>(outputTuple)[0];
	torch::Tensor criticOutput = std::get<1>(outputTuple)[0];

	// Save action returned by the actor (just the action type at index 0). 
	agent->actions.push_back(torch::full(1, actorOutput[0].item()));

	// Create and save logProb. 
	torch::Tensor logProb = agent->model->get()->logProb(actorOutput[0]);
	agent->logProbs.push_back(logProb);

	// Save value returned by the critic. 
	agent->values.push_back(criticOutput);

	// Copy output tensor to CPU for faster access while decoding. 
	actorOutput = actorOutput.to(torch::kCPU);

	// Process outputs to action. 
	uint32_t delay = 0;
	float action = TrainingEncoder::decodeAction(agent->agentID, actorOutput);

	// Reward agent. Accumulate rewards from occured events, which will be deleted now. 
	rewardAgent(agent, action != UINT8_MAX, getEnvironment());

	// Fill output vector. 
	output.push_back(actorOutput);
	output.push_back(criticOutput);

	return true;
}

void TrainingControllerContinuous::onAgentExecuted(AGENT_ID agentID) {
	// Get agent. 
	Agent* agent = getAgents()[agentID];

	if(agent->rewards.size() < getTrainingParameters()->trainingStepLength) {
		return;	// Not enough rewards. 
	}

	if(agent->totalReward != 0.0) {

#ifdef _DEBUG
		//float f_1 = agent->model->get()->a_lin1_->weight[0][0].item().toFloat();

		uint32_t tmpSize = static_cast<uint32_t>(agent->model->get()->a_lin1_->weight.size(1));

		std::vector<float> befores = std::vector<float>();
		befores.reserve(tmpSize);
		for(uint32_t i = 0; i < tmpSize; i++) {
			befores.push_back(agent->model->get()->a_lin1_->weight[0][i].item().toFloat());
		}
#endif

		// Optimize the agent based on the PPO algorithm. 
		optimizePPO(agent);

#ifdef _DEBUG
		//float f_2 = agent->model->get()->a_lin1_->weight[0][0].item().toFloat();

		std::vector<float> afters = std::vector<float>();
		afters.reserve(tmpSize);
		for(uint32_t i = 0; i < tmpSize; i++) {
			afters.push_back(agent->model->get()->a_lin1_->weight[0][i].item().toFloat());
		}

		std::vector<float> differences = std::vector<float>();
		differences.reserve(tmpSize);
		for(uint32_t i = 0; i < tmpSize; i++) {
			differences.push_back(Maths::abs(befores[i] - afters[i]));
		}
		bool anyDifferences = false;
		for(uint32_t i = 0; i < tmpSize; i++) {
			if(!Maths::compareFloat(differences[i], 0.0F)) {
				anyDifferences = true;
			}
		}
		if(!anyDifferences) {
			consoleOut("TrainingControllerContinuous::onAgentExecuted: No differences detected.");
		}
#endif

	} else {
		consoleOut("TrainingControllerContinuous::onAgentExecuted: Not optimizing agent " + std::to_string(agent->agentID) + ", because his total reward is 0.0.", false);
	}

#ifdef _DEBUG
	// Count actions. 
	std::vector<uint32_t> actionCounts = std::vector<uint32_t>(5);
	for(uint32_t i = 0; i < agent->actions.size(); i++) {
		uint32_t action = static_cast<uint32_t>(agent->actions[i].item().toInt());
		if(action < 4) {
			actionCounts[action]++;
		} else {
			actionCounts[4]++;
		}
	}
#endif

	// Reset agent. 
	resetAgentTrainingStep(agent);

	// Clean up state datas. 
	TrainingController::cleanUpStateDatas();

	//// Clear GPU cache still hold by libtorch. 
	//c10::cuda::CUDACachingAllocator::emptyCache();
}

bool TrainingControllerContinuous::onGameTickPassed() {
	// Update stepsTillAction. 
	if(getTrainingParameters()->policyStepLength > 1) {
		if(stepsTillAction == 0 && getStepsInThisEpisode() > 0) {
			stepsTillAction = getTrainingParameters()->policyStepLength - 1;
		} else {
			stepsTillAction--;
		}
	}
	// Increase step counter in this episode. 
	setStepsInThisEpisode(getStepsInThisEpisode() + 1);
	// Check if episode reached maximum length. 
	bool episodeReachedMaxLength = getStepsInThisEpisode() >= getTrainingParameters()->maxEpisodeLength;

	if(episodeReachedMaxLength) {
		return true;	// Terminate episode. 
	}
	return false;	// False = no need to terminate episode. 
}

// PROTECTED

bool TrainingControllerContinuous::initInternal() {
	setTrainedEpisodes(0);

#ifdef _DEBUG
	setVerbose(true);
#endif

	// Pre-fill state datas. 
	for(uint32_t i = 0; i < NUM_OF_AGENTS; i++) {
		getStateDatas().push_back(std::vector<StateData*>());
	}

	initAgents(NUM_OF_AGENTS);

	setStepsInThisEpisode(-1);
	stepsTillAction = getTrainingParameters()->policyStepLength - 1;
	episodesTillCheckpoint = getTrainingParameters()->episodesPerCheckpoint;

	return true;	// Let the program proceed. 
}

void TrainingControllerContinuous::cleanUpInternal() {
	// Clean up state datas. 
	TrainingController::cleanUpStateDatas();
	// Clean up agents. 
	for(Agent* agent : getAgents()) {
		delete agent;
	}
	getAgents().clear();
}

// PRIVATE

void TrainingControllerContinuous::rewardAgent(Agent* agent, bool didTakeAction, Environment* enviroment) {
	// Determine reward via rewarder. 
	double reward = TrainingRewarder::calculateReward(agent->agentID, didTakeAction, enviroment);

#ifdef MEASURE_TIME_GOAL
	if(agent->agentID == 0) {
		TRAINING_STEPS++;
		if(reward >= 100.0) {
			auto start = std::chrono::time_point_cast<std::chrono::seconds>(TRAINING_START).time_since_epoch().count();
			auto finish = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count();
			auto total = finish - start;
			std::cout << "Training ran from " + std::to_string(start) + " to " + std::to_string(finish) + "(total of " + std::to_string(total) + " seconds) in " + std::to_string(TRAINING_STEPS) + " training steps" << std::endl;
			abort();
		}
	}
#endif

	// Add reward. 
	agent->rewards.push_back(reward);
	agent->totalReward += reward;
	agent->rewardsCount++;
}

void TrainingControllerContinuous::resetAgentTrainingStep(Agent* agent) {
	agent->states.clear();
	agent->actions.clear();
	agent->values.clear();
	agent->logProbs.clear();
	agent->rewards.clear();
	agent->totalReward = 0.0;
	agent->rewardsCount = 0;
}
