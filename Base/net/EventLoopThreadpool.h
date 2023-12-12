#ifndef __NET_EVENT_LOOP_THREADPOOL_H__
#define __NET_EVENT_LOOP_THREADPOOL_H__

#include "EventLoopThread.h"

namespace net
{
    namespace event
    {
        class EventLoopThreadpool
        {
        public:
            enum class DistributionType
            {
                DEFAULT,
                ROUND_ROBIN,
                FEWER_EVENTS,
                FASTER_HANDLED
            };

        public:
            EventLoopThreadpool(const size_t num, const PollerType which)
            {
                auto hardwareNum = std::thread::hardware_concurrency();
                mThreadNum = num > hardwareNum ? hardwareNum : num;

                for (size_t i = 0; i < num; ++i)
                {
                    auto evThd = std::make_unique<EventLoopThread>(which);
                    mEventLoopThreads.push_back(std::move(evThd));
                }
            }

            EventLoopThreadpool(const EventLoopThreadpool &) = delete;
            EventLoopThreadpool &operator=(const EventLoopThreadpool &) = delete;
            EventLoopThreadpool(EventLoopThreadpool &&) = delete;
            EventLoopThreadpool &operator=(EventLoopThreadpool &&) = delete;

            ~EventLoopThreadpool()
            {
            }

            bool isInactive(const size_t i) const { return (mEventLoopThreads.at(i) == nullptr) || (!mEventLoopThreads.at(i)->isLooping()); }
            const size_t getSize() const { return mThreadNum; }

            std::shared_ptr<EventLoop> getEventLoop(const int index) const
            {
                if (index < 0 || static_cast<size_t>(index) >= mThreadNum)
                    return nullptr;
                return mEventLoopThreads.at(index)->getEventLoop();
            }

            std::shared_ptr<EventLoop> getEventLoop(const std::thread::id tid) const
            {
                for (const auto &elThd : mEventLoopThreads)
                {
                    if (elThd->getThreadID() == tid)
                    {
                        return elThd->getEventLoop();
                    }
                }

                return nullptr;
            }

            int getDesiredIndex(DistributionType which)
            {
                if (which == DistributionType::DEFAULT || which == DistributionType::ROUND_ROBIN)
                {
                    return roundRobinIndex();
                }
                else if (which == DistributionType::FEWER_EVENTS)
                {
                    return fewerEventsIndex();
                }
                else if (which == DistributionType::FASTER_HANDLED)
                {
                    return fasterHandledIndex();
                }
                return -1;
            }

            bool distributionEvent(const std::shared_ptr<EventHandler> &evHandler, DistributionType which)
            {
                int index = -1;
                if (which == DistributionType::DEFAULT || which == DistributionType::ROUND_ROBIN)
                {
                    index = roundRobinIndex();
                    if (index >= 0)
                    {
                        mEventLoopThreads.at(index)->addEvent(evHandler);
                        return true;
                    }
                }
                else if (which == DistributionType::FEWER_EVENTS)
                {
                    index = fewerEventsIndex();
                    if (index >= 0)
                    {
                        mEventLoopThreads.at(index)->addEvent(evHandler);
                        return true;
                    }
                }
                else if (which == DistributionType::FASTER_HANDLED)
                {
                    index = fasterHandledIndex();
                    if (index >= 0)
                    {
                        mEventLoopThreads.at(index)->addEvent(evHandler);
                        return true;
                    }
                }
                return false;
            }

            std::vector<std::thread::id> getEventLoopThreadIDs() const
            {
                std::vector<std::thread::id> res;
                for (const auto &eventThread : mEventLoopThreads)
                {
                    res.push_back(eventThread->getThreadID());
                }

                return res;
            }

        private:
            // distribution by round-robin
            int roundRobinIndex()
            {
                int res = -1;

                auto tmp = ++mNextIndex;
                if (mNextIndex == mThreadNum)
                    mNextIndex = 0;
                for (size_t i = mNextIndex; i < tmp + mThreadNum; ++i)
                {
                    if (!isInactive(i % mThreadNum))
                    {
                        res = i % mThreadNum;
                        break;
                    }
                    else
                    {
                        mNextIndex = (mNextIndex + 1) % mThreadNum;
                    }
                }

                return res;
            }

            // distribution by fewer-events-eventloop
            int fewerEventsIndex()
            {
                int res = -1;
                size_t fewer = std::numeric_limits<size_t>::max();
                for (size_t i = 0; i < mThreadNum; ++i)
                {
                    if (!isInactive(i) && fewer > mEventLoopThreads.at(i)->getEventSize())
                    {
                        fewer = mEventLoopThreads.at(i)->getEventSize();
                        res = i;
                    }
                }

                return res;
            }

            // distribution by faster-eventloop (AccumulatedTime/EnentsNum)
            int fasterHandledIndex()
            {
                int res = -1;
                double faster = std::numeric_limits<double>::max();
                for (size_t i = 0; i < mThreadNum; ++i)
                {
                    if (!isInactive(i) && faster > mEventLoopThreads.at(i)->getLastRate())
                    {
                        faster = mEventLoopThreads.at(i)->getLastRate();
                        res = i;
                    }
                }
                return res;
            }

        private:
            size_t mThreadNum;

            size_t mNextIndex = 0;

            std::vector<std::unique_ptr<EventLoopThread>> mEventLoopThreads;
        };
    }
}

#endif