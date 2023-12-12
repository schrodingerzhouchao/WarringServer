// deprecated part for now

#ifndef __REDIS_STREAM_CONSUMER_H__
#define __REDIS_STREAM_CONSUMER_H__

#include "RSBrokerHelper.h"
#include "message/CustomMessages.h"
#include <memory>

namespace broker
{
    class RSConsumer
    {
    public:
        RSConsumer(const sw::redis::ConnectionOptions &connOpts)
            : mRedis(connOpts)
        {
        }

    private:
        sw::redis::Redis mRedis;

        
        thread_local static std::unordered_map<std::string, std::unique_ptr<message::intranet::CLoginMessage>> m;
    };

}

#endif