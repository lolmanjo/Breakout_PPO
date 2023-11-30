#pragma once

#include "TrainingConsts.h"
#include "util/Random.h"

namespace PLANS {

	class Environment {
		public:
			Environment() = default;

			virtual uint32_t maxNumOfAgents() = 0;
			virtual bool onlyFinalReward() = 0;		// Whether the environment only rewards once per episode (at the last act) or continuously. 
			virtual void reset(uint32_t numOfAgents) = 0;
			virtual void update() = 0;
			virtual void getInputData(AGENT_ID agentID, std::vector<float>& data) = 0;
			virtual void onAction(AGENT_ID agentID, float action) = 0;
			virtual float rewardAgent(AGENT_ID agentID) = 0;
			virtual bool gameOver() = 0;
		protected:
			template<typename T>
			static void putIntoData(std::vector<float>& dataVector, int& currentIndex, T value) {
				dataVector[currentIndex++] = static_cast<float>(value);
			}
		private:
	};

	//############################ EnvironmentBinary ############################

	class EnvironmentBinary : public Environment {
		public:
			EnvironmentBinary();

			virtual uint32_t maxNumOfAgents() final override;
			virtual bool onlyFinalReward() final override;
			virtual void reset(uint32_t numOfAgents) final override;
			virtual void update() final override;
			virtual void getInputData(AGENT_ID agentID, std::vector<float>& data) final override;
			virtual void onAction(AGENT_ID agentID, float action) final override;
			virtual float rewardAgent(AGENT_ID agentID) final override;
			virtual bool gameOver() final override;
		protected:
		private:
			Random random;
			uint8_t state;
			std::vector<uint8_t> actions;
	};

	//############################ EnvironmentFloat ############################

	class EnvironmentFloat : public Environment {
		public:
			EnvironmentFloat();

			virtual uint32_t maxNumOfAgents() final override;
			virtual bool onlyFinalReward() final override;
			virtual void reset(uint32_t numOfAgents) final override;
			virtual void update() final override;
			virtual void getInputData(AGENT_ID agentID, std::vector<float>& data) final override;
			virtual void onAction(AGENT_ID agentID, float action) final override;
			virtual float rewardAgent(AGENT_ID agentID) final override;
			virtual bool gameOver() final override;
		protected:
		private:
			float state;
			std::vector<float> actions;
	};

	//############################ EnvironmentBreakout ############################

	/*
	*	Atari Breakout environment. 
	*		- Rewards are only given when the episode is game over (all lives gone). 
	*		- The reward equals the total score. 
	*/
	class EnvironmentBreakout : public Environment {
		public:
			EnvironmentBreakout();

			virtual uint32_t maxNumOfAgents() final override;
			virtual bool onlyFinalReward() final override;
			virtual void reset(uint32_t numOfAgents) final override;
			virtual void update() final override;
			virtual void getInputData(AGENT_ID agentID, std::vector<float>& data) final override;
			virtual void onAction(AGENT_ID agentID, float action) final override;
			virtual float rewardAgent(AGENT_ID agentID) final override;
			virtual bool gameOver() final override;
		protected:
		private:
			ale::ALEInterface ale;
			ale::ActionVect legal_actions;
			float reward;
	};

}
