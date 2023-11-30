#include "TrainingController.h"

#include <filesystem>
#include <chrono>
#include <random>

#include "../TrainingParameters.h"
#include "../TrainingRewarder.h"
#include "../TrainingEncoder.h"
#include "../TrainingLogger.h"
#include "../Environment.h"
#include "../util/Maths.h"
#include "../util/StringUtils.h"
#include "../util/IOUtils.h"
#include "../util/HTTPHelper.h"

using namespace PLANS;
using namespace AEX;

//############################ Agent ############################

Agent::Agent(AGENT_ID agentID) : agentID(agentID), model(nullptr), states(), actions(), logProbs(), values(), rewards(), totalReward(0.0), rewardsCount(0) {
	
	// Reserve memory for rewards. 
	rewards.reserve(TrainingController::getInstance()->getTrainingParameters()->trainingStepLength);

	// Create model. 
	model = new Model(STD);
	//model = new Model(TrainingControllerContinuous::LSTM_INPUT_SIZE, TrainingControllerContinuous::LSTM_OUTPUT_SIZE, STD);
	model->get()->normal(0.0, STD);

	// Move model to GPU before initializing optimizers, as doing it afterwards can cause problems with the optimizers references to the models parameters. 
#ifdef USE_CUDA
	model->get()->toDevice(torch::kCUDA);
#else
	model->get()->toDevice(torch::kCPU);
#endif
	
	// Create optimizer. 
	optimizer = new Optimizer(model->get()->parameters(), torch::optim::AdamOptions(TrainingController::getInstance()->getTrainingParameters()->learningRate));
}

Agent::~Agent() {
	delete model;
	delete optimizer;
}

//############################ StateData ############################

StateData::StateData(uint32_t stepIndex) : stepIndex(stepIndex), inputTensor(nullptr), inputTensorDevice(nullptr) {}

StateData::~StateData() {
	//inputTensor = torch::empty(1);
	//inputTensorDevice = torch::empty(1);
}

//############################ TrainingController ############################

// PUBLIC

TrainingController* TrainingController::getInstance() {
	return instance;
}

TrainingController::~TrainingController() {
	instance = nullptr;
}

bool TrainingController::init() {
	if(params == nullptr) {
		consoleOut("TrainingControllerContinuous::init: Given parameters pointer is nullptr.");
		return false;
	}
	if(enviroment == nullptr) {
		consoleOut("TrainingControllerContinuous::init: Given enviroment pointer is nullptr.");
		return false;
	}
	//
	ensureRequiredDirectories();
	// Init tensor options. 
#ifdef USE_CUDA
	tensorOptions = tensorOptions.device(torch::kCUDA).dtype(torch::kFloat32).requires_grad(false);
#else
	tensorOptions = tensorOptions.device(torch::kCPU).dtype(torch::kFloat32).requires_grad(false);
#endif
	tensorOptionsCPU = tensorOptionsCPU.device(torch::kCPU).dtype(torch::kFloat32).requires_grad(false);
	//
	return initInternal();
}

void TrainingController::cleanUp() {
	// Clean up parameters. 
	if(params != nullptr) {
		delete params;
		params = nullptr;
	}
	//
	cleanUpInternal();
}

void TrainingController::consoleOut(const std::string& output, bool regardVerbosity) const {
	if(verbose || !regardVerbosity) {
		std::cout << output << std::endl;
		//LogHelper::info(output);
	}
}

const TrainingParameters* TrainingController::getTrainingParameters() const {
	return params;
}

Environment* TrainingController::getEnvironment() const {
	return enviroment;
}

const torch::TensorOptions& TrainingController::getTensorOptions() const {
	return tensorOptions;
}

const torch::TensorOptions& TrainingController::getTensorOptionsCPU() const {
	return tensorOptionsCPU;
}

bool TrainingController::isVerbose() const {
	return verbose;
}

uint32_t TrainingController::getTrainedEpisodes() const {
	return trainedEpisodes;
}

uint32_t TrainingController::getStepsInThisEpisode() const {
	return stepsInThisEpisode;
}

// PROTECTED

