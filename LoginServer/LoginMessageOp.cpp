#include "LoginMessageOp.h"

namespace login_server
{
#if 0
    void LoginMessageOp::messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr)
    {
        if (tcpConnptr == nullptr)
        {
            return;
        }

        message::internet::MESSAGE_LENGTH_TYPE len = 0;
        auto tid = std::this_thread::get_id();

        // try to get a copy of message::internet::BaseMessage's MSG_LEN
        auto msg = tcpConnptr->tryGet(sizeof(len), false);
        if (msg.empty())
        {
            return;
        }

        ::memcpy(&len, msg.data(), sizeof(len));

        // std::cout << len <<std::endl;

        if (len >= sizeof(len))
        {
            // try to fetch a particular message
            msg = tcpConnptr->tryGet(len, true);
            if (!msg.empty())
            {
                message::internet::BaseMessage baseMsg;
                baseMsg.deserialize(msg.data());
                auto msgType = baseMsg.MSG_NAME;

                // check thread id is registered
                if (mMessageTypes.count(tid) == 0)
                {
                    LOG_FATAL << "NO SUCH THREAD ID " << tid;
                    return;
                }
                // check message type is registered
                if (mMessageTypes[tid].count(msgType) == 0)
                {
                    LOG_ERROR << "NO SUCH MESSAGE TYPE " << msgType;
                    return;
                }

                mMessageTypes[tid][msgType]->deserialize(msg.data());

                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::internet::BaseMessage))
                {
                    message::internet::BaseMessage *bMsg = mMessageTypes[tid].at(msgType).get();
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << bMsg->MSG_NAME
                             << " [MILLISECONDS SINCE EPOCH]" << bMsg->GET_MILLISECONDS_SINCE_EPOCH();
                }

                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::internet::CLoginMessage))
                {
                    message::internet::CLoginMessage *loginMsg = dynamic_cast<message::internet::CLoginMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << loginMsg->MSG_NAME
                             << " [MESSAGE CODE]" << loginMsg->requestCode << " [ACCOUNT]" << loginMsg->account
                             << " [PASSWORD]" << loginMsg->password << " [MILLISECONDS SINCE EPOCH]" << loginMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(loginMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::intranet::CLoginMessage intranetLoginMsg;
                    internet2intranet(*loginMsg, intranetLoginMsg, tcpConnptr->getName(), tcpConnptr->getIPPort());

                    broker::RSBrokerHelper::push(mRedis, mConf.BROKER_LOGIN_2_DB_KEY, intranetLoginMsg);
                }
                else if (typeid(*mMessageTypes[tid].at(msgType)) == typeid(message::internet::HeartBeatPacket))
                {
                    message::internet::HeartBeatPacket *hbMsg = dynamic_cast<message::internet::HeartBeatPacket *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << hbMsg->MSG_NAME
                             << " [MILLISECONDS SINCE EPOCH]" << hbMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(hbMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    // TODO control heartbeat frequency? if (mTcpServer.getEventLoop(tid)->isIntervalOut())

                    // reply heartbeat to client
                    message::internet::HeartBeatPacket repHBMsg;
                    repHBMsg.MSG_NAME = mConf.MESSAGE_LOGIN_HEARTBEAT_MSG;
                    repHBMsg.count();
                    auto data = repHBMsg.serialize();
                    tcpConnptr->send(data);
                }
                else
                {
                    // TODO restrict the client who send dangerous messages
                    LOG_INFO << tcpConnptr->getName() << " is undefined message type";
                }
            }
        }
    }
#endif
#if 0
    void LoginMessageOp::internet2intranet(const message::internet::CLoginMessage &internetMsg, message::intranet::CLoginMessage &intranetMsg,
                                           const std::string &id, const std::string &address)
    {
        intranetMsg.id = id;
        intranetMsg.type = message::code::login::MESSAGE_TYPE_LOGIN;

        intranetMsg.code = std::to_string(internetMsg.requestCode);

        intranetMsg.account = internetMsg.account;
        intranetMsg.password = internetMsg.account;
        intranetMsg.loginAddress = address;
    }

    void LoginMessageOp::intranet2internet(const message::intranet::SLoginMessage &intranetMsg, message::internet::SLoginMessage &internetMsg,
                                           const std::string &msgName, const std::string &token)
    {
        internetMsg.MSG_NAME = msgName;
        internetMsg.requestResult = std::stoi(intranetMsg.code);
        internetMsg.token = token;
    }
