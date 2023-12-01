#include "HTTPHelper.h"

#include <cpp-httplib/httplib.h>
#include <JSON/json.hpp>

#include "../trainingController/TrainingController.h"
#include "../TrainingConsts.h"

using namespace PLANS;
using namespace httplib;
using namespace nlohmann;

void HTTPHelper::postKeepAlive() {
    TrainingController* trainingController = TrainingController::getInstance();
    if(VM_ADDRESS == "") {
        trainingController->consoleOut("HTTPHelper::postKeepAlive: VM_ADDRESS is empty!", false);
        return;
    }
    Client cli(VM_ADDRESS, VM_PORT);
    cli.set_connection_timeout(0, 500000);  // 500 milliseconds
    cli.set_write_timeout(1, 0);            // 1 second
    auto res = cli.Get("/api/keepalive/tick?keepAliveType=ma_rlagent");
    if(!res) {
        trainingController->consoleOut("HTTPHelper::postKeepAlive: Error while sending keep alive.");
    }
}

void HTTPHelper::postRewardAverage(double average) {
    TrainingController* trainingController = TrainingController::getInstance();
    if(VM_ADDRESS == "") {
        trainingController->consoleOut("HTTPHelper::postRewardAverage: VM_ADDRESS is empty!");
        return;
    }
    //
    Client cli(VM_ADDRESS, VM_PORT);
    cli.set_connection_timeout(0, 500000);  // 500 milliseconds
    cli.set_write_timeout(1, 0);            // 1 second
    auto res = cli.Get(("/api/counter/set?counterType=ma_rewardAverage&count=" + std::to_string(average)).c_str());
    if(!res) {
        trainingController->consoleOut("HTTPHelper::postRewardAverage: Error while sending keep alive.", false);
    }
}

void HTTPHelper::postLastEpisodeFinished(uint32_t episodeCount) {
    TrainingController* trainingController = TrainingController::getInstance();
    if(VM_ADDRESS == "") {
        trainingController->consoleOut("HTTPHelper::postKeepAlive: VM_ADDRESS is empty!");
        return;
    }
    //
    Client cli(VM_ADDRESS, VM_PORT);
    cli.set_connection_timeout(0, 500000);  // 500 milliseconds
    cli.set_write_timeout(1, 0);            // 1 second
    auto res = cli.Get(("/api/counter/set?counterType=ma_lastEpisodeFinished&count=" + std::to_string(episodeCount)).c_str());
    if(!res) {
        trainingController->consoleOut("HTTPHelper::postKeepAlive: Error while sending keep alive.", false);
    }
}
