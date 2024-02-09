#include "TrainingConsts.h"
#include "Environment.h"
#include "TrainingParser.h"
#include "trainingController/TrainingController.h"
#include "trainingController/TrainingControllerContinuous.h"
#include "trainingController/TrainingControllerEpisodic.h"
#include "TrainingLogger.h"
#include "TrainingEncoder.h"
#include "util/Maths.h"
#include <chrono>
#include "util/HTTPHelper.h"

using namespace PLANS;

int main(int argc, const char** argv) {

	at::globalContext().setAllowTF32CuDNN(true);
	at::globalContext().setDeterministicCuDNN(true);

	// Parse training parameters. 
	TrainingParameters* parameters = new TrainingParameters();
	TrainingParser::parseConfigFile("./trainingConfig.json", parameters);
	
	// Init environment. 
	//Environment* enviroment = new EnvironmentBinary();
	Environment* enviroment = new EnvironmentBreakout();

	// Determine actual num of agents. 
	NUM_OF_AGENTS = Maths::min(NUM_OF_AGENTS_DESIRED, enviroment->maxNumOfAgents());
	
	// Init training controller. 
	//TrainingController* trainingController = new TrainingControllerContinuous(parameters, enviroment);
	TrainingController* trainingController = new TrainingControllerEpisodic(parameters, enviroment);
	if(!trainingController->init()) {
		return 1;	// Somehow failed. 
	}

	TrainingLogger::init();
	TrainingLogger::setLogFilePath(LOGS_DIRECTORY_PATH + parameters->modelNameSave + ".txt");
	TrainingLogger::onTrainingStarted(parameters);

	// Prepare first scenario. 
	trainingController->onNextScenarioRequired(true);

	// Train until telled to stop. 
	bool stop = false;
	bool actionTaken = false;
	float action = 0.0F;
	std::vector<torch::Tensor> output;
	bool episodeReachedMaxLength = false;
	bool environmentCaused = false;
	std::chrono::high_resolution_clock::time_point lastKeepAliveSent;
	double currentEpisodeProgress = 0.0;
	Random epsilonGreedyRandom;
	double epsilonGreedyChanceGrowth = parameters->epsilonGreedyEnd > parameters->epsilonGreedyStart ? parameters->epsilonGreedyEnd - parameters->epsilonGreedyStart : parameters->epsilonGreedyStart - parameters->epsilonGreedyEnd;
	double currentEpsilonGreedyChance = parameters->epsilonGreedyStart;
	while(!stop) {
		// Update environment. 
		enviroment->update();

		// Update agents. 
		for(uint32_t agentID = 0; agentID < NUM_OF_AGENTS; agentID++) {
			// Get action to take. 
			output.clear();
			actionTaken = trainingController->onActionRequired(agentID, output);

			if(actionTaken) {
				// Determine whether an action based on the agents output or a random action should be taken. 
				// For this to happen, epsilon greedy has to be enabled and the chance has to hit. 
				bool randomAction = false;
				if(parameters->epsilonGreedyEnabled) {
					// Update currentEpsilonGreedyChance. 
					currentEpsilonGreedyChance = Maths::clamp(parameters->epsilonGreedyStart + (epsilonGreedyChanceGrowth * currentEpisodeProgress), 0.0, 1.0);
					//
					randomAction = epsilonGreedyRandom.nextFloat() < currentEpsilonGreedyChance;

				}
				if(!randomAction) {
					// Decode action based on agent output. 
					action = TrainingEncoder::decodeAction(agentID, output[0]);
				} else {
					// Take a random action. 
					action = epsilonGreedyRandom.nextFloatInRange(0.0F, enviroment->getActionMax());
				}

				// Execute action. 
				enviroment->onAction(agentID, action);

				// Reward agent. 
				trainingController->onAgentExecuted(agentID);
			}
		}

		// Finish tick. 
		episodeReachedMaxLength = trainingController->onGameTickPassed();
		environmentCaused = enviroment->gameOver();
		if(episodeReachedMaxLength || environmentCaused) {
			TrainingLogger::onEpisodeTerminated(trainingController->getTrainedEpisodes(), episodeReachedMaxLength, environmentCaused);
			if(trainingController->onNextScenarioRequired(false)) {
				stop = true;
			} else {
				// Reset environment, as we will train another episode. 
				enviroment->reset(NUM_OF_AGENTS);
				// Update currentEpisodeProgress. 
				currentEpisodeProgress = static_cast<double>(trainingController->getTrainedEpisodes()) / static_cast<double>(parameters->maxEpisodes);
			}
		}

		// Keep alive stuff. 
		auto now = std::chrono::high_resolution_clock::now();
		uint32_t secondsElapsed = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(now - lastKeepAliveSent).count());
		if(secondsElapsed >= KEEP_ALIVE_INTERVAL) {
			// Send keep alive. 
			HTTPHelper::postKeepAlive();
			// Reset keep alive timer. 
			lastKeepAliveSent = std::chrono::high_resolution_clock::now();
		}
	}

	trainingController->cleanUp();
	delete trainingController;

	// Close logger. 
	TrainingLogger::setLogFilePath("");
}
