#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include "GameServConf.h"
#include "GameMessageOp.h"
#include "GameHeartbeatOp.h"
#include "SceneManager.h"

#include "Base/net/TcpServer.h"
#include "Base/message/CustomMessages.h"
#include "Base/utility/LoggerMessage.h"
#include "Base/utility/Cmd.h"
#include "Broker/RSBrokerHelper.h"

namespace game_server
{
    class GameServer
    {
    public:
        GameServer(GameServConf &gameServConf, net::socket::SocketAddr sa, std::string servName = "GameSever")
            : mConf(gameServConf),
              mTcpServer(sa, mConf.SERVER_IO_THREAD_NUM, servName, mConf.SERVER_SAFE_HASH_BUCKET_SIZE),
              mServRegRedis(mConf.getRegistryConnOpts(), mConf.getRegistryConnPoolOpts()),
              mSub(mServRegRedis.subscriber()),
              mBrokerRedis(mConf.getBrokerConnOpts(), mConf.getBrokerConnPoolOpts()),
              mSceneManager(mServRegRedis),
              mGameMessageOp(mBrokerRedis, mServRegRedis, mConf, mTcpServer, mSceneManager)
        {
            mEventLoopTIDs = mTcpServer.getEventLoopThreadIDs();
            for (auto tid : mEventLoopTIDs)
            {
                // register internet message type
                mGameMessageOp.registerMessageType<message::TimestampMessage>(tid, mConf.MESSAGE_GAME_BASE_MSG);
                mGameMessageOp.registerMessageType<message::HeartBeatPacket>(tid, mConf.MESSAGE_GAME_HEARTBEAT_MSG);
                mGameMessageOp.registerMessageType<message::DSMessage>(tid, mConf.MESSAGE_GAME_MSG);
                mGameMessageOp.registerMessageType<message::DSLoadOrUploadCharacterMessage>(tid, mConf.MESSAGE_GAME_CHARACTER_MSG);
            }

            mPort = sa.getPort();
            utility::Cmd cmd;
            mIP = cmd("ifconfig ens33 | awk 'NR==2{print $2}'");
        }

        GameServer(const GameServer &) = delete;
        GameServer &operator=(const GameServer &) = delete;
        GameServer(GameServer &&) = delete;
        GameServer &operator=(GameServer &&) = delete;

        ~GameServer()
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

            mTcpServer.setConnFilterFunc(std::bind(&GameServer::connFilterFunc, this, std::placeholders::_1));
            mTcpServer.setConnNameFunc(std::bind(&GameServer::connNameFunc, this, std::placeholders::_1));
            mTcpServer.setMessageFunc(std::bind(&GameServer::messageFunc, this, std::placeholders::_1));
            mTcpServer.setCloseFunc(std::bind(&GameServer::closeFunc,this,std::placeholders::_1));
            setLongtimeTask(std::bind(&GameServer::gameStreamConsumerFunc, this));
            setLongtimeTask(std::bind(&GameServer::PSConsumerFunc, this));

            mTcpServer.start();
        }

    private:
        void initConnBlacklist() { mTcpServer.addToBlacklist(mConf.CONNECTION_BLACKLIST); }

        bool connFilterFunc(const net::socket::SocketAddr &addr);

        void connNameFunc(std::string connName);

        void messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr);

        void closeFunc(std::string address);

        // run a thread with longtime task asynchronously
        void setLongtimeTask(std::function<void()> func);

        // read message from db serv through broker
        void gameStreamConsumerFunc();

        // redis pub/sub func
        void onMessageFunc(std::string channnel, std::string msg);
        void onPMessageFunc(std::string pattern, std::string channel, std::string msg);
        void onMetaFunc(sw::redis::Subscriber::MsgType type, sw::redis::OptionalString channel, long long num);

        // redis pub/sub consume messages loop
        void PSConsumerFunc();

    private:
        GameServConf &mConf;

        net::TcpServer mTcpServer;

        std::vector<std::thread::id> mEventLoopTIDs;

        sw::redis::Redis mServRegRedis;

        sw::redis::Subscriber mSub;

        sw::redis::Redis mBrokerRedis;

        std::string keyWithAddress;

        game_scene::SceneManager mSceneManager;

        GameMessageOp mGameMessageOp;

        std::vector<std::future<void>> mLongtimeTaskResults;

        uint16_t mPort;
        std::string mIP;
    };
}

#endif