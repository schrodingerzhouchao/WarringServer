#include "RSBrokerHelper.h"
#include "Base/message/CustomMessages.h"

using namespace sw::redis;
using namespace utility;

const std::string TEST_KEY_1 = "test:key1";
const std::string TEST_KEY_2 = "test:key2";

const std::string TEST_GROUP_1 = "group1";
const std::string TEST_GROUP_2 = "group2";

const std::string TEST_CONSUMER_1 = "consumer1";
const std::string TEST_CONSUMER_2 = "consumer2";

int main()
{
    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    ConnectionOptions connection_options;
    connection_options.host = "127.0.0.1";

    ConnectionPoolOptions pool_options;
    pool_options.size = 4;

    Redis redis(connection_options, pool_options);

    // Note!!! It seems need call it first
    broker::RSBrokerHelper::createGroup(redis, TEST_KEY_1, TEST_GROUP_1);

    for (size_t i = 0; i < 10; ++i)
    {
        message::test::CLoginMessage cLoginMsg;
        cLoginMsg.requestCode = 10;
        cLoginMsg.account = "zhangsan_" + std::to_string(i);
        cLoginMsg.password = "zhang_passwd_" + std::to_string(i * i);
        cLoginMsg.count();

        message::message_carrier::BaseMessage msgCarrier;
        msgCarrier.TYPE = "123321";
        msgCarrier.MESSAGE = cLoginMsg.serialize();

        // message::intranet::CLoginMessage intranetLoginMsg;
        // message::helper::internet2intranet(cLoginMsg, intranetLoginMsg, "192.168.123.456:" + std::to_string(i * (i + 1)));

        broker::RSBrokerHelper::push(redis, TEST_KEY_1, msgCarrier);
    }

    for (size_t i = 0; i < 10; ++i)
    {
        std::unordered_map<std::string, broker::ItemStreamType> result;

        auto len = broker::RSBrokerHelper::pull(redis, TEST_KEY_1, TEST_GROUP_1, TEST_CONSUMER_1, result, ">", 100_milliseconds, 1);

        if (len.has_value() && len.value() == 1)
        {
            message::message_carrier::BaseMessage msgCarrier;
            msgCarrier.FILL_2_THIS(result[TEST_KEY_1].at(0).second.value());
#if 1
            LOG_INFO << LOG_COUT << msgCarrier.TYPE;
            message::test::CLoginMessage result;
            result.deserialize(msgCarrier.MESSAGE.data());
            LOG_INFO << LOG_COUT << result.MSG_NAME << " " << result.account << " " << result.password << " " << result.GET_MILLISECONDS_SINCE_EPOCH();
#endif
        }
    }

    // std::this_thread::sleep_for(std::chrono::seconds(2));

#if 0
    std::vector<broker::GroupInfoType> groupsInfo;
    auto gInfoRes = broker::RSBrokerHelper::getGroupsInfo(redis, TEST_KEY_1, groupsInfo);
    if (gInfoRes.has_value() && gInfoRes.value() > 0)
    {
        std::cout << TEST_KEY_1 << "\'s groups info:\n";
        for (const auto &info : groupsInfo)
        {
            std::string groupName;
            size_t consumerNum;
            size_t pendingNum;
            std::string lastID;

            std::tie(std::ignore, groupName,
                     std::ignore, consumerNum,
                     std::ignore, pendingNum,
                     std::ignore, lastID,
                     std::ignore, std::ignore,
                     std::ignore, std::ignore) = info;

            std::cout << groupName << " " << consumerNum << " " << pendingNum << " " << lastID << std::endl;
        }
    }
#endif
}