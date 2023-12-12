
/**
 * A timer based on time wheels, with precision of second.
 * All timer tasks submitted through a safe queue for thread-safe.
 * Repetitive tasks will be saved and woken up at the right time repeatedly, one-time tasks just pass by once.
 **/

#ifndef __UTILITY_TIMEWHEEL_TIMER_H__
#define __UTILITY_TIMEWHEEL_TIMER_H__

#include "SafeQueue.h"
#include "Time.h"

#include <vector>
#include <array>
#include <list>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

size_t operator"" _hours(unsigned long long hours);
size_t operator"" _minutes(unsigned long long minutes);
size_t operator"" _seconds(unsigned long long seconds);
size_t operator"" _milliseconds(unsigned long long milliseconds);

namespace utility
{
    namespace timer
    {

        // TimerTask(HH,MM,SS,cycNum,id,taskFunc)
        // @cycNum
        //          -1: all-the-time task
        //          0:  remove task
        //          1:  one-time task
        //          n:  n-time task
        struct TimerTask
        {
            TimerTask() = default;
            TimerTask(const size_t hrs, const size_t mins, const size_t secs, const int cycNum, std::string id, std::function<void()> task)
                : mCycleNumber{cycNum},
                  mID{id},
                  mTask{task}
            {
                mSeconds = secs % 60;
                auto minCarry = secs / 60;
                mMinutes = (mins + minCarry) % 60;
                auto hrCarry = (mins + minCarry) / 60;
                mHours = hrs + hrCarry;
            }

            void offset(const size_t hrs, const size_t mins, const size_t secs)
            {
                auto carryMin = (mSeconds + secs) / 60;
                auto carryHr = (mMinutes + mins + carryMin) / 60;
                mHours += hrs + carryHr;
                mMinutes = (mMinutes + mins) % 60;
                mSeconds = (mSeconds + secs) % 60;
            }

            size_t mHours = 0;
            size_t mMinutes = 0;
            size_t mSeconds = 0;
            int mCycleNumber = 0;
            std::string mID;
            std::function<void()> mTask;
        };

        class TimerNode
        {
        public:
            TimerNode(TimerTask timerTask)
                : mTimerTask{timerTask}
            {
                if (mTimerTask.mSeconds > 0)
                    mHasSeconds = true;
                if (mTimerTask.mMinutes > 0)
                    mHasMinutes = true;
                if (mTimerTask.mHours > 0)
                    mHasHours = true;
                if (mTimerTask.mHours >= 24)
                    mRound = mTimerTask.mHours / 24;
            }
            TimerNode(const TimerNode &tn) = default;
            TimerNode operator=(const TimerNode &tn)
            {
                if (this == &tn)
                    return *this;

                TimerNode tmp(tn);
                std::swap(*this, tmp);
                return *this;
            }

            void setRound(size_t round) { mRound = round; }
            bool subRound()
            {
                if (mRound == 0)
                    return false;
                --mRound;
                return true;
            }
            bool hasSeconds() const { return mHasSeconds; }
            bool hasMinutes() const { return mHasMinutes; }
            bool hasHours() const { return mHasHours; }
            size_t getSeconds() const { return mTimerTask.mSeconds; }
            size_t getMinutes() const { return mTimerTask.mMinutes; }
            size_t getHours() const { return mTimerTask.mHours; }
            TimerTask getTimerTask() const { return mTimerTask; }
            std::string getId() const { return mTimerTask.mID; }
            int getCycNum() const { return mTimerTask.mCycleNumber; }
            auto getTaskFunc() const { return mTimerTask.mTask; }
            size_t getRound() const { return mRound; }

        private:
            TimerTask mTimerTask;
            bool mHasSeconds = false;
            bool mHasMinutes = false;
            bool mHasHours = false;
            size_t mRound = 0;
        };

        class TimeWheelTimer
        {
        private:
            typedef utility::SafeQueue<std::function<void()>> SafeTaskQueueType;

        public:
            TimeWheelTimer()
                : mRunning{true},
                  mThread(&TimeWheelTimer::tick, this),
                  mGetAndRegisterThread(&TimeWheelTimer::registerTask, this)
            {
                size_t total = mSecondSlotsNum + mMinuteSlotsNum + mHourSlotsNum;
                for (size_t i = 0; i < total; ++i)
                {
                    mTimeWheel.push_back(std::make_unique<std::list<TimerNode>>());
                }
            }

            TimeWheelTimer(const TimeWheelTimer &) = delete;
            TimeWheelTimer(TimeWheelTimer &&) = delete;
            TimeWheelTimer &operator=(const TimeWheelTimer &) = delete;
            TimeWheelTimer &operator=(TimeWheelTimer &&) = delete;

            ~TimeWheelTimer()
            {
                join();
                mTimeWheel.clear();
                mTaskEntry.clear();
                mRegisteredTasks.clear();
            }

            // bool linkDestQueue(const std::shared_ptr<SafeTaskQueueType> &extPtr);
            // bool isLinked() const;

            // id = seconds since epoch + increment in one second
            std::string generateID();

            // add a timer task
            // note: keep the task as simple as possible, just like change one on-off value, bucasue it runs on Timer thread
            void addTask(TimerTask timerTask);

            void stop() { mRunning = false; }

            // Destructor call this according to RAII, so no need to call actively
            void join()
            {
                if (mGetAndRegisterThread.joinable())
                    mGetAndRegisterThread.join();
                if (mThread.joinable())
                    mThread.join();
            }

        private:
            void tick();
            void checkSecondWheel(const size_t slotNo, const std::shared_ptr<SafeTaskQueueType> &extPtr = nullptr);
            void checkMinuteWheel(const size_t slotNo, const std::shared_ptr<SafeTaskQueueType> &extPtr = nullptr);
            void checkHourWheel(const size_t slotNo, const std::shared_ptr<SafeTaskQueueType> &extPtr = nullptr);

            // Add a new node, insert to the hour slot first if the task's HH is valid, then minute slot, and second slot finally
            void addTimerNodeFor(TimerTask timerTask);

            void registerTask();
            void repeatTimerTask(const std::string &key);
            void eraseTimerTask(const std::string &key);

        private:
            const size_t mSecondSlotsNum = 60;
            const size_t mMinuteSlotsNum = 60;
            const size_t mHourSlotsNum = 24;

            size_t mSeconds = 0;
            size_t mMinutes = 0;
            size_t mHours = 0;

            utility::Time mNow;
            std::atomic<size_t> mIncrPerSec = 0;

            bool mRunning;

            // Thread used to run 24-hour-clock tick, bound with tick()
            std::thread mThread;
            // Thread used to receive task and register task if needed, bound with registerTask()
            std::thread mGetAndRegisterThread;

            std::vector<std::unique_ptr<std::list<TimerNode>>> mTimeWheel;

            SafeQueue<TimerTask> mTaskEntry;
            // Save repetitive tasks
            std::unordered_map<std::string, TimerTask> mRegisteredTasks;

            // should be deprecated
            std::weak_ptr<SafeTaskQueueType> mDistPtr;
        };

    }
}

#endif