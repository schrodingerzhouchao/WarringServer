#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

#include "LoginServConf.h"
#include "LoginMessageOp.h"
#include "LoginHeartbeatOp.h"

#include "Base/net/TcpServer.h"
#include "Base/message/CustomMessages.h"
#include "Base/utility/LoggerMessage.h"
#include "Broker/RSBrokerHelper.h"

namespace login_server
{
    class LoginServer
    {
    public:
        LoginServer(const LoginServConf &loginServConf, net::socket::SocketAddr sa, std::string servName = "LoginSever")
            : mConf(loginServConf),
              mTcpServer(sa, mConf.SERVER_IO_THREAD_NUM, servName, mConf.SERVER_SAFE_HASH_BUCKET_SIZE),
              mRedis(mConf.getBrokerConnOpts(), mConf.getBrokerConnPoolOpts()),
              mLoginMessageOp(mRedis, mConf, mTcpServer)
        {
            mEventLoopTIDs = mTcpServer.getEventLoopThreadIDs();
            for (auto tid : mEventLoopTIDs)
            {
                // register internet message type
                mLoginMessageOp.registerMessageType<message::TimestampMessage>(tid, mConf.MESSAGE_LOGIN_BASE_MSG);
                mLoginMessageOp.registerMessageType<message::HeartBeatPacket>(tid, mConf.MESSAGE_LOGIN_HEARTBEAT_MSG);
                mLoginMessageOp.registerMessageType<message::LoginRequestMessage>(tid, mConf.MESSAGE_LOGIN_LOGIN_MSG);
                mLoginMessageOp.registerMessageType<message::LoginCharacterRequestMessage>(tid, mConf.MESSAGE_LOGIN_LOGIN_CHARACTER_MSG);
            }
#if 0
            mTcpServer.setTimeoutParam(mConf.EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS,
                                       mConf.EVENTLOOP_TIMEOUT_IN_SECONDS,
                                       mConf.EVENTLOOP_RESET_THRESHOLD_IN_SECONDS);
#endif
            broker::RSBrokerHelper::createGroup(mRedis, mConf.BROKER_DB_KEY, mConf.BROKER_DB_GROUP);
            broker::RSBrokerHelper::createGroup(mRedis, mConf.BROKER_LOGIN_KEY, mConf.BROKER_LOGIN_GROUP);
        }

        LoginServer(const LoginServer &) = delete;
        LoginServer &operator=(const LoginServer &) = delete;
        LoginServer(LoginServer &&) = delete;
        LoginServer &operator=(LoginServer &&) = delete;

        ~LoginServer()
        {
            // useless
            for (auto &longtimeTaskResult : mLongtimeTaskResults)
            {
                longtimeTaskResult.get();
            }
        }

        bool linkTimer(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr);

        bool registerTimerTask(size_t hours, size_t minutes, size_t seconds, int cyc, std::function<void()> func);

        void start(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr)
        {
            if (!linkTimer(timerPtr))
            {
                LOG_WARN << "should link the TimeWheelTimer";
            }

            initConnBlacklist();

            mTcpServer.setConnFilterFunc(std::bind(&LoginServer::connFilterFunc, this, std::placeholders::_1));
            mTcpServer.setConnNameFunc(std::bind(&LoginServer::connNameFunc, this, std::placeholders::_1));
            mTcpServer.setMessageFunc(std::bind(&LoginServer::messageFunc, this, std::placeholders::_1));
            setLongtimeTask(std::bind(&LoginServer::loginStreamConsumerFunc, this));

            mTcpServer.start();
        }

    private:
        void initConnBlacklist() { mTcpServer.addToBlacklist(mConf.CONNECTION_BLACKLIST); }

        bool connFilterFunc(const net::socket::SocketAddr &addr);

        void connNameFunc(std::string connName);

        void messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr);

        // run a thread with longtime task asynchronously
        void setLongtimeTask(std::function<void()> func);

        // read message from db serv through broker
        void loginStreamConsumerFunc();

    private:
        const LoginServConf &mConf;

        net::TcpServer mTcpServer;

        std::vector<std::thread::id> mEventLoopTIDs;

        sw::redis::Redis mRedis;

        // LoginHeartbeatOp mLoginHeartbeatOp;

        LoginMessageOp mLoginMessageOp;

        std::vector<std::future<void>> mLongtimeTaskResults;
    };
}

#endif