#ifndef __GAME_SERVER_MESSAGE_OPERATION_H__
#define __GAME_SERVER_MESSAGE_OPERATION_H__

#include "Base/message/GameMessage.h"
#include "Base/net/TcpConnection.h"
#include "Base/net/TcpServer.h"
#include "Base/utility/LoggerMessage.h"

#include "Broker/RSBrokerHelper.h"

#include "GameServConf.h"
#include "SceneManager.h"

namespace game_server
{
    class GameMessageOp
    {
    public:
        GameMessageOp(sw::redis::Redis &brokerRedis, sw::redis::Redis &servRegRedis, const GameServConf &conf, net::TcpServer &tcpServ, game_scene::SceneManager& sceneManager)
            : mBrokerRedis(brokerRedis),
              mConf(conf),
              mTcpServer(tcpServ),
              mSceneManager(sceneManager)
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
        sw::redis::Redis &mBrokerRedis;

        const GameServConf &mConf;

        // LoginHeartbeatOp &mLoginHeartbeatOp;

        net::TcpServer &mTcpServer;

        game_scene::SceneManager& mSceneManager;

        // <tid ,<message's MSG_NAME, unique_ptr<baseMsg>>>
        // std::unordered_map<std::thread::id, std::unordered_map<std::string, std::unique_ptr<message::internet::BaseMessage>>> mMessageTypes;
        std::unordered_map<std::thread::id, std::unordered_map<std::string, std::unique_ptr<message::TimestampMessage>>> mMessageTypes;
    };
}

#endif