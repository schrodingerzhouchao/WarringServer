#ifndef __REDIS_STREAM_BROKER_HELPER_H__
#define __REDIS_STREAM_BROKER_HELPER_H__

#include "Base/utility/LoggerMessage.h"
#include "Base/utility/Logger.h"

#include <sw/redis++/redis++.h>

#include <iostream>

namespace broker
{
    using ItemType = std::pair<std::string, sw::redis::Optional<std::vector<std::pair<std::string, std::string>>>>;
    using ItemStreamType = std::vector<ItemType>;

    /*
    <id, consumerName, idleTimeSinceLastDelivered, DeliveredTimes>
    */
    using PendingItemType = std::tuple<std::string, std::string, long long, long long>;

    /*
    <"name"                 groupName
     "consumers"            integer
     "pending"              integer
     "last-delivered-id"    id
     "entries-read"         integer
     "lag"                  integer>
    */
    using GroupInfoType = std::tuple<std::string, std::string,
                                     std::string, long long,
                                     std::string, long long,
                                     std::string, std::string,
                                     std::string, long long,
                                     std::string, long long>;
    /*
    <"name"                 consumerName
     "pending"              integer
     "idle"                 integer
     "inactive"             integer>
    */
    using ConsumerInfoType = std::tuple<std::string, std::string,
                                        std::string, long long,
                                        std::string, long long,
                                        std::string, long long>;

    class RSBrokerHelper
    {
    public:
        template <typename Message>
        static std::string push(sw::redis::Redis &redis, const std::string &key, const Message &msg, size_t maxLen = 5000)
        {
            std::vector<std::pair<std::string, std::string>> kvs;

            msg.FILL_2_VEC(kvs);

            try
            {
                return redis.xadd(key, "*", kvs.begin(), kvs.end(), maxLen, true);
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return {};
            }
        }

        static std::optional<size_t> pull(sw::redis::Redis &redis, const std::string &key,
                                          const std::string &group, const std::string &consumer,
                                          std::unordered_map<std::string, ItemStreamType> &result,
                                          const std::string &id = ">",
                                          size_t ms = 0, size_t count = 1)
        {
            result.clear();

            try
            {
                redis.xreadgroup(group, consumer, key, id, std::chrono::milliseconds(ms), count, std::inserter(result, result.end()));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(result.size());
        }

        static std::optional<size_t> pull(sw::redis::Redis &redis, const std::string &key,
                                          std::unordered_map<std::string, ItemStreamType> &result,
                                          const std::string &id = "$",
                                          size_t ms = 0, size_t count = 1)
        {
            result.clear();

            try
            {
                redis.xread(key, id, std::chrono::milliseconds(ms), count, std::inserter(result, result.end()));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(result.size());
        }

        static std::optional<size_t> pull(sw::redis::Redis &redis,
                                          const std::unordered_map<std::string, std::string> &keys,
                                          std::unordered_map<std::string, ItemStreamType> &result,
                                          size_t ms = 0, size_t count = 1)
        {
            result.clear();

            try
            {
                redis.xread(keys.begin(), keys.end(), std::chrono::milliseconds(ms), count, std::inserter(result, result.end()));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                // LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(result.size());
        }

        static bool createGroup(sw::redis::Redis &redis, const std::string &key, const std::string &group, const std::string &id = "$")
        {
            try
            {
                redis.xgroup_create(key, group, id, true);
            }
            catch (const sw::redis::ReplyError &e)
            {
                // std::cout << e.what() << std::endl;
                LOG_WARN << e.what();
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return false;
            }

            return true;
        }

        static long long ack(sw::redis::Redis &redis, const std::string &key, const std::string &group, const std::string &id)
        {
            return redis.xack(key, group, id);
        }

        static std::optional<size_t> getPending(sw::redis::Redis &redis, const std::string &key, const std::string &group,
                                                size_t count, std::vector<PendingItemType> &pendingItems)
        {
            pendingItems.clear();
            try
            {
                redis.xpending(key, group, "-", "+", count, std::back_inserter(pendingItems));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }

            return std::make_optional<size_t>(pendingItems.size());
        }

        static std::optional<size_t> getPending(sw::redis::Redis &redis, const std::string &key,
                                                const std::string &group, const std::string &consumer,
                                                size_t count, std::vector<PendingItemType> &pendingItems)
        {
            pendingItems.clear();
            try
            {
                redis.xpending(key, group, "-", "+", count, consumer, std::back_inserter(pendingItems));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }

            return std::make_optional<size_t>(pendingItems.size());
        }

        static std::optional<size_t> getGroupsInfo(sw::redis::Redis &redis, const std::string &key, std::vector<GroupInfoType> &groupsInfo)
        {
            groupsInfo.clear();
            try
            {
                redis.command("XINFO", "GROUPS", key, std::back_inserter(groupsInfo));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(groupsInfo.size());
        }

        static std::optional<size_t> getConsumersInfo(sw::redis::Redis &redis, const std::string &key,
                                                      const std::string &group, std::vector<ConsumerInfoType> &consumersInfo)
        {
            consumersInfo.clear();
            try
            {
                redis.command("XINFO", "CONSUMERS", key, group, std::back_inserter(consumersInfo));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(consumersInfo.size());
        }

        static size_t del(sw::redis::Redis &redis, const std::string &key, const std::string &id)
        {
            return redis.xdel(key, id);
        }

        static size_t del(sw::redis::Redis &redis, const std::string &key, const std::vector<std::string> &ids)
        {
            return redis.xdel(key, ids.begin(), ids.end());
        }

        static std::optional<size_t> claim(sw::redis::Redis &redis, const std::string &key,
                                           const std::string &group, const std::string &consumer,
                                           size_t milliseconds, const std::string &id, ItemStreamType &result)
        {
            result.clear();
            try
            {
                redis.xclaim(key, group, consumer, std::chrono::milliseconds(milliseconds), id, std::back_inserter(result));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(result.size());
        }

        static std::optional<size_t> claim(sw::redis::Redis &redis, const std::string &key,
                                           const std::string &group, const std::string &consumer,
                                           size_t milliseconds, const std::vector<std::string> &ids, ItemStreamType &result)
        {
            result.clear();
            try
            {
                redis.xclaim(key, group, consumer, std::chrono::milliseconds(milliseconds), ids.begin(), ids.end(), std::back_inserter(result));
            }
            catch (const sw::redis::Error &e)
            {
                // std::cerr << e.what() << std::endl;
                LOG_WARN << e.what();
                return std::nullopt;
            }
            return std::make_optional<size_t>(result.size());
        }

        static std::string getPreviousID(sw::redis::Redis &redis, const std::string &key, const std::string &id)
        {
            ItemStreamType result;
            redis.xrevrange(key, id, "-", 2, std::back_inserter(result));
            if (result.size() < 2)
            {
                return {};
            }

            return std::min(result[0].first, result[1].first);
        }

        static long sortedAdd(sw::redis::Redis &redis, const std::string &key, const std::string &member, double score)
        {
            try
            {
                return redis.zadd(key, member, score);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }
        static long sortedRemove(sw::redis::Redis &redis, const std::string &key, const std::string &member)
        {
            try
            {
                return redis.zrem(key, member);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR << e.what();
                return -1;
            }
        }

    private:
    };
}

#endif