#endif

    void LoginMessageOp::messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr)
    {
        if (tcpConnptr == nullptr)
        {
            return;
        }

        message::MESSAGE_LENGTH_TYPE len = 0;
        auto tid = std::this_thread::get_id();

        // try to get a copy of message::internet::BaseMessage's MSG_LEN
        auto msg = tcpConnptr->tryGet(sizeof(len), false);
        if (msg.empty())
        {
            return;
        }

        ::memcpy(&len, msg.data(), sizeof(len));

        // std::cout << len <<std::endl;

        if (len >= sizeof(len))
        {
            // try to fetch a particular message
            msg = tcpConnptr->tryGet(len, true);
            if (!msg.empty())
            {
                message::TimestampMessage baseMsg;
                baseMsg.deserialize(msg.data());
                auto msgType = baseMsg.MSG_NAME;

                // check thread id is registered
                if (mMessageTypes.count(tid) == 0)
                {
                    LOG_FATAL << "NO SUCH THREAD ID " << tid;
                    return;
                }
                // check message type is registered
                if (mMessageTypes[tid].count(msgType) == 0)
                {
                    LOG_ERROR << "NO SUCH MESSAGE TYPE " << msgType;
                    return;
                }

                mMessageTypes[tid][msgType]->deserialize(msg.data());

                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::TimestampMessage))
                {
                    message::TimestampMessage *bMsg = mMessageTypes[tid].at(msgType).get();
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << bMsg->MSG_NAME
                             << " [MILLISECONDS SINCE EPOCH]" << bMsg->GET_MILLISECONDS_SINCE_EPOCH();
                }

                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::LoginRequestMessage))
                {
                    message::LoginRequestMessage *loginMsg = dynamic_cast<message::LoginRequestMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << loginMsg->MSG_NAME
                             << " [MESSAGE CODE]" << loginMsg->REQUEST_CODE << " [ACCOUNT]" << loginMsg->ACCOUNT
                             << " [PASSWORD]" << loginMsg->PASSWORD << " [MILLISECONDS SINCE EPOCH]" << loginMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(loginMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::message_carrier::LoginMessage loginMsgCarrier;
                    loginMsgCarrier.TYPE = loginMsg->MSG_NAME;
                    loginMsgCarrier.SERVICE = mConf.BROKER_LOGIN_KEY;
                    loginMsgCarrier.NEXT_SERVICE = mConf.BROKER_DB_KEY;
                    loginMsgCarrier.CLIENT_ID = tcpConnptr->getName();
                    loginMsgCarrier.CLIENT_ADDRESS = tcpConnptr->getIPPort();
                    loginMsgCarrier.MESSAGE = msg;

                    broker::RSBrokerHelper::push(mRedis, mConf.BROKER_DB_KEY, loginMsgCarrier);
                }
                else if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::LoginCharacterRequestMessage))
                {
                    message::LoginCharacterRequestMessage *loginCharacterMsg = dynamic_cast<message::LoginCharacterRequestMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << loginCharacterMsg->MSG_NAME
                             << " [MESSAGE CODE]" << loginCharacterMsg->REQUEST_CODE
                             << " [ACCOUNT]" << loginCharacterMsg->ACCOUNT
                             << " [PASSWORD]" << loginCharacterMsg->PASSWORD
                             << " [CHARACTER_NAME] " << loginCharacterMsg->CHARACTER_NAME
                             << " [CHARACTER_GENDER] " << loginCharacterMsg->CHARACTER_GENDER
                             << " [CHARACTER_OCCUPTION] " << loginCharacterMsg->CHARACTER_OCCUPTION
                             << " [CHARACTER_STATE] " << loginCharacterMsg->CHARACTER_STATE
                             << " [MILLISECONDS SINCE EPOCH]" << loginCharacterMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(loginCharacterMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::message_carrier::LoginMessage loginMsgCarrier;
                    loginMsgCarrier.TYPE = loginCharacterMsg->MSG_NAME;
                    loginMsgCarrier.SERVICE=mConf.BROKER_LOGIN_KEY;
                    loginMsgCarrier.NEXT_SERVICE = mConf.BROKER_DB_KEY;
                    loginMsgCarrier.CLIENT_ID = tcpConnptr->getName();
                    loginMsgCarrier.CLIENT_ADDRESS = tcpConnptr->getIPPort();
                    loginMsgCarrier.MESSAGE = msg;

                    broker::RSBrokerHelper::push(mRedis, mConf.BROKER_DB_KEY, loginMsgCarrier);
                }
                else if (typeid(*mMessageTypes[tid].at(msgType)) == typeid(message::HeartBeatPacket))
                {
                    message::HeartBeatPacket *hbMsg = dynamic_cast<message::HeartBeatPacket *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << hbMsg->MSG_NAME
                             << " [MILLISECONDS SINCE EPOCH]" << hbMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(hbMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    // TODO control heartbeat frequency? if (mTcpServer.getEventLoop(tid)->isIntervalOut())

                    // reply heartbeat to client
                    message::HeartBeatPacket repHBMsg;
                    repHBMsg.MSG_NAME = mConf.MESSAGE_LOGIN_HEARTBEAT_MSG;
                    repHBMsg.count();
                    auto data = repHBMsg.serialize();
                    tcpConnptr->send(data);
                }
                else
                {
                    // TODO restrict the client who send dangerous messages
                    LOG_INFO << tcpConnptr->getName() << " is undefined message type";
                }
            }
        }
    }

#if 0
    void LoginMessageOp::message2carrier(message::LoginRequestMessage &loginRequestMsg, message::message_carrier::LoginRequestMessage &loginRequestMsgCarrier,
                                         const std::string &id, const std::string &address)
    {

        loginRequestMsgCarrier.TYPE = message::code::login::MESSAGE_TYPE_LOGIN;
        //loginRequestMsgCarrier.SERVICE = mConf.BROKER_LOGIN_KEY;
        loginRequestMsgCarrier.CLIENT_ID = id;
        loginRequestMsgCarrier.CLIENT_ADDRESS = address;
        loginRequestMsg.count();
        loginRequestMsgCarrier.MESSAGE = loginRequestMsg.serialize();
    }

    void LoginMessageOp::message2carrier(message::LoginResponseMessage &loginResponseMsg, message::message_carrier::LoginResponseMessage &loginResponseMsgCarrier,
                                         const std::string &id)
    {
        loginResponseMsgCarrier.TYPE = message::code::login::MESSAGE_TYPE_LOGIN;
        loginResponseMsgCarrier.CLIENT_ID = id;
        loginResponseMsg.count();
        loginResponseMsgCarrier.MESSAGE = loginResponseMsg.serialize();
    }
#endif
}