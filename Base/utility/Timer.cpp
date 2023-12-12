#include "Timer.h"
#include <iostream>

size_t operator"" _hours(unsigned long long hours) { return hours; }
size_t operator"" _minutes(unsigned long long minutes) { return minutes; }
size_t operator"" _seconds(unsigned long long seconds) { return seconds; }
size_t operator"" _milliseconds(unsigned long long milliseconds) { return milliseconds; }

namespace utility
{
    namespace timer
    {

#if 0
        bool TimeWheelTimer::linkDestQueue(const std::shared_ptr<SafeTaskQueueType> &extPtr)
        {
            mDistPtr = extPtr;
            auto p = mDistPtr.lock();
            if (p)
                return true;
            return false;
        }

        bool TimeWheelTimer::isLinked() const
        {
            auto p = mDistPtr.lock();
            if (p)
                return true;
            return false;
        }
#endif
        std::string TimeWheelTimer::generateID()
        {
            return std::to_string(mNow.secondsSinceEpoch()) + '-' + std::to_string(++mIncrPerSec);
        }

        void TimeWheelTimer::addTask(TimerTask timerTask)
        {
            mTaskEntry.push(timerTask);
        }

        void TimeWheelTimer::registerTask()
        {
            while (mRunning)
            {
                TimerTask timerTask;
                // mTaskEntry.waitAndPop(timerTask);

                if (mTaskEntry.waitAndPop(timerTask, 100))
                {
                    auto cycNum = timerTask.mCycleNumber;
                    if (cycNum == 0)
                    {
                        eraseTimerTask(timerTask.mID);
                        continue;
                    }
                    else if (cycNum < 0 || cycNum > 1)
                    {
                        mRegisteredTasks.insert(std::make_pair(timerTask.mID, timerTask));
                    }
                    addTimerNodeFor(timerTask);
                }
            }

            //std::cout << "TimeWheelTimer::registerTask() stop\n";
        }

        void TimeWheelTimer::addTimerNodeFor(TimerTask timerTask)
        {
            auto hours = timerTask.mHours;
            auto minutes = timerTask.mMinutes;
            auto seconds = timerTask.mSeconds;

            if (hours == 0 && minutes == 0 && seconds == 0)
                return;

            auto hrSlotNo = (hours + mHours - 1) % mHourSlotsNum;
            auto minSlotNo = (minutes + mMinutes - 1) % mMinuteSlotsNum;
            auto secSlotNo = (seconds + mSeconds) % mSecondSlotsNum;

            timerTask.offset(mHours, mMinutes, mSeconds);

            TimerNode timerNode(timerTask);
            if (hours != 0)
            {
                mTimeWheel.at(hrSlotNo + mSecondSlotsNum + mMinuteSlotsNum)->push_back(timerNode);
            }
            else if (minutes != 0)
            {
                mTimeWheel.at(minSlotNo + mSecondSlotsNum)->push_back(timerNode);
            }
            else
            {
                mTimeWheel.at(secSlotNo)->push_back(timerNode);
            }
#if 0
            std::cout << "add a new timer: "
                      << "id " << timerTask.mID << " | "
                      << hours << "hours " << minutes << "minutes " << seconds << "seconds"
                      << " at: " << mNow.getNow() << std::endl;
#endif
        }

        void TimeWheelTimer::tick()
        {
            auto p = mDistPtr.lock();

            while (mRunning)
            {
                for (size_t i = 0; i < mSecondSlotsNum; ++i)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    mIncrPerSec = 0;

                    checkSecondWheel(mSeconds, p);
                    ++mSeconds;
                    if (mSeconds == mSecondSlotsNum)
                    {
                        checkMinuteWheel(mMinutes + mSecondSlotsNum, p);
                        mSeconds = 0;
                        ++mMinutes;
                        if (mMinutes == mMinuteSlotsNum)
                        {
                            checkHourWheel((mHours % mHourSlotsNum) + mMinuteSlotsNum + mSecondSlotsNum, p);
                            mSeconds = 0;
                            mMinutes = 0;
                            ++mHours;
                        }
                    }
                    // std::cout << "**ticking**" << mHours << " hours\t" << mMinutes << " minutes\t" << mSeconds << " seconds\thash size=" << mRegisteredTasks.size() << std::endl;
                }
            }

