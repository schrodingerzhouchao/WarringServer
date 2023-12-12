#include "LoginServer.h"
#include "LoginMessageOp.h"

namespace login_server
{

    bool LoginServer::linkTimer(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr)
    {
        return mTcpServer.linkTimer(timerPtr);
    }

    bool LoginServer::registerTimerTask(size_t hours, size_t minutes, size_t seconds, int cyc, std::function<void()> func)
    {
        return mTcpServer.registerTimerTask(hours, minutes, seconds, cyc, func);
    }

    void LoginServer::setLongtimeTask(std::function<void()> func)
    {
        auto res = std::async(std::launch::async, func);
        mLongtimeTaskResults.push_back(std::move(res));
    }

    bool LoginServer::connFilterFunc(const net::socket::SocketAddr &addr)
    {
        auto ip = addr.getIP();
        if (mTcpServer.isInBlacklist(ip))
        {
            LOG_INFO << addr.getHostAndServiceNUM() << " is in blacklist";
            return false;
        }
        return true;
    }

    void LoginServer::connNameFunc(std::string connName)
    {
        LoginHeartbeatOp loginHeartbeatOp(mConf, mTcpServer, connName);
        // loginHeartbeatOp.registerHeartbeatTimeout();

        mTcpServer.registerTimerTask(0, 0, mConf.HEARTBEAT_TIMEOUT_IN_SECONDS, -1, std::bind(&LoginHeartbeatOp::checkHeartbeat, loginHeartbeatOp));
    }

    void LoginServer::messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr)
    {
        mLoginMessageOp.messageFunc(tcpConnptr);
    }

    void LoginServer::loginStreamConsumerFunc()
    {
        while (true)
        {
            std::unordered_map<std::string, broker::ItemStreamType> result;
            auto pullResult = broker::RSBrokerHelper::pull(mRedis, mConf.BROKER_LOGIN_KEY, mConf.BROKER_LOGIN_GROUP, "LoginStreamConsumer", result, ">", mConf.STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS);
            if (pullResult.has_value() && pullResult.value() == 1)
            {
                message::message_carrier::LoginMessage loginMsgCarrier;
                loginMsgCarrier.FILL_2_THIS(result[mConf.BROKER_LOGIN_KEY].at(0).second.value());

                LOG_INFO << "get service: " << loginMsgCarrier.SERVICE;

                std::shared_ptr<net::TcpConnection> connPtr;

                if (mTcpServer.getConnPtrRef(loginMsgCarrier.CLIENT_ID, connPtr) && connPtr)
                {
                    connPtr->putToBuffer(loginMsgCarrier.MESSAGE);

                    broker::RSBrokerHelper::ack(mRedis, mConf.BROKER_LOGIN_KEY, mConf.BROKER_LOGIN_GROUP, result[mConf.BROKER_LOGIN_KEY].at(0).first);
                }
                else
                {
                    LOG_WARN << loginMsgCarrier.CLIENT_ID << " has been lost";
                    broker::RSBrokerHelper::del(mRedis, mConf.BROKER_LOGIN_KEY, result[mConf.BROKER_LOGIN_KEY].at(0).first);
                }
            }
        }
    }

}