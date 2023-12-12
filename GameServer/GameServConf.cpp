#include "GameServConf.h"

namespace game_server
{
    bool GameServConf::readConf(sw::redis::ConnectionOptions connOpts, std::string_view loginConfKey)
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

    sw::redis::ConnectionOptions GameServConf::getBrokerConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = BROKER_CONN_HOST;
        connOpts.port = BROKER_CONN_PORT;
        connOpts.password = BROKER_CONN_PASSWORD;
        connOpts.db = BROKER_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions GameServConf::getBrokerConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = BROKER_CONN_POOL_SIZE;

        return connPoolOpts;
    }

    sw::redis::ConnectionOptions GameServConf::getRegistryConnOpts() const
    {
        sw::redis::ConnectionOptions connOpts;
        connOpts.host = REGISTRY_CONN_HOST;
        connOpts.port = REGISTRY_CONN_PORT;
        connOpts.password = REGISTRY_CONN_PASSWORD;
        connOpts.db = REGISTRY_CONN_DB;

        return connOpts;
    }

    sw::redis::ConnectionPoolOptions GameServConf::getRegistryConnPoolOpts() const
    {
        sw::redis::ConnectionPoolOptions connPoolOpts;
        connPoolOpts.size = REGISTRY_CONN_POOL_SIZE;

        return connPoolOpts;
    }

    bool GameServConf::intiKey(sw::redis::Redis &servRegRedis)
    {
        try
        {
            auto suffix = servRegRedis.incr(SERVICE_REGISTRY_ID_KEY);
            BROKER_GAME_KEY += std::to_string(suffix);

            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }
}