TrainingController::TrainingController(TrainingParameters* parameters, Environment* enviroment) : params(parameters), enviroment(enviroment), tensorOptions(), tensorOptionsCPU(), verbose(false), trainedEpisodes(0), stepsInThisEpisode(0), backwardMutex(), agents(), stateDatas(), stateDataMutex() {
	instance = this;
}

void TrainingController::setVerbose(bool verbose) {
	this->verbose = verbose;
}

void TrainingController::setTrainedEpisodes(uint32_t trainedEpisodes) {
	this->trainedEpisodes = trainedEpisodes;
}

void TrainingController::setStepsInThisEpisode(uint32_t stepsInThisEpisode) {
	this->stepsInThisEpisode = stepsInThisEpisode;
}

void TrainingController::initAgents(uint32_t numOfAgents) {
	if(!agents.empty()) {
		consoleOut("TrainingController::initAgents: Agents already initialized.");
		return;
	}
	// Create agents. 
	for(uint32_t i = 0; i < numOfAgents; i++) {
		agents.push_back(new Agent(i));
	}
	// Load agents. 
	uint32_t loadedEpisode = loadAgents();
	// Check for loading failure. 
	if(loadedEpisode == UINT32_MAX) {
		loadedEpisode = 0;	// We start at zero again. 
		TrainingLogger::logFile("#### No checkpoint loaded, starting at episode 0. ####");
	} else {
		TrainingLogger::logFile("#### Loaded checkpoint, starting at episode " + std::to_string(loadedEpisode) + ". ####");
	}
	trainedEpisodes = loadedEpisode;
}

std::vector<Agent*>& TrainingController::getAgents() {
	return agents;
}

void TrainingController::addStateData(AGENT_ID agentID, const StateData* stateData) {
	stateDataMutex.lock();
	std::vector<StateData*>& list = stateDatas[agentID];
#ifdef _DEBUG
	const StateData* tmp = getStateData(agentID, stateData->stepIndex);
	if(tmp != nullptr) {
		consoleOut("TrainingController::addStateData: LinkedList already contains a state data with step index \"" + std::to_string(stateData->stepIndex) + "\"");
		stateDataMutex.unlock();
		return;
	}
#endif
	list.push_back(const_cast<StateData*>(stateData));
	stateDataMutex.unlock();
}

const StateData* TrainingController::getStateData(AGENT_ID agentID, uint32_t stepIndex) {
	std::vector<StateData*>& list = stateDatas[agentID];
	for(const StateData* stateData : list) {
		if(stateData->stepIndex == stepIndex) {
			return stateData;
		}
	}
	return nullptr;
}

std::vector<std::vector<StateData*>>& TrainingController::getStateDatas() {
	return stateDatas;
}

const StateData* TrainingController::getOrCreateStateData(AGENT_ID agentID) {
	// Get or create state data. 
	stateDataMutex.lock();
	const StateData* stateData = getStateData(agentID, getStepsInThisEpisode());
	if(stateData != nullptr) {
		stateDataMutex.unlock();
		return stateData;
	} else {
		// Build state data via encoder. 
		stateData = TrainingEncoder::buildInputTensor(agentID, getEnvironment());
		// Save state data, as it's new. 
		stateDataMutex.unlock();
		addStateData(agentID, stateData);
		return stateData;
	}
}

void TrainingController::cleanUpStateDatas() {
	stateDataMutex.lock();
	for(std::vector<StateData*>& list : stateDatas) {
		for(StateData* stateData : list) {
			delete stateData;
		}
		list.clear();
	}
	stateDataMutex.unlock();
}

void TrainingController::ensureRequiredDirectories() const {
	if(!IOUtils::exists("./" + params->checkpointDirectoryName)) {
		IOUtils::createDirectory("./" + params->checkpointDirectoryName);
	}
}

