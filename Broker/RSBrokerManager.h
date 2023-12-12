#ifndef __REDIS_STREAM_BROKER_H__
#define __REDIS_STREAM_BROKER_H__

#include "RSBrokerHelper.h"

//#include "Base/utility/Logger.h"

#include <thread>
#include <set>
#include <queue>

namespace broker
{
    class RSBrokerManager
    {
    public:
        RSBrokerManager(const sw::redis::ConnectionOptions &connOpt)
            : mRedis(connOpt)
        {
        }

        void setNapSecond(size_t milliseconds) { mNapMilliseconds = milliseconds > 10 ? milliseconds : 1000; }

        // if message's idle time greater than tolerant time, xack it
        // the possible cause of this problem is that no customer is used to transfer the message, and the customer forget to xack or just down
        void setTolerantTime(size_t milliseconds) { mTolerantMilliseconds = milliseconds >= 8000 ? milliseconds : 10000; }

        // transfer message to someone customer when the idle time is greater than transfer time
        // transfer time should be less than tolerant time, may be [ TolerantTime > TransferTime * MaxDelivered ] is well
        void setTransferTime(size_t milliseconds) { mTransferMilliseconds = milliseconds > 1000 ? milliseconds : 1000; }

        void setMaxDelivered(size_t maxDelivered) { mMaxDelivered = maxDelivered >= 3 ? maxDelivered : 3; }

        bool registerKey(const std::string &key) { return mKeys.insert(key).second; }

        void run()
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(mNapMilliseconds));

                handlePending();

                // TODO monitor the stream's length instead of just relying on xadd's MAXLEN

                // TODO more...
            }
        }

    private:
        void pushPendingGroups(const std::string &key);

        void pushPendingConsumers(const std::string &key, const std::string &group);

        // get a sibling consumer in the same group
        std::string getOneSibling(const std::string &group, const std::string &consumer);

        // handle the pending message, transfer or just ack or ack then del
        void handlePending();

    private:
        sw::redis::Redis mRedis;

        size_t mNapMilliseconds = 1000;

        size_t mTolerantMilliseconds = 10000;

        size_t mTransferMilliseconds = 5000;

        size_t mMaxDelivered = 3;

        std::set<std::string> mKeys;

        // queue of <key, group with pending>
        std::queue<std::pair<std::string, std::string>> mPendingGroupQueue;

        // queue of <key, group, consumer with pending, size of pending>
        std::queue<std::tuple<std::string, std::string, std::string, size_t>> mPendingConsumerQueue;

        // hash table: <pending group, this group's consumers>
        std::unordered_map<std::string, std::set<std::string>> mPendingGroupsSibling;
    };
}

#endif
