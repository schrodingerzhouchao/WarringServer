#ifndef __GATEWAY_SERVER_CONFIGURE_H__
#define __GATEWAY_SERVER_CONFIGURE_H__

#include "Broker/RSBrokerHelper.h"

namespace gateway_server
{
    struct GatewayServConf
    {
        bool readConf(sw::redis::ConnectionOptions connOpts, std::string_view loginConfKey);

        sw::redis::ConnectionOptions getBrokerConnOpts() const;

        sw::redis::ConnectionPoolOptions getBrokerConnPoolOpts() const;

        sw::redis::ConnectionOptions getRegistryConnOpts() const;

        sw::redis::ConnectionPoolOptions getRegistryConnPoolOpts() const;

        bool intiKey(sw::redis::Redis &servRegRedis);

        // tcp server's event loop thread number
        size_t SERVER_IO_THREAD_NUM = 4;

        // tcp server's safe hash bucket size
        size_t SERVER_SAFE_HASH_BUCKET_SIZE = 1031;

        std::initializer_list<std::string> CONNECTION_BLACKLIST = {};

        // interval of detection of heartbeat
        size_t HEARTBEAT_TIMEOUT_IN_SECONDS = 60;

        // should satisfy the requirements: EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS << EVENTLOOP_TIMEOUT_IN_SECONDS << EVENTLOOP_RESET_THRESHOLD_IN_SECONDS
        // if EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS is too small and EVENTLOOP_TIMEOUT_IN_SECONDS is too big may call some custom function(if it is existed) frequently

        // detection interval in seconds
        size_t EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS = 30;

        // expiration time in seconds
        size_t EVENTLOOP_TIMEOUT_IN_SECONDS = 300;

        // reset poll timer count periodically if the poll timer is not used
        size_t EVENTLOOP_RESET_THRESHOLD_IN_SECONDS = 3600;

        // message type
        std::string MESSAGE_GATEWAY_BASE_MSG = "GATEWAY_BASE_MSG";
        std::string MESSAGE_GATEWAY_HEARTBEAT_MSG = "HEARTBEAT_MSG";
        std::string MESSAGE_GATEWAY_ENTER_GANE_MSG = "GATEWAY_ENTER_GAME_MSG";

        // broker redis connection options
        std::string BROKER_CONN_HOST = "127.0.0.1";
        int BROKER_CONN_PORT = 6379;
        std::string BROKER_CONN_PASSWORD = {};
        int BROKER_CONN_DB = 0;

        // broker redis connection pool options
        int BROKER_CONN_POOL_SIZE = SERVER_IO_THREAD_NUM + 1;

        // service registry redis connection options
        std::string REGISTRY_CONN_HOST = "127.0.0.1";
        int REGISTRY_CONN_PORT = 6379;
        std::string REGISTRY_CONN_PASSWORD = {};
        int REGISTRY_CONN_DB = 0;

        // service registry redis connection pool options
        int REGISTRY_CONN_POOL_SIZE = 1;

        // service registry id suffix key
        std::string SERVICE_REGISTRY_ID_KEY = "Warring:ServReg:IDSuffix";

        // service keys
        std::string SERVICE_KEYS = "Warring:Service:KEYS";
        // gateway service keys
        std::string GATEWAY_KEYS = "Warring:Service:GATEWAYS";

        // stream key and group of gateway serv
        std::string BROKER_GATEWAY_KEY = "Warring:GatewayServ_";
        std::string BROKER_GATEWAY_GROUP = "GatewayServ:Group";

        size_t BROKER_GATEWAY_KEY_HEARTBEAT_IN_SECOND = 5;
        size_t BROKER_GATEWAY_KEY_HEARTBEAT_TOLERANCE = 3;

        std::string BROKER_GATEWAY_KEY_HEARTBEAT_MESSAGE = "SEND_HEARTBEAT";

        // service register channel
        std::string SERVICE_REGISTRY_CHANNEL = "Warring:ServReg:Channel";

        // stream customer pull data block time
        size_t STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS = 100;
    };
}

#endif
