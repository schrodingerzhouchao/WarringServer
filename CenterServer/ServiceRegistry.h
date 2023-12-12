#ifndef __REDIS_SERVICE_REGISTRY_H__
#define __REDIS_SERVICE_REGISTRY_H__

#include "CenterServConf.h"

#include "Broker/RSBrokerHelper.h"
#include <Base/utility/LoggerMessage.h>
// #include <iostream>
#include <thread>

namespace center_server
{
    namespace registry
    {
        class ServiceRegistry
        {
        public:
            ServiceRegistry(const CenterServConf &conf)
                : mConf(conf),
                  mRedis(mConf.getServRegConnOpts(), mConf.getServRegConnPoolOpts())
            {
            }

            void initKeys()
            {
                mRedis.set(mConf.SERVICE_REGISTRY_ID_KEY, mIDSuffix);
            }

            void run()
            {
                std::unordered_set<std::string> serviceKeys;

                while (true)
                {
                    serviceKeys.clear();

                    mRedis.smembers(mConf.SERVICE_KEYS, std::inserter(serviceKeys, serviceKeys.begin()));
                    for (const auto &serviceKey : serviceKeys)
                    {
                        auto existed = mRedis.exists(serviceKey);
                        if (existed == 1)
                        {
                            if (mServiceKeys.insert_or_assign(serviceKey, 0).second)
                            {
                                mRedis.publish(mConf.SERVICE_REGISTRY_CHANNEL, "+" + serviceKey);
                                LOG_INFO << "get new service " << serviceKey;
                            }
                        }
                        else
                        {
                            mServiceKeys.insert_or_assign(serviceKey, mServiceKeys[serviceKey] + 1);
                            if (mServiceKeys[serviceKey] >= 3)
                            {
                                mRedis.srem(mConf.SERVICE_KEYS, serviceKey);
                                mRedis.zrem(mConf.GATEWAY_KEYS, serviceKey);
                                mRedis.zrem(mConf.GAME_KEYS, serviceKey);
                                mServiceKeys.erase(serviceKey);

                                mRedis.publish(mConf.SERVICE_REGISTRY_CHANNEL, "-" + serviceKey);
                                LOG_INFO << "lost service: " << serviceKey;
                            }
                        }
                    }

                    mRedis.publish(mConf.SERVICE_REGISTRY_CHANNEL, mConf.KEY_HEARTBEAT_MESSAGE);
                    std::string tmp;
                    for (const auto &serviceKey : mServiceKeys)
                    {
                        tmp.append(serviceKey.first + " ");
                    }
                    //LOG_INFO << "active services: " << tmp;
                    // check per 5 seconds
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }

        private:
            time_t getMillisecondsSinceEpoch()
            {
                std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
                return millis.count();
            }

        private:
            const CenterServConf &mConf;

            sw::redis::Redis mRedis;

            std::string mIDSuffix = "0";

            // <service key, heartbeat miss num>
            std::unordered_map<std::string, size_t> mServiceKeys;
        };
    }
}

#endif