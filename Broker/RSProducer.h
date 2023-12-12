// deprecated part for now

#ifndef __REDIS_STREAM_PRODUCER_H__
#define __REDIS_STREAM_PRODUCER_H__

#include "RSBrokerHelper.h"

namespace broker
{
    class RSProducer
    {
    public:
        RSProducer(const sw::redis::ConnectionOptions &connOpts, const sw::redis::ConnectionPoolOptions &connPoolOpts)
            : mRedis(connOpts, connPoolOpts)
        {
        }

        template <typename Msg>
        std::string push(const std::string &key, const Msg &msg, size_t maxLen)
        {
            return RSBrokerHelper::push(mRedis, key, msg, maxLen);
        }

    private:
        sw::redis::Redis mRedis;
    };
}

#endif