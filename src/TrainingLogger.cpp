#include "TrainingLogger.h"

#include <filesystem>
#include <ctime>

#include "trainingController/TrainingController.h"
#include "TrainingParameters.h"
#include "util/StringUtils.h"
#include "util/IOUtils.h"

using namespace PLANS;
using namespace AEX;

const bool TrainingLogger::ENABLED = true;
const bool TrainingLogger::LOG_REWARDS = true;

std::string TrainingLogger::currentLogFilePath = "";
std::ofstream TrainingLogger::outStream = {};

std::string TrainingLogger::determineValidLogFilePath(const std::string& logFilePath) {
	std::string pathWithoutEnding = logFilePath;
	StringUtils::replace(pathWithoutEnding, ".txt", "");
	std::string ret = pathWithoutEnding + ".txt";
	uint64_t nextSuffix = 1;
	while(std::filesystem::exists(std::filesystem::u8path(ret)) && !TrainingController::getInstance()->getTrainingParameters()->continueLogFile) {
		ret = pathWithoutEnding + "_" + std::to_string(nextSuffix++) + ".txt";
	}
	return ret;
}

void TrainingLogger::appendLineToFile(const std::string& string, bool addLogDate) {
	if(ENABLED && outStream.is_open()) {
		if(addLogDate) {
			outStream << getLogDate() + ": " + string + "\n";
		} else {
			outStream << string + "\n";
		}
		outStream.flush();
	}
}

void TrainingLogger::ensureLogsDirectory() {
	if(!IOUtils::exists("./logs/")) {
		IOUtils::createDirectory("./logs/");
	}
}

void TrainingLogger::ensureTmpDirectory() {
	if(!IOUtils::exists("./tmp/")) {
		IOUtils::createDirectory("./tmp/");
	}
}

std::string TrainingLogger::getLogDate() {
	time_t now = time(0);
	tm result;
#ifdef _WIN32
	localtime_s(&result, &now);
#elif __linux__
	localtime_r(&now, &result);
#endif
	char formattedData[22];
#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation="
#endif
	snprintf(formattedData, sizeof(formattedData), "%02d.%02d.%04d : %02d.%02d.%02d", result.tm_mday, 1 + result.tm_mon, 1900 + result.tm_year, result.tm_hour, result.tm_min, result.tm_sec);
#ifdef __linux__
#pragma GCC diagnostic pop
#endif
	return std::string(formattedData);
}

void TrainingLogger::init() {
	ensureLogsDirectory();
	ensureTmpDirectory();
}

void TrainingLogger::setLogFilePath(const std::string& logFilePath) {
	if(!ENABLED) {
		return;
	}
	if(outStream.is_open()) {
		appendLineToFile("###### Closing log file \"" + currentLogFilePath + "\" ######");
		outStream.flush();
		outStream.close();
	}
	if(logFilePath != "") {
		currentLogFilePath = determineValidLogFilePath(logFilePath);
		TrainingLogger::log("TrainingLogger::setLogFilePath: Path set to \"" + currentLogFilePath + "\".");
		try {
			outStream.open(currentLogFilePath, std::ios_base::app);
			appendLineToFile("###### Opened log file \"" + currentLogFilePath + "\" ######");
		} catch(std::exception& e) {
			TrainingLogger::log("TrainingLogger::setLogFilePath: Error while opening log file \"" + currentLogFilePath + "\". Error: " + e.what());
		}
	}
}

void TrainingLogger::log(const std::string& message) {
	std::cout << message << std::endl;
}

void TrainingLogger::logFile(const std::string& message) {
	log(message);
	appendLineToFile(message, true);
}

void TrainingLogger::onTrainingStarted(const TrainingParameters* trainingParameters) {
	appendLineToFile("##### Training started / resumed. Parameters: #####");
	appendLineToFile(">checkpointDirectoryName	:	" + trainingParameters->checkpointDirectoryName);
	appendLineToFile(">modelNameLoad	:	" + trainingParameters->modelNameLoad);
	appendLineToFile(">modelNameSave	:	" + trainingParameters->modelNameSave);
	appendLineToFile(">learningRate	:	" + std::to_string(trainingParameters->learningRate));
	appendLineToFile(">policyStepLength	:	" + std::to_string(trainingParameters->policyStepLength));
	appendLineToFile(">trainingStepLength	:	" + std::to_string(trainingParameters->trainingStepLength));
	appendLineToFile(">maxEpisodeLength	:	" + std::to_string(trainingParameters->maxEpisodeLength));
	appendLineToFile(">maxEpisodes	:	" + std::to_string(trainingParameters->maxEpisodes));
	appendLineToFile(">continueLogFile	:	" + std::string(trainingParameters->continueLogFile ? "true" : "false"));
}

void TrainingLogger::onNextSzenarioSet(const std::string& additionalInfo) {
	appendLineToFile("### Szenario set, additional info: \"" + additionalInfo + "\" ###");
}

void TrainingLogger::onAgentRewarded(AGENT_ID agentID, double reward) {
	if(!LOG_REWARDS) {
		return;
	}
	appendLineToFile("Agent \"" + std::to_string(agentID) + "\", reward=\"" + std::to_string(reward) + "\"");
}

void TrainingLogger::onAgentTrained(AGENT_ID agentID, double totalReward) {
	//appendLineToFile("# \"" + agentShortName + "\" trained. totalReward=\"" + std::to_string(totalReward) + "\", actorLoss=\"" + std::to_string(actorLoss) + "\", criticLoss=\"" + std::to_string(criticLoss) + "\" #");
	appendLineToFile("# Agent \"" + std::to_string(agentID) + "\" trained. total reward=\"" + std::to_string(totalReward) + "\" #");
}

void TrainingLogger::logActionCounts(AGENT_ID agentID, const std::vector<uint32_t>& actionCounts) {
	for(uint32_t i = 0; i < actionCounts.size(); i++) {
		appendLineToFile("# Agent \"" + std::to_string(agentID) + "\" executed action " + std::to_string(i) + " " + std::to_string(actionCounts[i]) + " times #");
	}
}

void TrainingLogger::onEpisodeTerminated(uint32_t episode, bool episodeReachedMaxLength, bool environmentCaused) {
	appendLineToFile("#### Episode terminated. episode=\"" + std::to_string(episode) + "\",episodeReachedMaxLength=\"" + std::string(episodeReachedMaxLength ? "true" : "false") + "\", environmentCaused=\"" + std::string(environmentCaused ? "true" : "false") + "\" ####");
}

void TrainingLogger::onCheckpointCreated(const std::string& checkpointFilePath, uint32_t episode) {
	appendLineToFile("## New checkpoint created. File path: \"" + checkpointFilePath + "\", episode: " + std::to_string(episode) + ". ##");
}

void TrainingLogger::onTrainingTerminated(uint64_t totalTrainingSeconds) {
	appendLineToFile("##### Training terminated. Total training time: " + std::to_string(totalTrainingSeconds) + " seconds #####");
}
