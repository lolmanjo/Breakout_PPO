#pragma once

#include <vector>

#include "TrainingConsts.h"
#include "TrainingParameters.h"

namespace PLANS {

	class TrainingLogger {
		private:
			static const bool ENABLED;
			static const bool LOG_REWARDS;

			static std::string currentLogFilePath;
			static std::ofstream outStream;

			static std::string determineValidLogFilePath(const std::string& logFilePath);
			static void appendLineToFile(const std::string& string, bool addLogDate = true);
			static void ensureLogsDirectory();
			static void ensureTmpDirectory();
			static std::string getLogDate();
		protected:
		public:
			static void init();
			static void setLogFilePath(const std::string& logFilePath);

			static void log(const std::string& message);
			static void logFile(const std::string& message);

			static void onTrainingStarted(const TrainingParameters* trainingParameters);
			static void onNextSzenarioSet(const std::string& additionalInfo);
			static void onAgentRewarded(AGENT_ID agentID, double reward);
			static void onAgentTrained(AGENT_ID agentID, double reward);
			static void logActionCounts(AGENT_ID agentID, const std::vector<uint32_t>& actionCounts);
			static void onEpisodeTerminated(uint32_t episode, bool episodeReachedMaxLength, bool environmentCaused);
			static void onCheckpointCreated(const std::string& checkpointFilePath, uint32_t episode);
			static void onTrainingTerminated(uint64_t totalTrainingSeconds);
	};

}
