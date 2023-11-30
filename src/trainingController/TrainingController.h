#pragma once

#include <cstdint>

#include "../Models.h"
#include "../util/Serialization.h"

namespace PLANS {

	// Determine which model to use. 
	using Model = ActorCritic;
	using ModelImpl = ActorCriticImpl;
	//using Model = ActorCriticOpenAIFive;
	//using ModelImpl = ActorCriticOpenAIFiveImpl;

	// Determine which optimizer to use. 
	using Optimizer = torch::optim::Adam;

	//############################ Agent ############################
	
	class Agent {
		public:
			Agent(AGENT_ID agentID);
			~Agent();
		protected:
		private:
			AGENT_ID agentID;

			Model* model;
			Optimizer* optimizer;

			std::vector<torch::Tensor> states;		// Tensors of size { LSTM_INPUT_SIZE }. 
			std::vector<torch::Tensor> actions;		// Tensors of size { 1 }. 
			std::vector<torch::Tensor> logProbs;
			std::vector<torch::Tensor> values;
			std::vector<double> rewards;
			double totalReward;
			uint32_t rewardsCount;

			friend class TrainingController;
			friend class TrainingControllerContinuous;
			friend class TrainingControllerEpisodic;
	};

	//############################ StateData ############################

	struct StateData {
		uint32_t stepIndex;
		torch::Tensor inputTensor;			// Build and stays on the CPU. Used by the loss function (which runs on the CPU). 
		torch::Tensor inputTensorDevice;	// Copied to the actual device (CPU or CUDA). Used by the torch modules. 

		StateData(uint32_t stepIndex);
		~StateData();
	};
	
	//############################ TrainingController ############################

	struct TrainingParameters;
	class Environment;
	
	class TrainingController {
		public:
			static TrainingController* getInstance();

			~TrainingController();

			// Called from TrainMain.cpp on application start. 
			bool init();

			// Called from TrainMain.cpp, when TrainingController::onGameTickPassed returned true before. 
			// Creates a new world and activates it in the game state. Also recreates the lock step manager (LockStepManagerTraining). 
			// Returns whether the maximum episode count has been reached. 
			virtual bool onNextScenarioRequired(bool isInit) = 0;

			// Called when an action is required. Returns whether an action has been produced. 
			// Creates a state tensor for the current step index in the perspective of the given faction. 
			virtual bool onActionRequired(AGENT_ID agentID, std::vector<torch::Tensor>& output) = 0;

			// Called from NPC::executeInternal (when the npc sucessfully executed and the events will be deleted next). 
			virtual void onAgentExecuted(AGENT_ID agentID) = 0;

			// Called after LockStepManagerTrainer::update from StateGame::update. 
			// Indicates, that the reward for the passed game tick can be calculated. Returns whether the episode should be terminated. 
			virtual bool onGameTickPassed() = 0;

			// Called from TrainMain.cpp. 
			void cleanUp();

			void consoleOut(const std::string& output, bool regardVerbosity = true) const;

			const TrainingParameters* getTrainingParameters() const;
			Environment* getEnvironment() const;
			const torch::TensorOptions& getTensorOptions() const;
			const torch::TensorOptions& getTensorOptionsCPU() const;
			bool isVerbose() const;
			uint32_t getTrainedEpisodes() const;
			uint32_t getStepsInThisEpisode() const;
		protected:
			TrainingController(TrainingParameters* parameters, Environment* enviroment);

			virtual bool initInternal() = 0;
			virtual void cleanUpInternal() = 0;

			void setVerbose(bool verbose);
			void setTrainedEpisodes(uint32_t trainedEpisodes);
			void setStepsInThisEpisode(uint32_t stepsInThisEpisode);

			void initAgents(uint32_t numOfAgents);
			std::vector<Agent*>& getAgents();

			void addStateData(AGENT_ID agentID, const StateData* stateData);
			const StateData* getStateData(AGENT_ID agentID, uint32_t stepIndex);
			std::vector<std::vector<StateData*>>& getStateDatas();
			const StateData* getOrCreateStateData(AGENT_ID agentID);
			void cleanUpStateDatas();

			void ensureRequiredDirectories() const;
			void serializeAgent(Agent* agent, const std::string& tmpFilePath, AEX::Serializer& serializer);
			void saveAgents(uint32_t episode, std::string& checkpointFilePath);	// SLOW. 
			void deserializeAgent(Agent* agent, const std::string& tmpFilePath, AEX::Deserializer& deserializer);
			uint32_t loadAgents();	// SLOW. 

			// Optimizes the given agent based on the PPO algorithm. 
			void optimizePPO(Agent* agent);

			void updateVMEpisodeCount(uint32_t episodeCount) const;
		private:
			static TrainingController* instance;

			TrainingParameters* params;
			Environment* enviroment;

			torch::TensorOptions tensorOptions;
			torch::TensorOptions tensorOptionsCPU;

			bool verbose;
			uint32_t trainedEpisodes;
			uint32_t stepsInThisEpisode;
			std::mutex backwardMutex;
			std::vector<Agent*> agents;

			std::vector<std::vector<StateData*>> stateDatas;
			std::mutex stateDataMutex;
	};

}
