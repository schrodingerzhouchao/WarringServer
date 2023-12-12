#include "RThread.h"

#include <iostream>

namespace base
{
    namespace thd
    { 
        /*
         * public
         */

        template <typename T>
        bool RThread<T>::linkExternalTasks(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &extPtr)
        {
            mExternalTasksPtr = extPtr;
            auto p = mExternalTasksPtr.lock();
            if (p)
                return true;
            return false;
        }

        template <typename T>
        bool RThread<T>::externalTasksLinked() const
        {
            auto p = mExternalTasksPtr.lock();
            if (p)
                return true;
            return false;
        }
        template <typename T>
        bool RThread<T>::linkResultsQueue(const std::shared_ptr<utility::SafeQueue<T>> &exPtr)
        {
            mResultsQueuePtr = exPtr;
            auto p = mResultsQueuePtr.lock();
            if (p)
                return true;
            return false;
        }
        template <typename T>
        bool RThread<T>::resultsQueueLinked() const
        {
            auto p = mResultsQueuePtr.lock();
            if (p)
                return true;
            return false;
        }

        template <typename T>
        std::pair<float, size_t> RThread<T>::getRatioAndMax()
        {
            auto res = std::make_pair(loadRatio(), mMaxLoadSize);
            return res;
        }

        template <typename T>
        void RThread<T>::command(CommandType cmd, const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &extPtr,
                                 size_t maxSize, size_t middleweightDiv, size_t lightweightDiv)
        {
            if (cmd == CommandType::NONE)
            {
                return;
            }
            if (cmd == CommandType::STOP)
            {
                stop();
                return;
            }
            if (cmd == CommandType::RESUME)
            {
                resume(extPtr);
                return;
            }
            if (cmd == CommandType::CANCEL)
            {
                cancel();
                return;
            }
            if (cmd == CommandType::UPDATE_DATA)
            {
                updateMember(maxSize, middleweightDiv, lightweightDiv, extPtr);
                return;
            }
            if (cmd == CommandType::CANCEL_WHEN_COMPLETE)
            {
                cancelWhenComplete();
                return;
            }
            if (cmd == CommandType::CAPTURE_FUTURE_SWITCH)
            {
                if (mCaptureFuture)
                    mCaptureFuture = false;
                else
                    mCaptureFuture = true;
                return;
            }
        }

        template <typename T>
        void RThread<T>::join()
        {
            if (mThread.joinable())
                mThread.join();
        }

        /*
         * private
         */

        template <typename T>
        int RThread<T>::addTasks(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &p, bool wait, size_t n)
        {
            if (n < 1)
                return -1;
            if (p == nullptr)
                return -1;

            TaskWithFuture<T> taskWithFut;

            if (n == 1)
            {
                if (wait)
                {
                    // p->waitAndPop(taskWithFut);
                    if (p->waitAndPop(taskWithFut, mGetTaskWaitMillieconds))
                    {
                        mTasks.push(taskWithFut);
                        return 1;
                    }
                    return 0;
                }
                else
                {
                    if (p->pop(taskWithFut))
                    {
                        mTasks.push(taskWithFut);
                        return 1;
                    }
                    return 0;
                }
            }
            auto x = std::min(n, mMaxLoadSize / 2);
            std::queue<TaskWithFuture<T>> temp;
            p->popN(temp, x);
            auto sz = temp.size();
            if (sz == 0)
                return 0;
            for (size_t i = 0; i < sz; ++i)
            {
                mTasks.push(temp.front());
                temp.pop();
            }
            return sz;
        }

        template <typename T>
        void RThread<T>::addByLevel(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &p, const size_t middleweight, const size_t lightweight)
        {
            if (!p)
                return;
            auto rate = loadRatio();
            if (rate >= mHighpoint)
            {
                // std::cout << "[0.8 max]****rate = " << rate << std::endl;
                return;
            }
            if (rate < mHighpoint && rate >= mLowpoint)
            {
                // std::cout << "[0.2 0.8)****rate = " << rate << std::endl;
                auto x = addTasks(p, false, lightweight);
                return;
            }
            if (rate < mLowpoint)
            {
                // std::cout << "[0.0 0.2)****rate = " << rate << std::endl;
                auto x = addTasks(p, false, middleweight);
                return;
            }
        }

        template <typename T>
        void RThread<T>::stop()
        {
            mStop = true;
            mCond.notify_one();
            mExternalTasksPtr.reset();
        }

        template <typename T>
        void RThread<T>::resume(const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &extPtr)
        {
            mStop = false;
            mExternalTasksPtr = extPtr;
            mCond.notify_one();
        }

        template <typename T>
        void RThread<T>::cancel()
        {
            mRunning = false;
            mExternalTasksPtr.reset();
            mTasks.clear();
        }

        template <typename T>
        void RThread<T>::cancelWhenComplete()
        {
            mRunning = false;
            mWipe = true;
            mExternalTasksPtr.reset();
        }

        template <typename T>
        void RThread<T>::updateMember(const size_t maxLoadSize, const size_t middleweightDiv, const size_t lightweightDiv,
                                      const std::shared_ptr<TaskWithFutureSafeQueueType<T>> &p)
        {
            mMaxLoadSize = maxLoadSize;
            mMiddleweight = mMaxLoadSize / middleweightDiv;
            mLightweight = mMaxLoadSize / lightweightDiv;
            mExternalTasksPtr = p;
        }

        template <typename T>
        void RThread<T>::work()
        {
            mID = std::this_thread::get_id();
            mRunning = true;

            TaskWithFuture<T> taskWithFut;

            while (mRunning)
            {
                {
                    std::unique_lock<std::mutex> lk(mMtx);
                    mCond.wait(lk, [this]
                               { return mStop == false; });
                }

                auto p = mExternalTasksPtr.lock();

                if (!mTasks.empty())
                {
                    mTasks.pop(taskWithFut);
                    taskWithFut.mFunc();

                    if (mCaptureFuture && taskWithFut.mFuture.valid())
                    {
                        mTaskResult = taskWithFut.mFuture.get();
                        //std::cout << "capture task result: " << getTaskResult() << std::endl;
                        auto p = mResultsQueuePtr.lock();
                        if (p)
                        {
                            p->push(mTaskResult);
                        }
                    }
                }
                else
                {
                    if (!p)
                        return;
                    // std::cout << "[0.0 0.0]****rate = " << loadRate() << std::endl;
                    auto x = addTasks(p, true);
                }

                addByLevel(p, mMiddleweight, mLightweight);
            }

            if (mWipe)
            {
                while (!mTasks.empty())
                {
                    mTasks.pop(taskWithFut);
                    taskWithFut.mFunc();
                    if (mCaptureFuture && taskWithFut.mFuture.valid())
                    {
                        mTaskResult = taskWithFut.mFuture.get();
                        std::cout << "capture task result: " << mTaskResult << std::endl;
                        auto p = mResultsQueuePtr.lock();
                        if (p)
                        {
                            p->push(mTaskResult);
                        }
                    }
                }
            }
            mInactive = true;
        }
    }
}