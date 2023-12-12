#include "RSBrokerManager.h"
#include "Base/utility/Logger.h"



namespace broker
{
    void RSBrokerManager::pushPendingGroups(const std::string &key)
    {
        std::vector<GroupInfoType> groupsInfo;
        auto groupsInfoResult = RSBrokerHelper::getGroupsInfo(mRedis, key, groupsInfo);
        if (groupsInfoResult.has_value() && (groupsInfoResult.value() > 0))
        {
            for (const auto &groupInfo : groupsInfo)
            {
                if (std::get<5>(groupInfo) > 0)
                {
                    mPendingGroupQueue.push(std::make_pair(key, std::get<1>(groupInfo)));
                }
                std::set<std::string> empty;
                mPendingGroupsSibling.insert_or_assign(std::get<1>(groupInfo), empty);
            }
        }
    }

    void RSBrokerManager::pushPendingConsumers(const std::string &key, const std::string &group)
    {
        std::vector<ConsumerInfoType> consumersInfo;
        auto consumersInfoResult = RSBrokerHelper::getConsumersInfo(mRedis, key, group, consumersInfo);
        if (consumersInfoResult.has_value() && consumersInfoResult.value() > 0)
        {

            if (mPendingGroupsSibling.find(group) != mPendingGroupsSibling.end())
            {
            }

            for (const auto &consumerInfo : consumersInfo)
            {
                if (std::get<3>(consumerInfo) > 0)
                {
                    mPendingConsumerQueue.push(std::make_tuple(key, group, std::get<1>(consumerInfo), std::get<3>(consumerInfo)));
                }

                if (mPendingGroupsSibling.find(group) != mPendingGroupsSibling.end())
                {
                    mPendingGroupsSibling[group].insert(std::get<1>(consumerInfo));
                }
            }
        }
    }

    std::string RSBrokerManager::getOneSibling(const std::string &group, const std::string &consumer)
    {
        if (mPendingGroupsSibling.find(group) != mPendingGroupsSibling.end())
        {
            auto it = std::find_if(mPendingGroupsSibling[group].cbegin(), mPendingGroupsSibling[group].cend(), [&](std::string c)
                                   { return c != consumer; });

            if (it != mPendingGroupsSibling[group].cend())
                return *it;
        }
        return {};
    }

    void RSBrokerManager::handlePending()
    {
        for (const auto &key : mKeys)
        {
            pushPendingGroups(key);
        }

        //LOG_INFO << "pending group size: " << mPendingGroupQueue.size();

        while (!mPendingGroupQueue.empty())
        {
            pushPendingConsumers(mPendingGroupQueue.front().first, mPendingGroupQueue.front().second);
            mPendingGroupQueue.pop();
        }

        while (!mPendingConsumerQueue.empty())
        {
            auto [key, group, consumer, pendingNum] = mPendingConsumerQueue.front();

            std::vector<broker::PendingItemType> pendingItems;

            auto pendingResult = RSBrokerHelper::getPending(mRedis, key, group, consumer, pendingNum, pendingItems);

            if (pendingResult.has_value() && pendingResult.value() > 0)
            {
                for (const auto &pendingItem : pendingItems)
                {
                    auto [id, c, pendingMilliseconds, deliveredNum] = pendingItem;

                    // reach the max delivered times, considered to be dead letter, del it
                    if (static_cast<size_t>(deliveredNum) > mMaxDelivered)
                    {
                        LOG_INFO << id << " has been delivered max times, is almost certainly dead";

                        RSBrokerHelper::ack(mRedis, key, group, id);
                        RSBrokerHelper::del(mRedis, key, id);

                        continue;
                    }

                    // pending too long time, just xack it
                    if (static_cast<size_t>(pendingMilliseconds) > mTolerantMilliseconds)
                    {
                        LOG_INFO << id << " has exceeded to tolerate timeout";

                        RSBrokerHelper::ack(mRedis, key, group, id);

                        continue;
                    }

                    // transfer it to sibling
                    if (static_cast<size_t>(pendingMilliseconds) > mTransferMilliseconds)
                    {
                        LOG_INFO << c << " try to transfer " << id << " to some sibling";

                        auto sibling = getOneSibling(group, c);

                        if (!sibling.empty())
                        {
                            LOG_INFO << id << " will be transfered to " << sibling;
                            ItemStreamType dump;
                            RSBrokerHelper::claim(mRedis, key, group, sibling, mTransferMilliseconds, id, dump);
                        }
                        else
                        {
                            LOG_INFO << c << " has no sibling";
                        }
                    }
                }
            }

            mPendingConsumerQueue.pop();
        }
    }
}