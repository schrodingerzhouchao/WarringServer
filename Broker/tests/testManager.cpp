#include "utility/Timer.h"
#include "utility/Logger.h"
#include "RSBrokerManager.h"

using namespace utility;

const std::string TEST_KEY_1 = "test:key1";
const std::string TEST_KEY_2 = "test:key2";

// const std::string TEST_GROUP_1 = "group1";
// const std::string TEST_GROUP_2 = "group2";

// const std::string TEST_CONSUMER_1 = "consumer1";
// const std::string TEST_CONSUMER_2 = "consumer2";

int main()
{
    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    LOG_DEBUG << "TEST RSBROKER MANAGER";

    sw::redis::ConnectionOptions connection_options;
    connection_options.host = "127.0.0.1";

    broker::RSBrokerManager brokerManager(connection_options);

    brokerManager.registerKey(TEST_KEY_1);
    //brokerManager.registerKey(TEST_KEY_2);

    brokerManager.setNapSecond(100);
    brokerManager.setTolerantTime(50000);
    brokerManager.setTransferTime(3000);
    brokerManager.setMaxDelivered(3);

    brokerManager.run();

    return 0;
}