void TrainingController::serializeAgent(Agent* agent, const std::string& tmpFilePath, Serializer& serializer) {
	// Write each thing to a temporary file and load them (there was no better solution to get the byte data). 
	int8_t* data;
	uint64_t dataSize = 0;

	torch::save(*agent->model, tmpFilePath);
	// Read from temporary file. 
	data = IOUtils::readBytesFromFile(tmpFilePath, dataSize);
	serializer.serialize(dataSize);
	serializer.serialize(data, dataSize);
	delete[] data;

	torch::save(*agent->optimizer, tmpFilePath);
	// Read from temporary file. 
	data = IOUtils::readBytesFromFile(tmpFilePath, dataSize);
	serializer.serialize(dataSize);
	serializer.serialize(data, dataSize);
	delete[] data;
}

void TrainingController::saveAgents(uint32_t episode, std::string& checkpointFilePath) {
	// Determine checkpoint file path. 
	std::string checkpointFileName = TrainingController::params->modelNameLoad + CHECKPOINT_FILE_EXTENSION;
	if(episode > 0) {
		StringUtils::replace(checkpointFileName, CHECKPOINT_FILE_EXTENSION, "_episode" + std::to_string(episode) + CHECKPOINT_FILE_EXTENSION);
	}
	checkpointFilePath = "./" + TrainingController::params->checkpointDirectoryName + "/" + checkpointFileName;

	// Prepare stuff. 
	Serializer serializer = Serializer();
	std::string tmpFilePath = "./" + TrainingController::params->checkpointDirectoryName + "/tmp.pt";

	// Serialize agents. 
	for(Agent* agent : agents) {
		serializeAgent(agent, tmpFilePath, serializer);
	}

	// Delete temporary file. 
	IOUtils::remove(tmpFilePath);

	// Write compressed bytes to final checkpoint file. 
	IOUtils::writeCompressedBytesToFile(checkpointFilePath, true, serializer.getSerializedData(), serializer.getDataLength());
}

void TrainingController::deserializeAgent(Agent* agent, const std::string& tmpFilePath, Deserializer& deserializer) {
	// Write each piece to temporary file and load it via "torch::load". 
	uint64_t dataSizePiece = 0;
	int8_t* pieceData = 0;

	deserializer.deserialize(dataSizePiece);
	deserializer.deserialize(pieceData, dataSizePiece);
	IOUtils::writeBytesToFile(tmpFilePath, true, pieceData, dataSizePiece);
	torch::load(*agent->model, tmpFilePath);

	deserializer.deserialize(dataSizePiece);
	deserializer.deserialize(pieceData, dataSizePiece);
	IOUtils::writeBytesToFile(tmpFilePath, true, pieceData, dataSizePiece);
	torch::load(*agent->optimizer, tmpFilePath);
}

