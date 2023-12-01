#include "TrainingControllerEpisodic.h"

#include "../TrainingParameters.h"
#include "../TrainingRewarder.h"
#include "../TrainingEncoder.h"
#include "../Environment.h"
#include "../util/Maths.h"
#include "../Models.h"
#include "../TrainingLogger.h"
#include "../util/HTTPHelper.h"

using namespace PLANS;
using namespace AEX;

TrainingControllerEpisodic::TrainingControllerEpisodic(TrainingParameters* parameters, Environment* enviroment) : TrainingController(parameters, enviroment), stepsTillAction(), episodesTillOptimization(), episodesTillCheckpoint(), lastEpisodeRewards() {}

bool TrainingControllerEpisodic::onNextScenarioRequired(bool isInit) {
	if(isInit) {
		// First szenario. 
		episodesTillOptimization = getTrainingParameters()->trainingStepLength;

	} else {
		// An episode ended. 
		setTrainedEpisodes(getTrainedEpisodes() + 1);

		// Log episode reward of agents and maybe manipulate reward values. 
		for(Agent* agent : getAgents()) {
			// Count rewards in this episode ("episode reward"). 
			double episodeReward = 0.0;
			for(uint32_t i = 0; i < getStepsInThisEpisode(); i++) {
				episodeReward += agent->rewards[agent->rewardsCount - getStepsInThisEpisode() + i];
			}

			// If agents ID is 0, post average of last 100 episode rewards. 
			if(agent->agentID == 0) {
				lastEpisodeRewards.add(episodeReward);
				if(lastEpisodeRewards.size() > 100) {
					// Remove oldest value. 
					lastEpisodeRewards.removeIndex(0);
					// Calculate average. 
					double average = calculateAverage(lastEpisodeRewards);
					// Post average. 
					HTTPHelper::postRewardAverage(average);
				}
			}

			// Log episode rewards. 
			TrainingLogger::onAgentRewarded(agent->agentID, episodeReward);

			// If the environment only gives a reward at the end, modify reward values. 
			if(getEnvironment()->onlyFinalReward() && episodeReward > 0.0) {
				// Set reward for all steps in this episode to the episode reward. 
				for(uint32_t i = 0; i < getStepsInThisEpisode(); i++) {
					agent->rewards[agent->rewardsCount - getStepsInThisEpisode() - 1 + i] = episodeReward;
				}
			}
		}

		episodesTillOptimization--;
		// Check whether its time for an optimization step. 
		if(episodesTillOptimization == 0) {
			// Optimization step. 
			// Optimize agents. 
			for(Agent* agent : getAgents()) {

				//if(agent->totalReward != 0.0) {
					// Optimize agent. 
					optimizePPO(agent);
					// Log total reward. 
					TrainingLogger::onAgentTrained(agent->agentID, agent->totalReward);
				//} else {
				//	consoleOut("TrainingControllerContinuous::onAgentExecuted: Not optimizing agent " + std::to_string(agent->agentID) + ", because his total reward is 0.0.", false);
				//}

				// Update VM episode count. 
				TrainingController::updateVMEpisodeCount(getTrainedEpisodes());

				// Reset agent. 
				resetAgentTrainingStep(agent);
			}

			// Clean up state datas. 
			TrainingController::cleanUpStateDatas();

			// Reset optimization counter. 
			episodesTillOptimization = getTrainingParameters()->trainingStepLength;
		}

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

	// Log setting of next szenario. 
	TrainingLogger::onNextSzenarioSet("");

	// Reset episode values. 
	setStepsInThisEpisode(-1);
	stepsTillAction = getTrainingParameters()->policyStepLength - 1;
	TrainingController::cleanUpStateDatas();

	return false;	// Don't terminate, there are episodes to do left. 
}

bool TrainingControllerEpisodic::onActionRequired(AGENT_ID agentID, std::vector<torch::Tensor>& output) {
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

void TrainingControllerEpisodic::onAgentExecuted(AGENT_ID agentID) {}

bool TrainingControllerEpisodic::onGameTickPassed() {
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

bool TrainingControllerEpisodic::initInternal() {
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

void TrainingControllerEpisodic::cleanUpInternal() {
	// Clean up state datas. 
	TrainingController::cleanUpStateDatas();
	// Clean up agents. 
	for(Agent* agent : getAgents()) {
		delete agent;
	}
	getAgents().clear();
}

void TrainingControllerEpisodic::rewardAgent(Agent* agent, bool didTakeAction, Environment* enviroment) {
	// Determine reward via rewarder. 
	double reward = TrainingRewarder::calculateReward(agent->agentID, didTakeAction, enviroment);

	// Add reward to agent. 
	agent->rewards.push_back(reward);
	agent->totalReward += reward;
	agent->rewardsCount++;
}

void TrainingControllerEpisodic::resetAgentTrainingStep(Agent* agent) {
	agent->states.clear();
	agent->actions.clear();
	agent->values.clear();
	agent->logProbs.clear();
	agent->rewards.clear();
	agent->totalReward = 0.0;
	agent->rewardsCount = 0;
}

double TrainingControllerEpisodic::calculateAverage(const ArrayList<double>& values) const {
	if(values.isEmpty()) {
		return 0.0;
	}
	double sum = 0.0;
	for(double d : values) {
		sum += d;
	}
	sum /= values.size();
	return sum;
}
