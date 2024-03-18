#pragma once

#include "TrainingController.h"
#include "../util/Collections.h"

namespace PLANS {

	class TrainingControllerEpisodic : public TrainingController {
		public:
			TrainingControllerEpisodic(TrainingParameters* parameters, Environment* enviroment);

			virtual bool onNextScenarioRequired(bool isInit) final override;

			virtual bool onActionRequired(AGENT_ID agentID, std::vector<torch::Tensor>& output) final override;

			virtual void onAgentExecuted(AGENT_ID agentID) final override;

			virtual bool onGameTickPassed() final override;
		protected:
			virtual bool initInternal() final override;
			virtual void cleanUpInternal() final override;
		private:
			uint32_t stepsTillAction;
			uint32_t episodesTillOptimization;
			uint32_t episodesTillCheckpoint;
			AEX::ArrayList<double> lastEpisodeRewards;

			// Called after every optimizer step to reset the rewards and values of the agents. 
			void resetAgentTrainingStep(Agent* agent);
			double calculateAverage(const AEX::ArrayList<double>& values) const;
	};

}
