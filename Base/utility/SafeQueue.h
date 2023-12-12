/*
 * 
 */

#ifndef __UTILITY_THREAD_SAFE_QUEUE_H__
#define __UTILITY_THREAD_SAFE_QUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>

namespace utility
{

    template <typename T>
    class SafeQueue
    {
    public:
        SafeQueue() = default;
        ~SafeQueue()
        {
            clear();
        }

        SafeQueue(const SafeQueue &rhs)
        {
            std::lock_guard<std::mutex> lock(rhs.mMutex);
            mData = rhs.mData;
        }
        SafeQueue &operator=(const SafeQueue &rhs)
        {
            std::lock_guard<std::mutex> lock(rhs.mMutex);
            mData = rhs.mData;
        }

        void push(T &val)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mData.push(val);
            mCond.notify_one();
        }

        void push(T &&val)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mData.push(std::move(val));
            mCond.notify_one();
        }

        bool pop(T &val)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mData.empty())
                return false;
            val = std::move(mData.front());
            mData.pop();
            return true;
        }
#if 0
        bool pop(T &&val)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mData.empty())
                return false;
            val = std::move(mData.front());
            mData.pop();
            return true;
        }
#endif
        size_t popN(std::queue<T> &q, size_t n)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mData.empty())
                return 0;
            auto min = std::min(n, mData.size());
            for (size_t i = 0; i < min; ++i)
            {
                // q.push(mData.front());
                q.push(std::move(mData.front()));
                mData.pop();
            }
            return min;
        }

        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mData.empty())
                return std::shared_ptr<T>();
            auto res = std::make_shared<T>(mData.front());
            mData.pop();
            return res;
        }

        void waitAndPop(T &val)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCond.wait(lock, [this]
                       { return !mData.empty(); });
            val = mData.front();
            mData.pop();
        }

        bool waitAndPop(T &val, const size_t millisecs)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mCond.wait_for(lock, std::chrono::milliseconds(millisecs), [this]
                               { return !mData.empty(); }))
            {
                val = mData.front();
                mData.pop();
                return true;
            }
            else
            {
                return false;
            }
        }
        void waitAndPop(T &&val)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCond.wait(lock, [this]
                       { return !mData.empty(); });
            val = std::move(mData.front());
            mData.pop();
        }

        bool waitAndPop(T &&val, const size_t millisecs)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mCond.wait_for(lock, std::chrono::milliseconds(millisecs), [this]
                               { return !mData.empty(); }))
            {
                val = std::move(mData.front());
                mData.pop();
                return true;
            }
            else
            {
                return false;
            }
        }
        std::shared_ptr<T> waitAndPop()
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCond.wait(lock, [this]
                       { return !mData.empty(); });
            auto res = std::make_shared<T>(mData.front());
            mData.pop();
            return res;
        }

        std::shared_ptr<T> waitAndPop(const size_t millisecs)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mCond.wait_for(lock, std::chrono::milliseconds(millisecs), [this]
                               { return !mData.empty(); }))
            {
                auto res = std::make_shared<T>(mData.front());
                mData.pop();
                return res;
            }
            else
            {
                return nullptr;
            }
        }

        void waitAndPopN(std::queue<T> &q, size_t n)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCond.wait(lock, [this]
                       { return !mData.empty(); });
            auto min = std::min(n, mData.size());
            for (size_t i = 0; i < min; ++i)
            {
                q.push(mData.front());
                mData.pop();
            }
        }

        bool empty()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mData.empty();
        }

        size_t size()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mData.size();
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            auto sz = mData.size();
            for (size_t i = 0; i < sz; ++i)
            {
                mData.pop();
            }
        }

    private:
        std::mutex mMutex;
        std::condition_variable mCond;
        std::queue<T> mData;
    };
}

#endif