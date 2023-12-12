#ifndef __NET_POLLER_H__
#define __NET_POLLER_H__

#include "EventHandler.h"
#include "../utility/Time.h"

#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>

#include <iostream>

namespace net
{
    namespace event
    {
        class Poller
        {
        public:
            Poller()
            {
            }
            virtual ~Poller()
            {
            }

            void registerEventHandler(const std::shared_ptr<EventHandler> &evHandler)
            {
                mEventHandlerHash.emplace(evHandler->getFD(), evHandler);
            }

            void registerEventHandler(int fd)
            {
                auto evHandlerPtr = std::make_shared<EventHandler>(fd);
                mEventHandlerHash.emplace(fd, std::move(evHandlerPtr));
            }

            virtual size_t poll(int timeoutMs, std::vector<std::shared_ptr<EventHandler>> &eventHandlers) = 0;
            virtual void update(const std::shared_ptr<EventHandler> &evHandler) = 0;
            virtual void remove(const std::shared_ptr<EventHandler> &evHandler) = 0;
            virtual void edgeTrigger(std::shared_ptr<EventHandler> &evHandler, bool on) = 0;

            size_t eventSize() const { return mEventHandlerHash.size(); }

            void setSettlementPoint(size_t settlementPoint) { mSettlement = (settlementPoint < 100 ? 100 : settlementPoint); }

            // params should satisfy the conditions: intervalSeconds << timeoutSeconds << resetThresholdSeconds
            // note: no checks in the implementation
            void setTimeoutParam(time_t intervalSeconds, time_t timeoutSeconds, time_t resetThresholdSeconds)
            {
                mDetectionIntervalInMilliseconds = intervalSeconds * 1000;
                mTimeoutInMilliseconds = timeoutSeconds * 1000;
                mRestThresholdInseconds = resetThresholdSeconds;
            }
#if 0
            // if true, also recount 
            bool isIntervalOut()
            {
                if (mBlockedForIntervalInMilliseconds >= mDetectionIntervalInMilliseconds)
                {
                    mBlockedForIntervalInMilliseconds = 0;
                    return true;
                }
                return false;
            }
            // if true, also recount 
            bool isTimeout()
            {
                if (mBlockedForTimeoutInMilliseconds >= mTimeoutInMilliseconds)
                {
                    mBlockedForTimeoutInMilliseconds = 0;
                    return true;
                }
                return false;
            }
#endif
            double getLastRate() const { return mRate; }
#if 0
            void testInfo()
            {
                std::cout << "event handler hash size = " << mEventHandlerHash.size() << std::endl;
            }
#endif
        protected:
            size_t mSettlement = 1000;
            size_t mAccumulatedMillisec = 0;
            size_t mTouchedEventNum = 0;
            double mRate = 0.0;

            std::atomic_long mBlockedForIntervalInMilliseconds = 0;
            std::atomic_long mBlockedForTimeoutInMilliseconds = 0;
            time_t mDetectionIntervalInMilliseconds = 30 * 1000;
            time_t mTimeoutInMilliseconds = 300 * 1000;
            time_t mRestThresholdInseconds = 3600;

            std::unordered_map<int, std::shared_ptr<EventHandler>> mEventHandlerHash;
        };
    }
}

#endif