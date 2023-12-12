#ifndef __LOGIN_SERVER_HEARTBEAT_OPERATION_H__
#define __LOGIN_SERVER_HEARTBEAT_OPERATION_H__

#include "Base/utility/LoggerMessage.h"
#include "LoginServConf.h"
#include "Base/net/TcpServer.h"
#include "Base/net/TcpConnection.h"

#include <string_view>

namespace login_server
{
    class LoginHeartbeatOp
    {
    public:
        LoginHeartbeatOp(const LoginServConf &conf, net::TcpServer &tcpServ, std::string_view connID)
            : mConf(conf),
              mTcpServer(tcpServ),
              mConnID(connID)
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
                    tcpConnPtr->activeClose(false);
                }
            }
        }

    private:
        const LoginServConf &mConf;
        net::TcpServer &mTcpServer;
        std::string mConnID;
    };
}

#endif