            // std::cout << "TimeWheelTimer::tick() stop\n";
        }

        void TimeWheelTimer::checkSecondWheel(const size_t slotNo, const std::shared_ptr<SafeTaskQueueType> &extPtr)
        {
            if (mTimeWheel.at(slotNo)->empty())
                return;
            for (auto it = mTimeWheel.at(slotNo)->begin(); it != mTimeWheel.at(slotNo)->end();)
            {
                auto cycNum = it->getCycNum();
                auto taskf = it->getTaskFunc();

                if (taskf)
                {
                    // std::cout << "taskf() " << std::this_thread::get_id() << std::endl;
                    taskf();
                }
#if 0
                if (!extPtr)
                    taskf();
                else
                    extPtr->push(taskf);
#endif
                if (cycNum == 1)
                {
                    auto id = it->getId();
                    eraseTimerTask(id);
                }
                else if (cycNum > 1 || cycNum < 0)
                {
                    auto id = it->getId();
                    repeatTimerTask(id);
                }
                mTimeWheel.at(slotNo)->erase(it++);
            }
        }
        void TimeWheelTimer::checkMinuteWheel(const size_t slotNo, const std::shared_ptr<SafeTaskQueueType> &extPtr)
        {
            if (mTimeWheel.at(slotNo)->empty())
                return;
            for (auto it = mTimeWheel.at(slotNo)->begin(); it != mTimeWheel.at(slotNo)->end();)
            {
                if (!it->hasSeconds()) // ?10
                {
                    auto taskf = it->getTaskFunc();
                    mTimeWheel.at(slotNo)->erase(it++);

                    if (taskf)
                        taskf();
#if 0
                    if (!extPtr)
                        taskf();
                    else
                        extPtr->push(taskf);
#endif
                }
                else
                {
                    auto task = it->getTimerTask();
                    auto secSlotNo = (it->getSeconds() + mSeconds) % mSeconds;
                    mTimeWheel.at(slotNo)->erase(it++);
                    mTimeWheel.at(secSlotNo)->push_back(task);
                }
            }
        }

        void TimeWheelTimer::checkHourWheel(const size_t slotNo, const std::shared_ptr<SafeTaskQueueType> &extPtr)
        {
            if (mTimeWheel.at(slotNo)->empty())
                return;

            for (auto it = mTimeWheel.at(slotNo)->begin(); it != mTimeWheel.at(slotNo)->end();)
            {
                if (it->getRound() == 0)
                {
                    if (it->hasMinutes()) // 11?
                    {
                        auto task = it->getTimerTask();
                        auto minSlotNo = (it->getMinutes() + mMinutes) % mMinuteSlotsNum + mSecondSlotsNum - 1;
                        mTimeWheel.at(slotNo)->erase(it++);
                        mTimeWheel.at(minSlotNo)->push_back(task);
                    }
                    else if (it->hasSeconds()) // 101
                    {
                        auto task = it->getTimerTask();
                        auto secSlotNo = (it->getSeconds() + mSeconds) % mSecondSlotsNum;
                        mTimeWheel.at(slotNo)->erase(it++);
                        mTimeWheel.at(secSlotNo)->push_back(task);
                    }
                    else // 100
                    {
                        auto taskf = it->getTaskFunc();
                        mTimeWheel.at(slotNo)->erase(it++);

                        if (taskf)
                            taskf();
#if 0
                        if (!extPtr)
                            taskf();
                        else
                            extPtr->push(taskf);
#endif
                    }
                }
                else
                {
                    it->subRound();
                    ++it;
                    continue;
                }
            }
        }

        void TimeWheelTimer::repeatTimerTask(const std::string &key)
        {
            auto it = mRegisteredTasks.find(key);
            if (it == mRegisteredTasks.end())
            {
                return;
            }
            if (it->second.mCycleNumber > 0)
                --(it->second.mCycleNumber);

            TimerTask timerTask(it->second);
            addTimerNodeFor(timerTask);
        }
        void TimeWheelTimer::eraseTimerTask(const std::string &key)
        {
            auto it = mRegisteredTasks.find(key);
            if (it == mRegisteredTasks.end())
            {
                return;
            }
            mRegisteredTasks.erase(it);
        }
    }
}