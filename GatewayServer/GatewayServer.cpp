#include "GatewayServer.h"
#include "GatewayMessageOp.h"

namespace gateway_server
{

    bool GatewayServer::linkTimer(const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr)
    {
        return mTcpServer.linkTimer(timerPtr);
    }

    bool GatewayServer::registerTimerTask(size_t hours, size_t minutes, size_t seconds, int cyc, std::function<void()> func)
    {
        return mTcpServer.registerTimerTask(hours, minutes, seconds, cyc, func);
    }

    void GatewayServer::setLongtimeTask(std::function<void()> func)
    {
        auto res = std::async(std::launch::async, func);
        mLongtimeTaskResults.push_back(std::move(res));
    }

    bool GatewayServer::connFilterFunc(const net::socket::SocketAddr &addr)
    {
        auto ip = addr.getIP();
        if (mTcpServer.isInBlacklist(ip))
        {
            LOG_INFO << addr.getHostAndServiceNUM() << " is in blacklist";
            return false;
        }
        return true;
    }

    void GatewayServer::connNameFunc(std::string connName)
    {
        GatewayHeartbeatOp gatewayHeartbeatOP(mConf, mTcpServer, connName);

        mTcpServer.registerTimerTask(0, 0, mConf.HEARTBEAT_TIMEOUT_IN_SECONDS, -1, std::bind(&GatewayHeartbeatOp::checkHeartbeat, gatewayHeartbeatOP));
    }

    void GatewayServer::messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr)
    {
        mGatewayMessageOp.messageFunc(tcpConnptr);
    }

    void GatewayServer::gatewayStreamConsumerFunc()
    {
        // while (true)
        {
            std::unordered_map<std::string, broker::ItemStreamType> result;
            auto pullResult = broker::RSBrokerHelper::pull(mBrokerRedis, mConf.BROKER_GATEWAY_KEY, mConf.BROKER_GATEWAY_GROUP, "GatewayStreamConsumer", result, ">", mConf.STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS);
            if (pullResult.has_value() && pullResult.value() == 1)
            {
                message::message_carrier::GatewayMessage gatewayMsgCarrier;
                gatewayMsgCarrier.FILL_2_THIS(result[mConf.BROKER_GATEWAY_KEY].at(0).second.value());
#if 0
                LOG_DEBUG << "db_consumer read from " << mConf.BROKER_LOGIN_4_DB_KEY << " TYPE " << loginResMsgCarrier.TYPE
                          << " ID " << loginResMsgCarrier.CLIENT_ID
                          << " MESSAGE_LEN " << loginResMsgCarrier.MESSAGE.length();
#endif
                std::shared_ptr<net::TcpConnection> connPtr;

                if (mTcpServer.getConnPtrRef(gatewayMsgCarrier.CLIENT_ID, connPtr) && connPtr)
                {
                    connPtr->putToBuffer(gatewayMsgCarrier.MESSAGE);
                }
                else
                {
                    LOG_WARN << gatewayMsgCarrier.CLIENT_ID << " has been lost";
                }
            }
        }
    }

    bool GatewayServer::initKey()
    {
        auto result = mConf.intiKey(mServRegRedis);
        if (result)
            broker::RSBrokerHelper::createGroup(mServRegRedis, mConf.BROKER_GATEWAY_KEY, mConf.BROKER_GATEWAY_GROUP);

        return result;
    }

    void GatewayServer::registerService()
    {
        keyWithAddress = mConf.BROKER_GATEWAY_KEY + ":" + mIP + ":" + std::to_string(mPort);
        mServRegRedis.sadd(mConf.SERVICE_KEYS, keyWithAddress);
        std::unordered_map<std::string, std::string> address = {{"address", mIP}, {"port", std::to_string(mPort)}};
        mServRegRedis.hset(keyWithAddress, address.cbegin(), address.cend());

        mServRegRedis.expire(keyWithAddress,
                             std::chrono::seconds(mConf.BROKER_GATEWAY_KEY_HEARTBEAT_IN_SECOND * mConf.BROKER_GATEWAY_KEY_HEARTBEAT_TOLERANCE));

        mSub.on_message(std::bind(&GatewayServer::onMessageFunc, this, std::placeholders::_1, std::placeholders::_2));

        mSub.on_pmessage(std::bind(&GatewayServer::onPMessageFunc, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        mSub.on_meta(std::bind(&GatewayServer::onMetaFunc, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        mSub.subscribe(mConf.SERVICE_REGISTRY_CHANNEL);
    }

    void GatewayServer::onMessageFunc(std::string channnel, std::string msg)
    {
        //LOG_INFO << "from channel: " << channnel << " get message: " << msg;
        if (msg == mConf.BROKER_GATEWAY_KEY_HEARTBEAT_MESSAGE)
        {
            auto activeNum = mTcpServer.getActiveConnectionNum();

            //LOG_INFO << "active client num: " << activeNum;

            mServRegRedis.expire(keyWithAddress,
                                 std::chrono::seconds(mConf.BROKER_GATEWAY_KEY_HEARTBEAT_IN_SECOND * mConf.BROKER_GATEWAY_KEY_HEARTBEAT_TOLERANCE));

            mServRegRedis.zadd(mConf.GATEWAY_KEYS, keyWithAddress, activeNum);
        }
        else if(msg.front() == '+')
        {
            // new service is running
        }
        else if(msg.front() == '-')
        {
            // someone service is lost
        }
    }
    void GatewayServer::onPMessageFunc(std::string pattern, std::string channel, std::string msg)
    {
    }
    void GatewayServer::onMetaFunc(sw::redis::Subscriber::MsgType type, sw::redis::OptionalString channel, long long num)
    {
    }
    void GatewayServer::PSConsumerFunc()
    {

        while (true)
        {
            try
            {
                mSub.consume();
            }
            catch (const sw::redis::TimeoutError &e)
            {
                // Try again.
                continue;
            }
            catch (const sw::redis::Error &err)
            {
                // Handle other exceptions.
            }
        }
    }
}