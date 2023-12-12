

#ifndef __UTILITY_THREAD_SAFE_HASHMAP_H__
#define __UTILITY_THREAD_SAFE_HASHMAP_H__

#include <functional>
#include <utility>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <vector>

namespace utility
{
    template <typename Key, typename Value>
    class Bucket
    {
    public:
        bool find(const Key &key, Value &value) const
        {
            std::shared_lock<std::shared_mutex> lk(mSharedMutex);
            auto it = std::find_if(mData.begin(), mData.end(), [&](const std::pair<Key, Value> &node)
                                   { return node.first == key; });
            if (it == mData.end())
            {
                return false;
            }
            else
            {
                value = it->second;
                return true;
            }
        }

        bool find(const Key &key, std::shared_ptr<Value> &valptr) const
        {
            std::shared_lock<std::shared_mutex> lk(mSharedMutex);
            auto it = std::find_if(mData.begin(), mData.end(), [&](const std::pair<Key, Value> &node)
                                   { return node.first == key; });
            if (it == mData.end())
            {
                return false;
            }
            else
            {
                valptr = std::make_shared<Value>(it->second);
                return true;
            }
        }

        void insert(const Key &key, const Value &value)
        {
            std::unique_lock<std::shared_mutex> lk(mSharedMutex);
            auto it = std::find_if(mData.begin(), mData.end(), [&](const std::pair<Key, Value> &node)
                                   { return node.first == key; });
            if (it == mData.end())
            {
                mData.push_back(std::make_pair(key, value));
            }
        }

        void insertOrAssign(const Key &key, const Value &value)
        {
            std::unique_lock<std::shared_mutex> lk(mSharedMutex);
            auto it = std::find_if(mData.begin(), mData.end(), [&](const std::pair<Key, Value> &node)
                                   { return node.first == key; });
            if (it == mData.end())
            {
                mData.push_back(std::make_pair(key, value));
            }
            else
            {
                it->second = value;
            }
        }

        void erase(const Key &key)
        {
            std::unique_lock<std::shared_mutex> lk(mSharedMutex);
            auto it = std::find_if(mData.begin(), mData.end(), [&](const std::pair<Key, Value> &node)
                                   { return node.first == key; });
            if (it != mData.end())
            {
                mData.erase(it);
            }
        }

        bool empty() const
        {
            std::unique_lock<std::shared_mutex> lk(mSharedMutex);
            return mData.empty();
        }

        size_t size() const
        {
            std::unique_lock<std::shared_mutex> lk(mSharedMutex);
            return mData.size();
        }

        void readAll(std::function<void(const Value &)> func) const
        {
            std::unique_lock<std::shared_mutex> lk(mSharedMutex);
            for (const auto &it : mData)
            {
                func(it.second);
            }
        }

    private:
        std::list<std::pair<Key, Value>> mData;
        mutable std::shared_mutex mSharedMutex;
    };

    template <typename Key, typename Value, typename Hash = std::hash<Key>>
    class SafeHashmap
    {
    public:
        SafeHashmap(size_t bucketNum = 1031, const Hash &hashfunc = Hash())
            : mBucketSize{bucketNum},
              mHashFunc{hashfunc}
        {
            if (mBucketSize <= 0)
            {
                mBucketSize = 1031;
            }

            mHashtable.reserve(mBucketSize);

            for (size_t i = 0; i < bucketNum; ++i)
            {
                mHashtable.push_back(std::make_unique<Bucket<Key, Value>>());
            }
        }
        SafeHashmap(const SafeHashmap &) = delete;
        SafeHashmap &operator=(const SafeHashmap &) = delete;
        SafeHashmap(SafeHashmap &&) = delete;
        SafeHashmap &operator=(SafeHashmap &&) = delete;
        ~SafeHashmap() { mHashtable.clear(); }

        bool find(const Key &key, Value &value) const
        {
            return mHashtable.at(mHashFunc(key) % mBucketSize)->find(key, value);
        }
        bool find(const Key &key, std::shared_ptr<Value> &valPtr) const
        {
            return mHashtable.at(mHashFunc(key) % mBucketSize)->find(key, valPtr);
        }

        void insert(const Key &key, const Value &value)
        {
            mHashtable.at(mHashFunc(key) % mBucketSize)->insert(key, value);
        }
        void insertOrAssign(const Key &key, const Value &value)
        {
            mHashtable.at(mHashFunc(key) % mBucketSize)->insert(key, value);
        }
        void erase(const Key &key)
        {
            mHashtable.at(mHashFunc(key) % mBucketSize)->erase(key);
        }

        size_t bucketSize() const { return mBucketSize; }

        size_t size() const
        {
            auto res = 0;
            for (const auto &bucket : mHashtable)
            {
                res += bucket->size();
            }

            return res;
        }

        void readAll(std::function<void(const Value &)> func, size_t index) const
        {
            if (index >= mBucketSize)
                return;
            mHashtable.at(index)->readAll(func);
        }

    private:
        size_t mBucketSize;
        Hash mHashFunc;
        std::vector<std::unique_ptr<Bucket<Key, Value>>> mHashtable;
    };

}

#endif