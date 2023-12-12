#ifndef __LOGIN_SERVER_CONFIGURE_H__
#define __LOGIN_SERVER_CONFIGURE_H__

#include "Broker/RSBrokerHelper.h"

namespace login_server
{
    struct LoginServConf
    {
        bool readConf(sw::redis::ConnectionOptions connOpts, std::string_view loginConfKey);

        sw::redis::ConnectionOptions getBrokerConnOpts() const;

        sw::redis::ConnectionPoolOptions getBrokerConnPoolOpts() const;

        // tcp server's event loop thread number
        size_t SERVER_IO_THREAD_NUM = 4;

        // tcp server's safe hash bucket size
        size_t SERVER_SAFE_HASH_BUCKET_SIZE = 1031;

        std::initializer_list<std::string> CONNECTION_BLACKLIST = {};

        // interval of detection of heartbeat
        size_t HEARTBEAT_TIMEOUT_IN_SECONDS = 3600;

        // should satisfy the requirements: EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS << EVENTLOOP_TIMEOUT_IN_SECONDS << EVENTLOOP_RESET_THRESHOLD_IN_SECONDS
        // if EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS is too small and EVENTLOOP_TIMEOUT_IN_SECONDS is too big may call some custom function(if it is existed) frequently

        // detection interval in seconds
        size_t EVENTLOOP_DETECTION_INTERVAL_IN_SECONDS = 30;

        // expiration time in seconds
        size_t EVENTLOOP_TIMEOUT_IN_SECONDS = 300;

        // reset poll timer count periodically if the poll timer is not used
        size_t EVENTLOOP_RESET_THRESHOLD_IN_SECONDS = 3600;

        // message type
        std::string MESSAGE_LOGIN_BASE_MSG = "LOGIN_BASE_MSG";
        std::string MESSAGE_LOGIN_HEARTBEAT_MSG = "HEARTBEAT_MSG";
        std::string MESSAGE_LOGIN_LOGIN_MSG = "LOGIN_LOGIN_MSG";
        std::string MESSAGE_LOGIN_LOGIN_CHARACTER_MSG = "LOGIN_LOGIN_CHARACTER_MSG";
        std::string MESSAGE_LOGIN_LOGIN_RESPONSE_MSG = "LOGIN_LOGIN_RESPONSE_MSG";

        // redis connection options
        std::string BROKER_CONN_HOST = "127.0.0.1";
        int BROKER_CONN_PORT = 6379;
        std::string BROKER_CONN_PASSWORD = {};
        int BROKER_CONN_DB = 0;

        // redis connection pool options
        int BROKER_CONN_POOL_SIZE = SERVER_IO_THREAD_NUM + 1;

        // stream key and group of db serv
        std::string BROKER_DB_KEY = "Warring:DBServ4LoginServ";
        std::string BROKER_DB_GROUP = "Warring:DBServ:Group";
        // stream key and group of login serv
        std::string BROKER_LOGIN_KEY = "Warring:LoginServ";
        std::string BROKER_LOGIN_GROUP = "Warring:LoginServ:Group";
        

        // stream customer pull data block time
        size_t STREAM_CUSTOMER_PULL_NAP_IN_MILLISECONDS = 100;
    };
}

#endif