uint32_t TrainingController::loadAgents() {
	// Determine checkpoint file path. 
	uint32_t highestEpisode = UINT32_MAX;
	std::string checkpointFilename;
	// Search checkpoints for valid files. 
	DirectoryIterator directoryIterator = IOUtils::getDirectoryIterator("./" + TrainingController::params->checkpointDirectoryName + "/");
	uint32_t splitCount;
	std::string* splitParts = new std::string[2];
	std::string tmpString;
	uint32_t currentEpisode;
	for(const DirectoryEntry entry : directoryIterator) {
		if(entry.isDirectory) {
			continue;	// Ignore directories. 
		}
		if(entry.extension != CHECKPOINT_FILE_EXTENSION) {
			continue;	// Wrong file extension. 
		}
		splitCount = StringUtils::split(entry.name, "_", splitParts, 2);
		// Check for correct model name. 
		if(splitCount == 1) {
			tmpString = splitParts[0];
			StringUtils::replace(tmpString, CHECKPOINT_FILE_EXTENSION, "");
			if(tmpString != TrainingController::params->modelNameLoad) {
				continue;	// Wrong model name. 
			}
		} else if(splitCount == 2 && splitParts[0] != TrainingController::params->modelNameLoad) {
			continue;	// Wrong model name. 
		}
		// Check for episode in file name. 
		if(StringUtils::startsWith(splitParts[1], "episode") && StringUtils::endsWith(splitParts[1], CHECKPOINT_FILE_EXTENSION)) {
			// There is an episode encoded. Parse it. 
			tmpString = splitParts[1];
			StringUtils::replace(tmpString, "episode", "");
			StringUtils::replace(tmpString, CHECKPOINT_FILE_EXTENSION, "");
			try {
				currentEpisode = static_cast<uint32_t>(std::stoi(tmpString));
			} catch(std::exception&) {
				// Not just a number, also letters. Can't parse the episode. Ignore this checkpoint file. 
				continue;
			}
		} else {
			// There is no episode encoded. Assume episode 0 then. 
			currentEpisode = 0;
		}
		// Check if the current episode is greater. 
		if(highestEpisode == UINT32_MAX || currentEpisode > highestEpisode) {
			highestEpisode = currentEpisode;
			checkpointFilename = entry.name;
		}
	}
	delete[] splitParts;
	// 
	if(highestEpisode == UINT32_MAX) {
		consoleOut("TrainingController::loadAgents: No checkpoint file found.", false);
		return UINT32_MAX;
	}
	std::string checkpointFilePath = "./" + TrainingController::params->checkpointDirectoryName + "/" + checkpointFilename;

	std::string tmpFilePath = "./" + TrainingController::params->checkpointDirectoryName + "/tmp.pt";

	// Load compressed bytes. 
	uint64_t dataSizeTotal = 0;
	int8_t* data = IOUtils::readCompressedBytesFromFile(checkpointFilePath, dataSizeTotal);

	// Create deserializer. 
	Deserializer deserializer = Deserializer(data, dataSizeTotal, 0, true);

	// Deserialize agents. 
	for(Agent* agent : agents) {
		deserializeAgent(agent, tmpFilePath, deserializer);
	}

	// Delete temporary file. 
	IOUtils::remove(tmpFilePath);

	return highestEpisode;
}

