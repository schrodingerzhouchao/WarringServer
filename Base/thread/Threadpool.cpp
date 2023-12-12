#include "Threadpool.h"
#include <numeric>
#include <cmath>
#include <algorithm>

namespace base
{
    namespace thd
    {
        void Threadpool::setTimer(const std::string &id, TimerType which)
        {
            auto timerPtr = mTimeWheelTimerWeak.lock();
            if (timerPtr)
            {
                if (which == TimerType::ADJUST_THREAD_NUMBER)
                {
                    utility::timer::TimerTask tt(0, 0, 5, -1, id, std::bind(&Threadpool::adjustThdNumTimeoutNotify, this));
                    timerPtr->addTask(tt);
                    mAdjustThdNumTaskID = id;
                }
                else if (which == TimerType::COLLECT_THREAD_STATE)
                {
                }
                else if (which == TimerType::TODO_MORE)
                {
                }
            }
            else
                std::cout << "todo get timewheel timer ptr error\n";
        }

        void Threadpool::expandThreads(const size_t num)
        {
            auto n = num;
            if ((mActiveNum + num) > mMaxNum)
                n = mMaxNum - mActiveNum;
            size_t res = 0;

            for (size_t i = mHardwareNum; i < mActiveNum; ++i)
            {
                if (mThreads.at(i)->isInactive())
                {
                    // std::cout << "HOLE\n";
                    auto thdPtr = std::make_unique<RThread<uint16_t>>();
                    if (thdPtr->linkExternalTasks(mTasks))
                    {
                        mThreads.at(i).swap(thdPtr);
                        --n;
                        ++res;
                    }
                }
            }

            for (size_t i = 0; i < n; ++i)
            {
                auto thdPtr = std::make_unique<RThread<uint16_t>>();
                if (thdPtr->linkExternalTasks(mTasks))
                {
                    mThreads.push_back(std::move(thdPtr));
                    ++res;
                }
            }
            mActiveNum += res;
            std::cout << "expandThreads()" << mActiveNum << " " << mThreads.size() << std::endl;
        }

        void Threadpool::shrinkThreads(const size_t num)
        {
            auto n = mActiveNum - num;

            if (n < mHardwareNum)
                return;

            for (size_t i = mActiveNum - 1; i >= n; --i)
            {
                mThreads.at(i)->command(RThread<uint16_t>::CommandType::CANCEL_WHEN_COMPLETE);
            }
            mActiveNum = n;
            std::cout << "shrinkThreads()" << mActiveNum << " " << mThreads.size() << std::endl;
        }

        void Threadpool::getExpectationAndAdjust(const bool countTimeout, size_t &total, size_t &n)
        {
            if (!countTimeout)
            {
                uint16_t res = 0;
                mResults->waitAndPop(res);
                total += res;
                ++n;
                // std::cout << "total = " << total << " n = " << n << std::endl;
            }
            else
            {
                uint16_t ratio = total / n;
                mExpectation = mHardwareNum * (1 + ratio);
                //mExpectation = total/n;
                std::cout << "get expectation: " << mExpectation << std::endl;
                if (mExpectation > mActiveNum)
                {
                    expandThreads(mExpectation - mActiveNum);
                }
                else if (mExpectation < mActiveNum)
                {
                    shrinkThreads(mActiveNum - mExpectation);
                }

                total = n = 0;
                resetAdjustThdNumTimeout();
            }
        }

        void Threadpool::updateThreadInfo()
        {
            for (size_t i = 0; i < mThreads.size(); ++i)
            {
                ThreadInfo thdInfo{mThreads.at(i)->getID(),
                                   mThreads.at(i)->isRunning(),
                                   mThreads.at(i)->isInactive(),
                                   mThreads.at(i)->maxLoad(),
                                   mThreads.at(i)->size(),
                                   mThreads.at(i)->loadRatio()};
                mThreadsHash.insert_or_assign(i, thdInfo);
            }
        }

        void Threadpool::readyCaptureThdResults(size_t i)
        {
            if (!mThreads.at(i)->isCaptureFuture())
                mThreads.at(i)->command(RThread<uint16_t>::CommandType::CAPTURE_FUTURE_SWITCH);

            mThreads.at(i)->linkResultsQueue(mResults);
        }
        void Threadpool::cancelCollectThdResults(size_t i)
        {
            if (mThreads.at(i)->isCaptureFuture())
                mThreads.at(i)->command(RThread<uint16_t>::CommandType::CAPTURE_FUTURE_SWITCH);
        }

        double Threadpool::getExpectation(const std::vector<uint16_t> &data)
        {
            auto total = std::accumulate(data.begin(), data.end(), 0);
            return static_cast<double>(total) / data.size();
        }

        double Threadpool::getVariance(const double expectation, const std::vector<uint16_t> &data)
        {
            double total = 0.0;
            std::for_each(std::cbegin(data), std::cend(data), [&total](uint16_t x)
                          { total += (std::pow(x, 2)); });
            return total / data.size();
        }
    }
}