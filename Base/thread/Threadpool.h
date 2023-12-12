/*
 * thread pool's initial size = machine's core number
 * Number of thread = Available Cores * (1 + Wait time / Service time);
 * some thread(for example pool's index = 0) as special thread, which calculates ((Wait time) / (Service time))
 * accoring to above value adjust thread number dynamically
 */

#ifndef __BASE_THREADPOOL_H__
#define __BASE_THREADPOOL_H__

#include "RThread.h"
#include "../utility/Timer.h"
#include <functional>

#include <iostream>

namespace base
{
    namespace thd
    {
        class Threadpool
        {
        public:
            enum class TimerType
            {
                NONE,
                ADJUST_THREAD_NUMBER,
                COLLECT_THREAD_STATE,
                TODO_MORE
            };

        public:
            Threadpool()
                : mRunning{true},
                  mThread{&Threadpool::work, this}
            {
                mHardwareNum = std::thread::hardware_concurrency();
                mActiveNum = mHardwareNum <= mMaxNum ? mHardwareNum : mMaxNum;
                mThreads.reserve(mMaxNum);

                mTasks = std::make_shared<TaskWithFutureSafeQueueType<uint16_t>>();
                mResults = std::make_shared<utility::SafeQueue<uint16_t>>();

                for (size_t i = 0; i < mActiveNum; ++i)
                {
                    auto thdPtr = std::make_unique<RThread<uint16_t>>();
                    if (thdPtr->linkExternalTasks(mTasks))
                    {
                        mThreads.push_back(std::move(thdPtr));
                    }
                    else
                    {
                        std::cout << "TODO RThread " << i << " Link TaskWithFuture queue error\n";
                    }
                }

                if (!mThreads.at(SPECIAL_INDEX)->isCaptureFuture())
                    mThreads.at(SPECIAL_INDEX)->command(RThread<uint16_t>::CommandType::CAPTURE_FUTURE_SWITCH);
                readyCaptureThdResults(SPECIAL_INDEX);
            }

            Threadpool(const Threadpool &) = delete;
            Threadpool &operator=(const Threadpool &) = delete;
            Threadpool(Threadpool &&) = delete;
            Threadpool &operator=(Threadpool &&) = delete;

            ~Threadpool()
            {
#if 0
                for (size_t i = 0; i < mActiveNum; ++i)
                {
                    mThreads.at(i)->command(RThread<uint16_t>::CommandType::CANCEL_WHEN_COMPLETE);
                    mThreads.at(i)->join();
                }
#endif
                mThreads.clear();
                mThreadsHash.clear();

                // std::cout << "~Threadpool\n";
            }

            void getTimewheelTimer(const std::shared_ptr<utility::timer::TimeWheelTimer> &twTimerPtr) { mTimeWheelTimerWeak = twTimerPtr; }
            void setTimer(const std::string &id, TimerType which);

            template <typename F, typename... Args>
            void addTask(F &&f, Args &&...args)
            {
                using returnType = decltype(f(args...));
                using taskType = std::packaged_task<returnType()>;

                auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
                std::shared_ptr<taskType> task = std::make_shared<taskType>(std::move(func));

                base::thd::TaskWithFuture<uint16_t> twf;

                if constexpr (std::is_void_v<returnType>)
                {
                    twf.mFunc = std::move(func);
                }
                else
                {
                    twf.mFuture = task->get_future();
                    twf.mFunc = [task]() -> void
                    { (*task)(); };
                }

                mTasks->push(twf);
            }

            // expand threadpool, fill "holes" caused by shrinkage first
            void expandThreads(const size_t num);

            // shrink threadpool, do not remove the unique_ptr<RThread>, just inactive them when these threads finish the local tasks
            // will generate "holes" in threadpool, and will be filled when expanding
            void shrinkThreads(const size_t num);

            void readyCaptureThdResults(size_t i);
            void cancelCollectThdResults(size_t i);

            // calculate the expected number of threadpool and adjust it
            void getExpectationAndAdjust(const bool countTimeout, size_t &total, size_t &n);

            void updateThreadInfo();

            // update thread's info in threadpool 
            void collectThdState(const bool collect)
            {
                if (collect)
                {
                    updateThreadInfo();
                }
            }

            void join()
            {
                if (mThread.joinable())
                    mThread.join();
            }

        private:
            void adjustThdNumTimeoutNotify()
            {
                std::cout << "adjustThdNumTimeoutNotify\n";
                mAdjustThdNumTimeout = true;
            }
            void resetAdjustThdNumTimeout()
            {
                std::cout << "reset adjustThdNumTimeoutNotify\n";
                mAdjustThdNumTimeout = false;
            }
            void collectThreadStateTimeoutNotify() {}
            void resetCollectThreadStateTimeout() {}

            double getExpectation(const std::vector<uint16_t> &data);
            double getVariance(const double expectation, const std::vector<uint16_t> &data);

            void work()
            {
                size_t total = 0, n = 0;

                while (mRunning)
                {
                    {
                        std::unique_lock<std::mutex> lk(mMtx);
                        mCond.wait_for(lk, std::chrono::seconds(3), [this]
                                       { return (mIsAdjustThreadNum || mIsCollectThreadState); });
                    }
                    if (mIsAdjustThreadNum)
                        getExpectationAndAdjust(mAdjustThdNumTimeout, total, n);

                    collectThdState(mIsCollectThreadState);

                }
            }

        private:
            struct ThreadInfo
            {
                std::thread::id mID;
                bool mRunning = false;
                bool mInactive = false;
                size_t mMaxLoadSize = 0;
                size_t mCurrentSize = 0;
                float mRatio = 0.0f;
            };

        private:
            std::atomic_bool mRunning;
            std::thread mThread;
            size_t mHardwareNum;
            const size_t mMaxNum = MAX_THREADS_NUM();
            std::atomic_uint mActiveNum;

            const size_t SPECIAL_INDEX = 0;
            size_t mExpectation = 0;

            std::mutex mMtx;
            std::condition_variable mCond;

            bool mIsAdjustThreadNum = true;
            
            // check whether the special thread is timeout, which calculates the wait_time/serv_time
            bool mAdjustThdNumTimeout = false;
            bool mIsCollectThreadState = false;
            std::string mAdjustThdNumTaskID = "";
            std::string mCollectThdStaTaskID = "";

            std::unordered_map<size_t, ThreadInfo> mThreadsHash;

            std::shared_ptr<TaskWithFutureSafeQueueType<uint16_t>> mTasks;
            std::shared_ptr<utility::SafeQueue<uint16_t>> mResults;
            std::vector<std::unique_ptr<RThread<uint16_t>>> mThreads;

            std::weak_ptr<utility::timer::TimeWheelTimer> mTimeWheelTimerWeak;
        };
    }
}

#endif