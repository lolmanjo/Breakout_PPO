#pragma once

#include "TrainingController.h"

#include <cstdint>

namespace PLANS {

	//############################ TrainingControllerContinuous ############################

	class TrainingControllerContinuous : public TrainingController {
		public:
			TrainingControllerContinuous(TrainingParameters* parameters, Environment* enviroment);

			virtual bool onNextScenarioRequired(bool isInit) final override;

			virtual bool onActionRequired(AGENT_ID agentID, std::vector<torch::Tensor>& output) final override;

			virtual void onAgentExecuted(AGENT_ID agentID) final override;

			virtual bool onGameTickPassed() final override;
		protected:
			virtual bool initInternal() final override;
			virtual void cleanUpInternal() final override;
		private:
			uint32_t stepsTillAction;
			uint32_t episodesTillCheckpoint;

			void rewardAgent(Agent* agent, bool didTakeAction, Environment* enviroment);
			// Called after every optimizer step to reset the rewards and values of the agents. 
			void resetAgentTrainingStep(Agent* agent);
	};

}
