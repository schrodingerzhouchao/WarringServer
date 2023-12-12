#ifndef __GAME_SERVER_HEARTBEAT_OPERATION_H__
#define __GAME_SERVER_HEARTBEAT_OPERATION_H__

#include "Base/utility/LoggerMessage.h"
#include "Base/net/TcpServer.h"
#include "Base/net/TcpConnection.h"

#include "GameServConf.h"
#include "SceneManager.h"

#include <string_view>

namespace game_server
{
    class GameHeartbeatOp
    {
    public:
        GameHeartbeatOp(const GameServConf &conf, net::TcpServer &tcpServ, std::string_view connID, game_scene::SceneManager &sceneManager)
            : mConf(conf),
              mTcpServer(tcpServ),
              mConnID(connID),
              mSceneManager(sceneManager)
        {
        }

        void checkHeartbeat()
        {
            std::shared_ptr<net::TcpConnection> tcpConnPtr;
            if (mTcpServer.getConnPtrRef(mConnID, tcpConnPtr))
            {
                auto now = utility::Time::millisecondsSinceEpoch();
                auto lastTimestamp = tcpConnPtr->getLastTimetamp();
                LOG_DEBUG << "NOW_TIMESTAMP: " << now << " LAST_TIMESTAMP: " << lastTimestamp;

                if ((now - lastTimestamp) >= (mConf.HEARTBEAT_TIMEOUT_IN_SECONDS * 1000))
                {
                    LOG_INFO << mConnID << " 's heartbeat is timeout, close it";
                    mSceneManager.removeScene(tcpConnPtr->getIPPort());
                    tcpConnPtr->activeClose(false);
                }
            }
        }

    private:
        const GameServConf &mConf;
        net::TcpServer &mTcpServer;
        std::string mConnID;
        game_scene::SceneManager &mSceneManager;
    };
}

#endif