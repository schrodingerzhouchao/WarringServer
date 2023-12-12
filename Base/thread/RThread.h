/*
 * thread with local task queue, only run the task within this queue
 * has load limit, push tasks in varying degrees(0.0~0.2~0.8~1.0) to avoid tasks concentration in a single thread
 * including pause,resume, cancel, update and so on
 */

#ifndef __BASE_RTHREAD_H__
#define __BASE_RTHREAD_H__

#include "ThreadParam.h"
#include <thread>
#include <atomic>
#include <functional>
#include <utility>
#include <algorithm>
#include <type_traits>

#include <iostream>

namespace base
{
    namespace thd
    {
        template <typename T>
        class RThread
        {
        public:
            enum class CommandType
            {
                NONE,                 // do nothing
                STOP,                 // stop one thread
                RESUME,               // resume one thread
                CANCEL,               // cancel one thread
                UPDATE_DATA,          // update parameters
                CANCEL_WHEN_COMPLETE, // cancel when threads finished
                CAPTURE_FUTURE_SWITCH // whether capture result
            };

        public:
            RThread()
                : mRunning{false},
                  mStop{false},
                  mThread(&RThread::work, this)
            {
                mMiddleweight = mMaxLoadSize / 2;
                mLightweight = mMaxLoadSize / 4;
            }

            RThread(const RThread &) = delete;
            RThread &operator=(const RThread &) = delete;
            RThread(RThread &&) = delete;
            RThread &operator=(RThread &&) = delete;

            ~RThread()
            {
                mExternalTasksPtr.reset();
                mTasks.clear();
                join();
                std::cout << "~RThread " << mID << std::endl;
            }

            bool linkExternalTasks(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &extPtr);
            bool externalTasksLinked() const;

            bool linkResultsQueue(const std::shared_ptr<utility::SafeQueue<T>>& exPtr);
            bool resultsQueueLinked() const;

            bool isRunning() const { return mRunning; }
            bool isCaptureFuture() const { return mCaptureFuture; }
            bool isInactive() const { return mInactive; }

            size_t size() { return mTasks.size(); }
            size_t maxLoad() { return mMaxLoadSize; }

            float loadRatio() { return static_cast<float>(mTasks.size()) / mMaxLoadSize; }

            std::pair<float, size_t> getRatioAndMax();

            void command(CommandType cmd, const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &extPtr = nullptr,
                         size_t maxSize = 10, size_t middleweightDiv = 2, size_t lightweightDiv = 4);

            std::thread::id getID() { return mID; }
            size_t getIDInt() const
            {
                size_t res = *(size_t *)&mID;
                return res;
            }
            std::string getIDStr() const { return std::to_string(getIDInt()); }

            T getTaskResult() const { return mTaskResult; }

            void join();

        private:
            int addTasks(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &p, bool wait = false, size_t n = 1);

            // add tasks by load degree
            void addByLevel(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &p, const size_t middleweight, const size_t lightweight);

            void stop();

            void resume(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &extPtr);

            void cancel();

            void cancelWhenComplete();

            // update part parameters
            void updateMember(const size_t maxLoadSize, const size_t middleweightDiv, const size_t lightweightDiv,
                              const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &p);

            void work();

        private:
            CommandType cmd = CommandType::NONE;

            std::atomic_bool mRunning;
            std::atomic_bool mStop;
            bool mInactive = false;
            bool mWipe = false;

            std::mutex mMtx;
            std::condition_variable mCond;

            std::thread mThread;
            std::thread::id mID;

            size_t mMaxLoadSize = RTHREAD_MAX_LOAD_NUMBER();
            const float mLowpoint = 0.2;
            const float mHighpoint = 0.8;

            size_t mMiddleweight = 2;
            size_t mLightweight = 1;

            size_t mGetTaskWaitMillieconds = 3000;

            bool mCaptureFuture = false;
            T mTaskResult;
            TaskWithFutureSafeQueueType<T> mTasks;
            std::weak_ptr<TaskWithFutureSafeQueueType<T>> mExternalTasksPtr;
            std::weak_ptr<utility::SafeQueue<T>> mResultsQueuePtr;
        };


        template <typename T, typename F, typename... Args>
        void addTask(std::shared_ptr<TaskWithFutureSafeQueueType<T>> &taskWithFutQueuePtr, F &&f, Args &&...args)
        {
            using returnType = decltype(f(args...));
            using taskType = std::packaged_task<returnType()>;

            auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            std::shared_ptr<taskType> task = std::make_shared<taskType>(std::move(func));

            base::thd::TaskWithFuture<T> twf;

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

            taskWithFutQueuePtr->push(twf);
        }

    }
}

#include "RThread.inl"

#endif
