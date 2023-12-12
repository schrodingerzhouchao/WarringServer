
/*
 *
 *
 */

#ifndef __NET_EVENT_LOOP_H__
#define __NET_EVENT_LOOP_H__

#include "NetParam.h"
#include "EventHandler.h"
#include "PollPoller.h"
#include "EpollPoller.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include <thread>
#include <memory>

namespace net
{
    namespace event
    {
        class EventLoop
        {
        public:
            EventLoop(PollerType pollerType)
                : mWakeupFD(createEventfd()),
                  mPollTimeoutMS(POLL_TIMEOUT_MS())
            {
                if (pollerType == PollerType::POLL_POLLER)
                    mPoller = std::make_shared<event::PollPoller>();
                else
                    mPoller = std::make_shared<event::EpollPoller>();

                mWakeupHandler = std::make_shared<event::EventHandler>(mWakeupFD);

                mWakeupHandler->setReadFunc(std::bind(&EventLoop::nap, this));
                mWakeupHandler->enableReading();
                mPoller->registerEventHandler(mWakeupHandler);
                mPoller->update(mWakeupHandler);
            }

            ~EventLoop()
            {
                mWakeupHandler->disableAll();
                mPoller->remove(mWakeupHandler);
                ::close(mWakeupFD);
            }

            void loop()
            {
                mRunning = true;

                while (mRunning)
                {
                    mEventHandlers.clear();
                    mPoller->poll(mPollTimeoutMS, mEventHandlers);
                    mIsDoingEvent = true;
                    for (const auto &eventHandler : mEventHandlers)
                    {
                        eventHandler->doEvnet();
                    }
                    mIsDoingEvent = false;
                }
            }
            void registerEventHandler(const std::shared_ptr<EventHandler> &evHandler)
            {
                mPoller->registerEventHandler(evHandler);
            }

            void update(const std::shared_ptr<EventHandler> &evHandler)
            {
                mPoller->update(evHandler);
            }

            void remove(const std::shared_ptr<EventHandler> &evHandler)
            {
                mPoller->remove(evHandler);
            }

            void wakeup()
            {
                uint64_t one = 1;
                ssize_t n = ::write(mWakeupFD, &one, sizeof one);
                if (n != sizeof(one))
                {
                    ::perror("TODO EventLoop::wakeup()");
                }
            }

            void quit()
            {
                mRunning = false;

                // if (!mIsDoingEvent)
                wakeup();
            }

            void setSettlementPoint(size_t settlementPoint) { mPoller->setSettlementPoint(settlementPoint); }

            // params should satisfy the conditions: intervalSeconds << timeoutSeconds << resetThresholdSeconds
            // note: no checks in the implementation
            void setTimeoutParam(time_t intervalSeconds, time_t timeoutSeconds, time_t resetThresholdSeconds)
            {
                mPoller->setTimeoutParam(intervalSeconds, timeoutSeconds, resetThresholdSeconds);
            }

            size_t getEventSize() const { return mPoller->eventSize(); }
            double getLastRate() const { return mPoller->getLastRate(); }
            bool isRunning() const { return mRunning; }

            // if true, also recount
            // bool isIntervalOut() { return mPoller->isIntervalOut(); }
            // if true, also recount
            // bool isTimeout() { return mPoller->isTimeout(); }

            Poller *testGetPoller() const { return mPoller.get(); }

        private:
            void nap()
            {
                uint64_t one = 1;
                ssize_t n = ::read(mWakeupFD, &one, sizeof one);
                if (n != sizeof(one))
                {
                    ::perror("TODO EventLoop::nap()");
                }
            }

            int createEventfd()
            {
                int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
                if (evtfd < 0)
                {
                    ::perror("EventLoop::createEventfd()");
                }
                return evtfd;
            }

        private:
            int mWakeupFD;

            bool mRunning = false;

            std::shared_ptr<EventHandler> mWakeupHandler;
            std::shared_ptr<Poller> mPoller;

            bool mIsDoingEvent = false;

            const int mPollTimeoutMS;
            std::vector<std::shared_ptr<EventHandler>> mEventHandlers;
        };
    }
}

#endif