#include "GatewayMessageOp.h"

namespace gateway_server
{
    void GatewayMessageOp::messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr)
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
#if 0
                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::TimestampMessage))
                {
                    message::TimestampMessage *bMsg = mMessageTypes[tid].at(msgType).get();
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << bMsg->MSG_NAME
                             << " [MILLISECONDS SINCE EPOCH]" << bMsg->GET_MILLISECONDS_SINCE_EPOCH();
                }
#endif
                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::EnterGameRequestMessage))
                {
                    message::EnterGameRequestMessage *enterMsg = dynamic_cast<message::EnterGameRequestMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << enterMsg->MSG_NAME
                             << " [MESSAGE CODE]" << enterMsg->REQUEST_CODE << " [ACCOUNT]" << enterMsg->ACCOUNT
                             << " [CHARACTER]" << enterMsg->CHARACTER << " [MILLISECONDS SINCE EPOCH]" << enterMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(enterMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::message_carrier::GatewayMessage gatewayMsgCarrier;
                    gatewayMsgCarrier.TYPE = enterMsg->MSG_NAME;
                    gatewayMsgCarrier.CLIENT_ID = tcpConnptr->getName();
                    gatewayMsgCarrier.MESSAGE = msg;

                    // broker::RSBrokerHelper::push(mRedis, mConf.BROKER_LOGIN_2_DB_KEY, loginReqMsgCarrier);
                }
                else if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::ChatRequestMessage))
                {
                    message::ChatRequestMessage *chatMsg = dynamic_cast<message::ChatRequestMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << chatMsg->MSG_NAME
                             << " [MESSAGE CODE]" << chatMsg->REQUEST_CODE
                             << " [ACCOUNT]" << chatMsg->ACCOUNT
                             << " [CHANNEL]" << chatMsg->CHANNEL
                             << " [NOTE] " << chatMsg->NOTE
                             << " [MILLISECONDS SINCE EPOCH]" << chatMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(chatMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::message_carrier::GatewayMessage gatewayMsgCarrier;
                    gatewayMsgCarrier.TYPE = chatMsg->MSG_NAME;
                    gatewayMsgCarrier.CLIENT_ID = tcpConnptr->getName();
                    gatewayMsgCarrier.MESSAGE = msg;

                    //broker::RSBrokerHelper::push(mRedis, mConf.BROKER_LOGIN_2_DB_KEY, loginReqMsgCarrier);
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
                    repHBMsg.MSG_NAME = mConf.MESSAGE_GATEWAY_HEARTBEAT_MSG;
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
}