void TrainingController::optimizePPO(Agent* agent) {

	consoleOut("TrainingController::optimizePPO: Agent " + std::to_string(agent->agentID) + ", total reward: " + std::to_string(agent->totalReward), false);

	torch::Tensor t_values = torch::cat(agent->values).detach().to(torch::kCPU);

	// Calculate the returns. 
	torch::Tensor gae = torch::zeros(1, getTensorOptionsCPU());
	std::vector<torch::Tensor> returns = std::vector<torch::Tensor>(agent->rewards.size(), torch::zeros(1, getTensorOptionsCPU()));
	torch::Tensor delta;
	for(size_t i = agent->rewards.size() - 1; i > 0; i--) {
		delta = agent->rewards[i] + getTrainingParameters()->ppo_gamma * t_values[i - 1] - t_values[i];	// See [SchulmanEtAl, 2017]_PPO, Equation (12)
		gae = delta + getTrainingParameters()->ppo_gamma * getTrainingParameters()->ppo_lambda * gae;

		returns[i] = gae + t_values[i];
	}

	// Build copies of tensors. 
	torch::Tensor t_logProbs = torch::cat(agent->logProbs).detach().to(torch::kCPU);
	torch::Tensor t_returns = torch::cat(returns).detach();
	torch::Tensor t_states = torch::cat(agent->states).to(torch::kCPU);
	torch::Tensor t_actions = torch::cat(agent->actions).to(torch::kCPU);

	// Calculate the advantages. 
	torch::Tensor t_advantages = (t_returns - t_values).slice(0, 0, getTrainingParameters()->trainingStepLength);	// Size: { trainingStepLength(120) }

	//int64_t a_a = t_advantages.dim();
	//int64_t a_b = t_advantages.size(0);

	// NOTE: From here the sources are https://github.com/mhubii/ppo_libtorch/tree/master and https://github.com/ericyangyu/PPO-for-Beginners. 
	torch::Tensor mini_states;
	torch::Tensor mini_actions;
	torch::Tensor mini_logProbs;
	torch::Tensor mini_returns;
	torch::Tensor mini_advantages;
	for(uint32_t i = 0; i < getTrainingParameters()->ppo_epochs; i++) {
		// Construct mini batch. 
		mini_states = torch::zeros({ getTrainingParameters()->ppo_miniBatchSize, LSTM_INPUT_SIZE }, getTensorOptions());
		mini_actions = torch::zeros({ getTrainingParameters()->ppo_miniBatchSize, LSTM_OUTPUT_SIZE }, getTensorOptions());
		mini_logProbs = torch::zeros({ getTrainingParameters()->ppo_miniBatchSize, LSTM_OUTPUT_SIZE }, getTensorOptions());
		mini_returns = torch::zeros({ getTrainingParameters()->ppo_miniBatchSize, 1 }, getTensorOptions());
		mini_advantages = torch::zeros({ getTrainingParameters()->ppo_miniBatchSize, 1 }, getTensorOptions());

		uint32_t idx;
		for(uint32_t b = 0; b < getTrainingParameters()->ppo_miniBatchSize; b++) {
			//idx = std::uniform_int_distribution<uint32_t>(0, getTrainingParameters()->trainingStepLength - 1)(re);	// Randomize order. 
			idx = (i * getTrainingParameters()->ppo_miniBatchSize) + b;
			mini_states[b] = t_states[idx];
			mini_actions[b] = t_actions[idx];
			mini_logProbs[b] = t_logProbs[idx];
			mini_returns[b] = t_returns[idx];
			mini_advantages[b] = t_advantages[idx];
		}

		//mini_states.dim();
		//mini_states.size(0); // trainingStepLength
		//mini_states.size(1); // TrainingControllerContinuous::LSTM_INPUT_SIZE

		//std::vector<torch::Tensor> av = agent->model->get()->forward(mini_states, false); // action value pairs

		double beta = getTrainingParameters()->ppo_beta;

		std::tuple<torch::Tensor, torch::Tensor> av = agent->model->get()->forward(mini_states, false); // action value pairs
		torch::Tensor action = std::get<0>(av);
		torch::Tensor entropy = agent->model->get()->entropy().mean();
		torch::Tensor new_log_prob = agent->model->get()->logProb(mini_actions);

		torch::Tensor old_log_prob = mini_logProbs;
		torch::Tensor ratio = (new_log_prob - old_log_prob).exp();
		torch::Tensor surr1 = ratio * mini_advantages;	//  ratio is { 30, 6 }, mini_advantages is { 30, 1 }. 
		torch::Tensor surr2 = torch::clamp(ratio, 1.0 - beta, 1.0 + beta) * mini_advantages;

		torch::Tensor val = std::get<1>(av);
		torch::Tensor actorLoss = -torch::min(surr1, surr2).mean();
		torch::Tensor criticLoss = (mini_returns - val).pow(2).mean();

		// Calculate total loss. 
		torch::Tensor totalLoss = 0.5 * criticLoss + actorLoss - getTrainingParameters()->ppo_gamma * entropy;

		// Lock mutex because of potentially asynchronous backward call
		backwardMutex.lock();

		//agent->model->get()->actor.get()->weight.dim();
		//float f_1 = agent->model->get()->actor.get()->weight[0][0].item().toFloat();
		//float f_1 = agent->model->get()->a_lin3_->weight[0][0].item().toFloat();
		//float loss = totalLoss.item().toFloat();

		// Update model. 
		agent->optimizer->zero_grad();
		//torch::Tensor tt = totalLoss.grad();
		if(i != getTrainingParameters()->ppo_epochs - 1) {
			// Retain graph if this is not the last ppo_epoch. 
			totalLoss.backward({}, true);
		} else {
			// This is the last ppo_epoch, don't retain the graph. 
			totalLoss.backward({}, false);
		}
		//torch::Tensor tt_2 = totalLoss.grad();
		agent->optimizer->step();

		//float f_2 = agent->model->get()->actor.get()->weight[0][0].item().toFloat();
		//float f_2 = agent->model->get()->a_lin3_->weight[0][0].item().toFloat();

		// Unlock mutex. 
		backwardMutex.unlock();
	}
}

void TrainingController::updateVMEpisodeCount(uint32_t episodeCount) const {
	HTTPHelper::postLastEpisodeFinished(episodeCount);
}

// PRIVATE

TrainingController* TrainingController::instance = nullptr;
