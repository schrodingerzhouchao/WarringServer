#ifndef __GATEWAY_SERVER_H__
#define __GATEWAY_SERVER_H__

#include "GatewayServConf.h"
#include "GatewayMessageOp.h"
#include "GatewayHeartbeatOp.h"

#include "Base/net/TcpServer.h"
#include "Base/message/CustomMessages.h"
#include "Base/utility/LoggerMessage.h"
#include "Base/utility/Cmd.h"
#include "Broker/RSBrokerHelper.h"

namespace gateway_server
{
    class GatewayServer
    {
    public:
        GatewayServer(GatewayServConf &gatewayServConf, net::socket::SocketAddr sa, std::string servName = "GatewaySever")
            : mConf(gatewayServConf),
              mTcpServer(sa, mConf.SERVER_IO_THREAD_NUM, servName, mConf.SERVER_SAFE_HASH_BUCKET_SIZE),
              mServRegRedis(mConf.getRegistryConnOpts(), mConf.getRegistryConnPoolOpts()),
              mSub(mServRegRedis.subscriber()),
              mBrokerRedis(mConf.getBrokerConnOpts(), mConf.getBrokerConnPoolOpts()),
              mGatewayMessageOp(mBrokerRedis, mConf, mTcpServer)
        {
            mEventLoopTIDs = mTcpServer.getEventLoopThreadIDs();
            for (auto tid : mEventLoopTIDs)
            {
                // register internet message type
                mGatewayMessageOp.registerMessageType<message::TimestampMessage>(tid, mConf.MESSAGE_GATEWAY_BASE_MSG);
                mGatewayMessageOp.registerMessageType<message::HeartBeatPacket>(tid, mConf.MESSAGE_GATEWAY_HEARTBEAT_MSG);
                mGatewayMessageOp.registerMessageType<message::EnterGameRequestMessage>(tid, mConf.MESSAGE_GATEWAY_ENTER_GANE_MSG);
            }

            mPort = sa.getPort();
            utility::Cmd cmd;
            mIP = cmd("ifconfig ens33 | awk 'NR==2{print $2}'");
        }

        GatewayServer(const GatewayServer &) = delete;
        GatewayServer &operator=(const GatewayServer &) = delete;
        GatewayServer(GatewayServer &&) = delete;
        GatewayServer &operator=(GatewayServer &&) = delete;

        ~GatewayServer()
        {
            // useless
            for (auto &longtimeTaskResult : mLongtimeTaskResults)
            {
                longtimeTaskResult.get();
            }
        }

        bool initKey();

        void registerService();

        bool linkTimer(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr);

        bool registerTimerTask(size_t hours, size_t minutes, size_t seconds, int cyc, std::function<void()> func);

        void start(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr)
        {
            if (!linkTimer(timerPtr))
            {
                LOG_WARN << "should link the TimeWheelTimer";
            }

            initConnBlacklist();

            mTcpServer.setConnFilterFunc(std::bind(&GatewayServer::connFilterFunc, this, std::placeholders::_1));
            mTcpServer.setConnNameFunc(std::bind(&GatewayServer::connNameFunc, this, std::placeholders::_1));
            mTcpServer.setMessageFunc(std::bind(&GatewayServer::messageFunc, this, std::placeholders::_1));
            setLongtimeTask(std::bind(&GatewayServer::gatewayStreamConsumerFunc, this));
            setLongtimeTask(std::bind(&GatewayServer::PSConsumerFunc, this));

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
        void gatewayStreamConsumerFunc();

        // redis pub/sub func
        void onMessageFunc(std::string channnel, std::string msg);
        void onPMessageFunc(std::string pattern, std::string channel, std::string msg);
        void onMetaFunc(sw::redis::Subscriber::MsgType type, sw::redis::OptionalString channel, long long num);

        // redis pub/sub consume messages loop
        void PSConsumerFunc();

    private:
        GatewayServConf &mConf;

        net::TcpServer mTcpServer;

        std::vector<std::thread::id> mEventLoopTIDs;

        sw::redis::Redis mServRegRedis;

        sw::redis::Subscriber mSub;

        sw::redis::Redis mBrokerRedis;

        std::string keyWithAddress;

        GatewayMessageOp mGatewayMessageOp;

        std::vector<std::future<void>> mLongtimeTaskResults;

        uint16_t mPort;
        std::string mIP;
    };
}

#endif