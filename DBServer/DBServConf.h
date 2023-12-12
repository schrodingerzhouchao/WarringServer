#ifndef __DB_SERVER_CONFIGURE_H__
#define __DB_SERVER_CONFIGURE_H__

#include "Broker/RSBrokerHelper.h"

namespace db_server
{
    struct DBServConf
    {
        bool readConf(sw::redis::ConnectionOptions connOpts, std::string_view dbConfKey);

        sw::redis::ConnectionOptions getBrokerConnOpts() const;

        sw::redis::ConnectionPoolOptions getBrokerConnPoolOpts() const;

        sw::redis::ConnectionOptions getCacheConnOpts() const;

        sw::redis::ConnectionPoolOptions getCacheConnPoolOpts() const;

        sw::redis::ConnectionOptions getServRegConnOpts() const;

        sw::redis::ConnectionPoolOptions getServRegConnPoolOpts() const;

        // message type
        // login message type
        std::string MESSAGE_LOGIN_LOGIN_MSG = "LOGIN_LOGIN_MSG";
        std::string MESSAGE_LOGIN_LOGIN_CHARACTER_MSG = "LOGIN_LOGIN_CHARACTER_MSG";
        std::string MESSAGE_LOGIN_LOGIN_RESPONSE_MSG = "LOGIN_LOGIN_RESPONSE_MSG";
        std::string MESSAGE_LOGIN_ERROR_MSG = "LOGIN_ERROR_MSG";

        // gateway message type

        // game message type
        std::string MESSAGE_GAME_MSG = "GAME_MSG";
        std::string MESSAGE_GAME_CHARACTER_MSG = "GAME_CHARACTER_MSG";
        std::string MESSAGE_GAME_RESPONSE_MSG = "GAME_RESPONSE_MSG";
        std::string MESSAGE_GAME_ERROR_MSG = "GAME_ERROR_MSG";

        // TODO xx message type

        // broker redis connection options
        std::string BROKER_CONN_HOST = "127.0.0.1";
        int BROKER_CONN_PORT = 6379;
        std::string BROKER_CONN_PASSWORD = {};
        int BROKER_CONN_DB = 0;

        // broker redis connection pool options
        int BROKER_CONN_POOL_SIZE = 16;

        // cache redis connection options
        std::string CACHE_CONN_HOST = "127.0.0.1";
        int CACHE_CONN_PORT = 6379;
        std::string CACHE_CONN_PASSWORD = {};
        int CACHE_CONN_DB = 0;

        // cache redis connection pool options
        int CACHE_CONN_POOL_SIZE = 16;

        // cache redis connection options
        std::string SERVICE_REGISTRY_CONN_HOST = "127.0.0.1";
        int SERVICE_REGISTRY_CONN_PORT = 6379;
        std::string SERVICE_REGISTRY_CONN_PASSWORD = {};
        int SERVICE_REGISTRY_CONN_DB = 0;

        // cache redis connection pool options
        int SERVICE_REGISTRY_CONN_POOL_SIZE = 1;

        // mongodb pool's uri
        std::string MONGODB_URI = "mongodb://192.168.119.128:27017/?minPoolSize=8&maxPoolSize=8";

        // mongodb db name
        std::string MONGODB_DB_NAME = "warring";

        // mongodb db collection name
        std::string MONGODB_COLLECTION_ACCOUNT_NAME = "accounts";
        std::string MONGODB_COLLECTION_CHARACTER_NAME = "characters";
        std::string MONGODB_COLLECTION_XXX_NAME = "XXX";
        std::string MONGODB_COLLECTION_XXXX_NAME = "XXXX";

        // stream key and group of db serv
        //std::string BROKER_DB_KEY = "Warring:DBServ";
        std::string BROKER_DB_4_LOGIN_KEY = "Warring:DBServ4LoginServ";
        std::string BROKER_DB_4_GATEWAY_KEY = "Warring:DBServ4GatewayServ";
        std::string BROKER_DB_4_GAME_KEY = "Warring:DBServ4GameServ";
        std::string BROKER_DB_GROUP = "Warring:DBServ:Group";

        // service's lose threshold
        size_t SERVICE_LOSE_THRESHOLD_IN_SECONDS = 15;

        // gateway service keys
        std::string GATEWAY_KEYS = "Warring:Service:GATEWAYS";

        size_t LOGIN_SERVICE_READER_THREAD_NUM = 4;

        size_t GAME_SERVICE_READER_THREAD_NUM = 4;

        // ds scene name prefix
        std::string SERVICE_DS_PREFIX = "Warring:Scene:";

        // stream customer pull data block time
        size_t STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS = 100;
    };
}

#endif