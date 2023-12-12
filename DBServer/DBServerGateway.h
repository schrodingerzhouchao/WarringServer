#ifndef __DB_SERVER_GATEWAY_H__
#define __DB_SERVER_GATEWAY_H__

#include "Broker/RSBrokerHelper.h"
#include "Base/utility/LoggerMessage.h"

#include "DBServConf.h"
#include "db/Database.h"
#include "cache/RCache.h"

#include "QueryTask.h"

#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>

#include <future>

namespace db_server
{
    template <typename MsgType>
    class DBServGateman
    {
    public:
        DBServGateman(std::string_view readKey, std::string_view group, std::string_view consumer)
            : mReadKey(readKey),
              mGroup(group),
              mConsumer(consumer)
        {
        }

        void setThreadNum(size_t num) { mThreadNum = num; }
        size_t getThreadNum() const { return mThreadNum; }

        void reader(sw::redis::Redis &redis, size_t napInMilliseconds)
        {
            while (true)
            {
                std::unordered_map<std::string, broker::ItemStreamType> result;
                auto pullResult = broker::RSBrokerHelper::pull(redis, mReadKey, mGroup, mConsumer, result, ">", napInMilliseconds);
                if (pullResult.has_value() && pullResult.value() == 1)
                {
                    MsgType msg;

                    msg.FILL_2_THIS(result[mReadKey].at(0).second.value());

                    LOG_INFO << "DBServGateman read from " << mReadKey << " TYPE: " << msg.TYPE << " ID:" << msg.CLIENT_ID << " SERVICE: " << msg.SERVICE << " NEXT_SERVICE: " << msg.NEXT_SERVICE;

                    if (mHandleFuncs.count(msg.TYPE) == 1 && mHandleFuncs[msg.TYPE])
                    {
                        LOG_INFO << LOG_COUT << "Match message type " << msg.TYPE;
                        mHandleFuncs[msg.TYPE](msg);
                    }
                    else
                    {
                        LOG_ERROR << "No such message type " << msg.TYPE;
                    }

                    if (msg.TYPE == "LOGIN_ERROR_MSG")
                    {
                        LOG_ERROR << "error message type " << msg.TYPE;
                        broker::RSBrokerHelper::del(redis, mReadKey, result[mReadKey].at(0).first);
                    }

                    auto acked = broker::RSBrokerHelper::ack(redis, mReadKey, mGroup, result[mReadKey].at(0).first);
                    if (acked == 1)
                    {
                        if (!msg.NEXT_SERVICE.empty())
                        {
                            broker::RSBrokerHelper::push(redis, msg.NEXT_SERVICE, msg);
                        }
                    }
                }
            }
        }

        void registerFunc(const std::string &type, std::function<void(MsgType &)> func) { mHandleFuncs.emplace(type, std::move(func)); }

        std::string_view getReadKey() const { return mReadKey; }

        std::string_view getGroup() const { return mGroup; }
        std::string_view getConsumer() const { return mConsumer; }

    private:
        std::string mReadKey;
        std::string mGroup;
        std::string mConsumer;

        size_t mThreadNum = 1;

        std::unordered_map<std::string, std::function<void(MsgType &)>> mHandleFuncs;
    };

    class DBServGateway
    {
    public:
        DBServGateway(DBServConf &conf)
            : mConf(conf),
              mBrokerRedis(mConf.getBrokerConnOpts(), mConf.getBrokerConnPoolOpts()),
              mServRegRedis(mConf.getServRegConnOpts(), mConf.getServRegConnPoolOpts()),
              mCache(mConf),
              mDatabase(mConf.MONGODB_URI),
              mLoginGateman(mConf.BROKER_DB_4_LOGIN_KEY, mConf.BROKER_DB_GROUP, mConsumerPrefix + std::to_string(++mNo)),
              mGameGateman(mConf.BROKER_DB_4_GAME_KEY, mConf.BROKER_DB_GROUP, mConsumerPrefix + std::to_string(++mNo)),
              mQueryTask(mConf, mDatabase, mCache, mServRegRedis)

