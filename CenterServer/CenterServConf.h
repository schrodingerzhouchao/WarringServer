#ifndef __CENTER_SERVER_CONFIGURE_H__
#define __CENTER_SERVER_CONFIGURE_H__

#include <sw/redis++/redis++.h>

namespace center_server
{
        struct CenterServConf
        {
                bool readConf(sw::redis::ConnectionOptions connOpts, std::string_view CSConfKey);

                sw::redis::ConnectionOptions getServRegConnOpts() const;

                sw::redis::ConnectionPoolOptions getServRegConnPoolOpts() const;

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
                // game service keys
                std::string GAME_KEYS = "Warring:Service:GAMES";
                // xxxx service keys
                // std::string XXXX_KEYS = "Warring:Service:XXXX";
                
                std::string KEY_HEARTBEAT_MESSAGE = "SEND_HEARTBEAT";

                // service register channel
                std::string SERVICE_REGISTRY_CHANNEL = "Warring:ServReg:Channel";
        };
}

#endif