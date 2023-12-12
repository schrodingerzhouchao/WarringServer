#include "CenterServConf.h"

namespace center_server
{
    bool CenterServConf::readConf(sw::redis::ConnectionOptions connOpts, std::string_view CSConfKey)
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

    sw::redis::ConnectionOptions CenterServConf::getServRegConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = REGISTRY_CONN_HOST;
        connOpts.port = REGISTRY_CONN_PORT;
        connOpts.password = REGISTRY_CONN_PASSWORD;
        connOpts.db = REGISTRY_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions CenterServConf::getServRegConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = REGISTRY_CONN_POOL_SIZE;

        return connPoolOpts;
    }
#if 0
    sw::redis::ConnectionOptions CenterServConf::getConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = CACHE_CONN_HOST;
        connOpts.port = CACHE_CONN_PORT;
        connOpts.password = CACHE_CONN_PASSWORD;
        connOpts.db = CACHE_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions CenterServConf::getConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = CACHE_CONN_POOL_SIZE;

        return connPoolOpts;
    }
#endif
}