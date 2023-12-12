#ifndef __GATEWAY_SERVER_MESSAGE_OPERATION_H__
#define __GATEWAY_SERVER_MESSAGE_OPERATION_H__

#include "Base/message/GatewayMessage.h"
#include "Base/net/TcpConnection.h"
#include "Base/net/TcpServer.h"
#include "Base/utility/LoggerMessage.h"

#include "Broker/RSBrokerHelper.h"

#include "GatewayServConf.h"

namespace gateway_server
{
    class GatewayMessageOp
    {
    public:
        GatewayMessageOp(sw::redis::Redis &redis, const GatewayServConf &conf, net::TcpServer &tcpServ)
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

    private:
        sw::redis::Redis &mRedis;

        const GatewayServConf &mConf;

        // LoginHeartbeatOp &mLoginHeartbeatOp;

        net::TcpServer &mTcpServer;

        // <tid ,<message's MSG_NAME, unique_ptr<baseMsg>>>
        // std::unordered_map<std::thread::id, std::unordered_map<std::string, std::unique_ptr<message::internet::BaseMessage>>> mMessageTypes;
        std::unordered_map<std::thread::id, std::unordered_map<std::string, std::unique_ptr<message::TimestampMessage>>> mMessageTypes;
    };
}

#endif