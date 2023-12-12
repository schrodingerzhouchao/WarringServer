#include "LoginServConf.h"

namespace login_server
{
    bool LoginServConf::readConf(sw::redis::ConnectionOptions connOpts, std::string_view loginConfKey)
    {
        try
        {
            sw::redis::Redis redis(connOpts);

            // TODO redis.hgetall
        }
        catch (const sw::redis::Error &err)
        {
            return false;
        }

        return true;
    }

    sw::redis::ConnectionOptions LoginServConf::getBrokerConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = BROKER_CONN_HOST;
        connOpts.port = BROKER_CONN_PORT;
        connOpts.password = BROKER_CONN_PASSWORD;
        connOpts.db = BROKER_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions LoginServConf::getBrokerConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = BROKER_CONN_POOL_SIZE;

        return connPoolOpts;
    }
}