        {
            mLoginGateman.setThreadNum(mConf.LOGIN_SERVICE_READER_THREAD_NUM);

            mLoginGateman.registerFunc(mConf.MESSAGE_LOGIN_LOGIN_MSG,
                                       std::bind(&DBServGateway::loginHandleFunc, this, std::placeholders::_1));
            mLoginGateman.registerFunc(mConf.MESSAGE_LOGIN_LOGIN_CHARACTER_MSG,
                                       std::bind(&DBServGateway::loginCharacterHandleFunc, this, std::placeholders::_1));

            mGameGateman.setThreadNum(mConf.GAME_SERVICE_READER_THREAD_NUM);

            mGameGateman.registerFunc(mConf.MESSAGE_GAME_MSG,
                                      std::bind(&DBServGateway::gameHandleFunc, this, std::placeholders::_1));
            mGameGateman.registerFunc(mConf.MESSAGE_GAME_CHARACTER_MSG,
                                      std::bind(&DBServGateway::gameCharacterHandleFunc, this, std::placeholders::_1));
        }

        ~DBServGateway()
        {
            for (auto &thdRes : mBackgroundThdsResult)
            {
                thdRes.get();
            }
        }

        void start()
        {
            mDatabase.initDBCollName("warring", "accounts", "characters");

            mDatabase.initDBCollName(mConf.MONGODB_DB_NAME, mConf.MONGODB_COLLECTION_ACCOUNT_NAME, mConf.MONGODB_COLLECTION_CHARACTER_NAME);

            mDatabase.check();

            mDatabase.createIndex();

            broker::RSBrokerHelper::createGroup(mBrokerRedis, mConf.BROKER_DB_4_LOGIN_KEY, mConf.BROKER_DB_GROUP);
            broker::RSBrokerHelper::createGroup(mBrokerRedis, mConf.BROKER_DB_4_GATEWAY_KEY, mConf.BROKER_DB_GROUP);
            broker::RSBrokerHelper::createGroup(mBrokerRedis, mConf.BROKER_DB_4_GAME_KEY, mConf.BROKER_DB_GROUP);

            for (size_t i = 0; i < mLoginGateman.getThreadNum(); ++i)
            {
                auto res = std::async(std::launch::async, std::bind(&DBServGateman<message::message_carrier::LoginMessage>::reader,
                                                                    mLoginGateman, std::ref(mBrokerRedis), mConf.STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS));

                mBackgroundThdsResult.push_back(std::move(res));
            }
#if 1
            for (size_t i = 0; i < mGameGateman.getThreadNum(); ++i)
            {
                auto res = std::async(std::launch::async, std::bind(&DBServGateman<message::message_carrier::GameMessage>::reader,
                                                                    mGameGateman, std::ref(mBrokerRedis), mConf.STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS));

                mBackgroundThdsResult.push_back(std::move(res));
            }
#endif
        }

    private:
        void loginHandleFunc(message::message_carrier::LoginMessage &loginMsgCarrier)
        {
            mQueryTask.getLoginResponseMsg(loginMsgCarrier);
        }

        void loginCharacterHandleFunc(message::message_carrier::LoginMessage &loginMsgCarrier)
        {
            mQueryTask.getLoginCharacterResponseMsg(loginMsgCarrier);
        }

        void gameHandleFunc(message::message_carrier::GameMessage &gameMsgCarrier)
        {
            mQueryTask.getGameResponseMsg(gameMsgCarrier);
        }
        void gameCharacterHandleFunc(message::message_carrier::GameMessage &gameMsgCarrier)
        {
            mQueryTask.getGameCharacterResponseMsg(gameMsgCarrier);
        }

        // TODO void gameGatewayHandleFunc();
        // TODO void otherTypeHandleFunc()

    private:
        DBServConf &mConf;

        sw::redis::Redis mBrokerRedis;

        sw::redis::Redis mServRegRedis;

        cache::RCache mCache;

        db::Database mDatabase;

        const std::string mConsumerPrefix = "CONSUMER_";

        size_t mNo = 0;

        DBServGateman<message::message_carrier::LoginMessage> mLoginGateman;

        DBServGateman<message::message_carrier::GameMessage> mGameGateman;

        db_server::task::QueryTask mQueryTask;

        std::vector<std::future<void>> mBackgroundThdsResult;
    };
}

#endif
