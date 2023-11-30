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
	while(!stop) {
		// Update environment. 
		enviroment->update();

		// Update agents. 
		for(uint32_t agentID = 0; agentID < NUM_OF_AGENTS; agentID++) {
			// Get action to take. 
			output.clear();
			actionTaken = trainingController->onActionRequired(agentID, output);

			if(actionTaken) {
				action = TrainingEncoder::decodeAction(agentID, output[0]);

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
