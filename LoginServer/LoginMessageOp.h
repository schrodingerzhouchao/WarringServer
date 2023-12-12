#ifndef __LOGIN_SERVER_MESSAGE_OPERATION_H__
#define __LOGIN_SERVER_MESSAGE_OPERATION_H__

#include "Base/message/LoginMessage.h"
#include "Base/net/TcpConnection.h"
#include "Base/net/TcpServer.h"
#include "Base/utility/LoggerMessage.h"

#include "Broker/RSBrokerHelper.h"

#include "LoginServConf.h"

namespace login_server
{
    class LoginMessageOp
    {
    public:
        LoginMessageOp(sw::redis::Redis &redis, const LoginServConf &conf, net::TcpServer &tcpServ)
            : mRedis(redis),
              mConf(conf),
              mTcpServer(tcpServ)
        {
        }

        template <typename MESSAGE>
        void registerMessageType(std::thread::id tid, std::string_view msgName)
        {
            mMessageTypes[tid].emplace(msgName, std::make_unique<MESSAGE>());
        }

        void messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr);

        auto &getMessageTypes() { return mMessageTypes; }
#if 0
        static void internet2intranet(const message::internet::CLoginMessage &internetMsg, message::intranet::CLoginMessage &intranetMsg,
                                      const std::string &id, const std::string &address);

        static void intranet2internet(const message::intranet::SLoginMessage &intranetMsg, message::internet::SLoginMessage &internetMsg,
                                      const std::string &msgName, const std::string &token = "");
#endif

#if 0
        static void message2carrier(message::LoginRequestMessage &loginRequestMsg, message::message_carrier::LoginRequestMessage &loginRequestMsgCarrier,
                                    const std::string &id, const std::string &address);

        static void message2carrier(message::LoginResponseMessage &loginResponseMsg, message::message_carrier::LoginResponseMessage &loginResponseMsgCarrier,
                                    const std::string &id);
#endif

    private:
        sw::redis::Redis &mRedis;

        const LoginServConf &mConf;

        // LoginHeartbeatOp &mLoginHeartbeatOp;

        net::TcpServer &mTcpServer;

        // <tid ,<message's MSG_NAME, unique_ptr<baseMsg>>>
        // std::unordered_map<std::thread::id, std::unordered_map<std::string, std::unique_ptr<message::internet::BaseMessage>>> mMessageTypes;
        std::unordered_map<std::thread::id, std::unordered_map<std::string, std::unique_ptr<message::TimestampMessage>>> mMessageTypes;
    };

}

#endif