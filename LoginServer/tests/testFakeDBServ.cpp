#include "Base/message/LoginMessage.h"
#include "Base/utility/Logger.h"
#include "Broker/RSBrokerHelper.h"

const std::string Login2DBStreamKey = "Warring:LoginServ:DBServ";
const std::string Login2DBStreamGroup = "LoginServ:DBServ:Group";

const std::string Login4DBStreamKey = "Warring:DBServ:LoginServ";
const std::string Login4DBStreamGroup = "DBServ:LoginServ:Group";

int main()
{
    auto twTimerPtr = std::make_shared<utility::timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<utility::logger::Logger>();

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);
    sw::redis::Redis redis("tcp://127.0.0.1:6379");

    broker::RSBrokerHelper::createGroup(redis, Login2DBStreamKey, Login2DBStreamGroup);
    broker::RSBrokerHelper::createGroup(redis, Login4DBStreamKey, Login4DBStreamGroup);

    for (;;)
    {
        std::unordered_map<std::string, broker::ItemStreamType> result;

        auto pullResult = broker::RSBrokerHelper::pull(redis, Login2DBStreamKey, Login2DBStreamGroup, "db_consumer", result, ">", 100_milliseconds);
        if (pullResult.has_value() && pullResult.value() == 1)
        {
            message::message_carrier::LoginMessage loginReqMsgCarrier;
            loginReqMsgCarrier.FILL_2_THIS(result[Login2DBStreamKey].at(0).second.value());
            LOG_DEBUG << "db_consumer read from " << Login2DBStreamKey << " TYPE " << loginReqMsgCarrier.TYPE
                      << " ID " << loginReqMsgCarrier.CLIENT_ID
                      << " ADDRESS " << loginReqMsgCarrier.CLIENT_ADDRESS
                      << " MESSAGE_LEN " << loginReqMsgCarrier.MESSAGE.length();
            message::LoginRequestMessage reqMsg;
            reqMsg.deserialize(loginReqMsgCarrier.MESSAGE.data());
            LOG_DEBUG << "MESSAGE " << reqMsg.MSG_NAME << " " << reqMsg.REQUEST_CODE << " " << reqMsg.ACCOUNT << " " << reqMsg.PASSWORD;

            message::LoginResponseMessage resMsg;
            resMsg.MSG_NAME = reqMsg.MSG_NAME;
            resMsg.RESPONSE_CODE = message::code::login::LOGIN_LOGIN_RESULT_SUCCESS;
            resMsg.TOKEN = "123456789";
            resMsg.count();

            message::message_carrier::LoginMessage loginResMsgCarrier;
            loginResMsgCarrier.TYPE = loginReqMsgCarrier.TYPE;
            loginResMsgCarrier.CLIENT_ID = loginReqMsgCarrier.CLIENT_ID;
            loginResMsgCarrier.MESSAGE = resMsg.serialize();

            broker::RSBrokerHelper::ack(redis, Login2DBStreamKey, Login2DBStreamGroup, result[Login2DBStreamKey].at(0).first);

            broker::RSBrokerHelper::push(redis, Login4DBStreamKey, loginResMsgCarrier);
        }
    }
}