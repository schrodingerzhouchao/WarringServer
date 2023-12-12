#include "DBServConf.h"

namespace db_server
{
    bool DBServConf::readConf(sw::redis::ConnectionOptions connOpts, std::string_view dbConfKey)
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

    sw::redis::ConnectionOptions DBServConf::getBrokerConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = BROKER_CONN_HOST;
        connOpts.port = BROKER_CONN_PORT;
        connOpts.password = BROKER_CONN_PASSWORD;
        connOpts.db = BROKER_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions DBServConf::getBrokerConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = BROKER_CONN_POOL_SIZE;

        return connPoolOpts;
    }

    sw::redis::ConnectionOptions DBServConf::getCacheConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = CACHE_CONN_HOST;
        connOpts.port = CACHE_CONN_PORT;
        connOpts.password = CACHE_CONN_PASSWORD;
        connOpts.db = CACHE_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions DBServConf::getCacheConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = CACHE_CONN_POOL_SIZE;

        return connPoolOpts;
    }

    sw::redis::ConnectionOptions DBServConf::getServRegConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = SERVICE_REGISTRY_CONN_HOST;
        connOpts.port = SERVICE_REGISTRY_CONN_PORT;
        connOpts.password = SERVICE_REGISTRY_CONN_PASSWORD;
        connOpts.db = SERVICE_REGISTRY_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions DBServConf::getServRegConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = SERVICE_REGISTRY_CONN_POOL_SIZE;

        return connPoolOpts;
    }
}