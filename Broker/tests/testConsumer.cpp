#include "Base/utility/Timer.h"
#include "Base/utility/Logger.h"
#include "Base/message/CustomMessages.h"
#include "RSBrokerHelper.h"
#include <iostream>
#include <thread>

using namespace sw::redis;
using namespace utility;
using namespace std;

const std::string TEST_KEY_1 = "test:key1";
const std::string TEST_KEY_2 = "test:key2";

const std::string TEST_GROUP_1 = "group1";
const std::string TEST_GROUP_2 = "group2";

const std::string TEST_CONSUMER_1 = "consumer1";
const std::string TEST_CONSUMER_2 = "consumer2";

thread_local size_t num = 0;

void consumer(sw::redis::Redis &redis, const std::string &key, const std::string &group, const std::string &consumer)
{
    for (;;)
    {
        ++num;

        std::unordered_map<std::string, broker::ItemStreamType> result;

        if (consumer == TEST_CONSUMER_1)
        {
            auto pullResult = broker::RSBrokerHelper::pull(redis, key, group, consumer, result, ">", 100_milliseconds);
            if (pullResult.has_value() && pullResult.value() > 0)
            {
                LOG_DEBUG << LOG_COUT << "pull size: " << pullResult.value();

                message::message_carrier::BaseMessage msgCarrier;
                msgCarrier.FILL_2_THIS(result[key].at(0).second.value());
                message::test::CLoginMessage resultMsg;

                resultMsg.deserialize(msgCarrier.MESSAGE.data());

                LOG_DEBUG << LOG_COUT << consumer << " read from " << key << " ****normal**** "
                          << " TYPE " << msgCarrier.TYPE << " ** "
                          << resultMsg.MSG_NAME << " ** " << resultMsg.requestCode
                          << " ** " << resultMsg.account << " ** " << resultMsg.password;

                // if (consumer == TEST_CONSUMER_1)
                if (num % 2 == 0)
                {
                    if (broker::RSBrokerHelper::ack(redis, key, group, result[key].at(0).first) > 0)
                    {
                        LOG_DEBUG << LOG_COUT << consumer << " ****normal**** ack " << result[key].at(0).first;
                    }
                }
            }
        }
#if 1
        else
        {
            // bad usage, just for test, no need to do like this in real
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::vector<broker::PendingItemType> pendingItems;
            auto pendingResult = broker::RSBrokerHelper::getPending(redis, key, group, consumer, 1, pendingItems);
            if (pendingResult.has_value() && pendingResult.value() > 0)
            {
                auto id = std::get<0>(pendingItems[0]);
                LOG_DEBUG << LOG_COUT << consumer << " read pending items, id: " << id;
                auto previousID = broker::RSBrokerHelper::getPreviousID(redis, key, id);
                LOG_DEBUG << LOG_COUT << consumer << " get previous id " << previousID << " of " << id;

                auto pullResult = broker::RSBrokerHelper::pull(redis, key, group, consumer, result, previousID, 1_milliseconds);
                if (pullResult.has_value() && pullResult.value() == 1)
                {
                    message::test::CLoginMessage intranetLoginMsg;
                    message::message_carrier::BaseMessage msgCarrier;

                    msgCarrier.FILL_2_THIS(result[key].at(0).second.value());
                    intranetLoginMsg.deserialize(msgCarrier.MESSAGE.data());

                    LOG_DEBUG << LOG_COUT << consumer << " read from " << key << " ****fill-in**** "
                              << intranetLoginMsg.MSG_NAME
                              << " ** " << intranetLoginMsg.account << " ** " << intranetLoginMsg.password;

                    if (broker::RSBrokerHelper::ack(redis, key, group, result[key].at(0).first) > 0)
                    {
                        LOG_DEBUG << LOG_COUT << consumer << " ****fill-in**** ack " << result[key].at(0).first;
                    }
                }
            }
        }
#endif
    }
}

int main()
{
    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    ConnectionOptions connection_options;
    connection_options.host = "127.0.0.1";

    ConnectionPoolOptions pool_options;
    pool_options.size = 8;

    auto redis = Redis(connection_options, pool_options);

    broker::RSBrokerHelper::createGroup(redis, TEST_KEY_1, TEST_GROUP_1);
    // broker::RSBrokerHelper::createGroup(redis, TEST_KEY_1, TEST_GROUP_2);
    // broker::RSBrokerHelper::createGroup(redis, TEST_KEY_2, TEST_GROUP_1);
    // broker::RSBrokerHelper::createGroup(redis, TEST_KEY_2, TEST_GROUP_2);

    std::thread t111(consumer, std::ref(redis), std::ref(TEST_KEY_1), std::ref(TEST_GROUP_1), std::ref(TEST_CONSUMER_1));

    std::thread t112(consumer, std::ref(redis), std::ref(TEST_KEY_1), std::ref(TEST_GROUP_1), std::ref(TEST_CONSUMER_2));
#if 0    
    std::thread t121(consumer, std::ref(redis), std::ref(TEST_KEY_1), std::ref(TEST_GROUP_2), std::ref(TEST_CONSUMER_1));
    std::thread t122(consumer, std::ref(redis), std::ref(TEST_KEY_1), std::ref(TEST_GROUP_2), std::ref(TEST_CONSUMER_2));

    std::thread t211(consumer, std::ref(redis), std::ref(TEST_KEY_2), std::ref(TEST_GROUP_1), std::ref(TEST_CONSUMER_1));
    std::thread t212(consumer, std::ref(redis), std::ref(TEST_KEY_2), std::ref(TEST_GROUP_1), std::ref(TEST_CONSUMER_2));
    std::thread t221(consumer, std::ref(redis), std::ref(TEST_KEY_2), std::ref(TEST_GROUP_2), std::ref(TEST_CONSUMER_1));
    std::thread t222(consumer, std::ref(redis), std::ref(TEST_KEY_2), std::ref(TEST_GROUP_2), std::ref(TEST_CONSUMER_2));
#endif

    t111.join();

    t112.join();
#if 0
    t121.join();
    t122.join();

    t211.join();
    t212.join();
    t221.join();
    t222.join();
#endif

    return 0;
}
