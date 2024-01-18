#include "Environment.h"

#include "util/Maths.h"

using namespace PLANS;

//############################ EnvironmentBinary ############################

EnvironmentBinary::EnvironmentBinary() : Environment(), random(), state(), actions() {}

uint32_t EnvironmentBinary::maxNumOfAgents() {
	return UINT32_MAX;
}

bool EnvironmentBinary::onlyFinalReward() {
	return false;
}

void EnvironmentBinary::reset(uint32_t numOfAgents) {
	actions.clear();
	for(uint32_t i = 0; i < numOfAgents; i++) {
		actions.push_back(0);
	}
	// Set initial state. 
	update();
}

void EnvironmentBinary::update() {
	state = static_cast<uint8_t>(random.nextIntInRange(1, 2));
}

void EnvironmentBinary::getInputData(AGENT_ID agentID, std::vector<float>& data) {
	// Put relevant environment data into "data" via "putIntoData". 
	int currIndex = 0;
	for(int64_t i = 0; i < LSTM_INPUT_SIZE; i++) {
		putIntoData(data, currIndex, state);
	}
}

void EnvironmentBinary::onAction(AGENT_ID agentID, float action) {
	actions[agentID] = static_cast<uint8_t>(action);
}

float EnvironmentBinary::rewardAgent(AGENT_ID agentID) {
	if(state == actions[agentID]) {
		return 10.0F;
	} else {
		return -10.0F;
	}
}

bool EnvironmentBinary::gameOver() {
	return false;
}

//############################ EnvironmentFloat ############################

EnvironmentFloat::EnvironmentFloat() : Environment(), state(1.0F), actions() {}

uint32_t EnvironmentFloat::maxNumOfAgents() {
	return UINT32_MAX;
}

bool EnvironmentFloat::onlyFinalReward() {
	return false;
}

void EnvironmentFloat::reset(uint32_t numOfAgents) {
	actions.clear();
	for(uint32_t i = 0; i < numOfAgents; i++) {
		actions.push_back(0.0F);
	}
}

void EnvironmentFloat::update() {}

void EnvironmentFloat::getInputData(AGENT_ID agentID, std::vector<float>& data) {
	// Put relevant environment data into "data" via "putIntoData". 
	int currIndex = 0;
	for(int64_t i = 0; i < LSTM_INPUT_SIZE; i++) {
		putIntoData(data, currIndex, state);
	}
}

void EnvironmentFloat::onAction(AGENT_ID agentID, float action) {
	actions[agentID] = action;
}

float EnvironmentFloat::rewardAgent(AGENT_ID agentID) {
	return Maths::abs(state - actions[agentID]);
}

bool EnvironmentFloat::gameOver() {
	return false;
}

//############################ EnvironmentBreakout ############################

EnvironmentBreakout::EnvironmentBreakout() : ale(), reward() {
	// Prepare interface. 
	ale.setInt("random_seed", 123);
	ale.setFloat("repeat_action_probability", 0.25);
	ale.loadROM("./roms/Breakout.bin");
	// Get the vector of legal actions. 
	legal_actions = ale.getMinimalActionSet();
	ale.lives();
}

uint32_t EnvironmentBreakout::maxNumOfAgents() {
	return 1;
}

bool EnvironmentBreakout::onlyFinalReward() {
	return true;
}

void EnvironmentBreakout::reset(uint32_t numOfAgents) {
	ale.reset_game();
}

void EnvironmentBreakout::update() {}

void EnvironmentBreakout::getInputData(AGENT_ID agentID, std::vector<float>& data) {
	const ale::ALEScreen& screen = ale.getScreen();
	const ale::ALERAM& ram = ale.getRAM();

	//ale::ALEState state = ale.cloneState();
	//size_t arraySize = screen.arraySize();
	//ale::pixel_t* array = screen.getArray();
	//for(size_t i = 0; i < arraySize; i++) {
	//	data.push_back(static_cast<ale::pixel_t>(array[i]));
	//}
	
	//// Give whole ram as input. 
	//size_t arraySize = ram.size();
	//const ale::byte_t* array = ram.array();
	//for(size_t i = 0; i < arraySize; i++) {
	//	data.push_back(static_cast<ale::byte_t>(array[i]));
	//}

	// Give selected values of the ram as input. Source: https://www.codeproject.com/Articles/5271949/Learning-Breakout-From-RAM-Part-1
	int numOfValues = 13;
	int valuesIndices[13] = {70, 71, 72, 74, 75, 90, 94, 95, 99, 101, 103, 105, 119};
	// Gahter values. 
	for(size_t i = 0; i < numOfValues; i++) {
		data.push_back(ram.array()[valuesIndices[i]]);
	}

}

void EnvironmentBreakout::onAction(AGENT_ID agentID, float action) {
	//int actionInt = Maths::min(Maths::max(0, static_cast<int>(action)), 1);
	//ale::Action a = legal_actions[actionInt + 3];
	ale::Action a = legal_actions[Maths::min(Maths::max(0, static_cast<int>(action)), static_cast<int>(legal_actions.size() - 1))];
	reward = static_cast<float>(ale.act(a));
}

float EnvironmentBreakout::rewardAgent(AGENT_ID agentID) {
	return reward;
}

bool EnvironmentBreakout::gameOver() {
	return ale.game_over();
}
