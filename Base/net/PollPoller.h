#ifndef __NET_POLL_POLLER_H__
#define __NET_POLL_POLLER_H__

#include "Poller.h"

#include <vector>
#include <memory>

namespace net
{
    namespace event
    {
        class PollPoller : public Poller
        {
        public:
            PollPoller()
            {
            }
            virtual ~PollPoller() override
            {
            }

            size_t poll(int timeoutMs, std::vector<std::shared_ptr<EventHandler>> &eventHandlers) override
            {
#if 0
                auto blockStartPoint = utility::Time::millisecondsSinceEpoch();
#endif
                auto readyNum = ::poll(&*mPollfds.begin(), mPollfds.size(), timeoutMs);

                auto startPoint = utility::Time::millisecondsSinceEpoch();

                if (readyNum > 0)
                {
                    for (auto it = mPollfds.begin(); it != mPollfds.end() && readyNum > 0; ++it)
                    {
                        --readyNum;
                        auto evHandlerIt = mEventHandlerHash.find(it->fd);
                        if (evHandlerIt != mEventHandlerHash.end())
                        {
                            auto evHandlerPtr = evHandlerIt->second;
                            evHandlerPtr->setRevents(it->revents);
                            eventHandlers.push_back(std::move(evHandlerPtr));
                        }
                    }
                    mTouchedEventNum += readyNum;
                }
                else if (readyNum == 0)
                {
                    // std::cout << "TODO nothing happened\n";
                }
                else
                {
                    ::perror("TODO PollPoller::poll()");
                }

                auto dur = utility::Time::millisecondsSinceEpoch() - startPoint;
                mAccumulatedMillisec += dur;

                if (mTouchedEventNum >= mSettlement)
                {
                    mRate = static_cast<double>(mAccumulatedMillisec) / mTouchedEventNum;
                    mTouchedEventNum = 0;
                    mAccumulatedMillisec = 0;
                }
#if 0
                auto blockEndPoint = utility::Time::millisecondsSinceEpoch();

                mBlockedForIntervalInMilliseconds = mBlockedForTimeoutInMilliseconds += (blockEndPoint - blockStartPoint);

                if ((mBlockedForTimeoutInMilliseconds / 1000) >= mRestThresholdInseconds)
                {
                    mBlockedForTimeoutInMilliseconds = 0;
                }
                if ((mBlockedForIntervalInMilliseconds / 1000) >= mRestThresholdInseconds)
                {
                    mBlockedForIntervalInMilliseconds = 0;
                }
#endif
                return dur;
            }

            void edgeTrigger(std::shared_ptr<EventHandler> &evHandler, bool on) override
            {
            }

            void update(const std::shared_ptr<EventHandler> &evHandler) override
            {
                auto it = mEventHandlerHash.find(evHandler->getFD());
                if (it == mEventHandlerHash.end())
                    return;

                if (evHandler->getIndex() < 0)
                {
                    pollfd pfd;
                    pfd.fd = evHandler->getFD();
                    pfd.events = evHandler->getEvents();
                    pfd.revents = 0;
                    mPollfds.push_back(pfd);
                    int index = static_cast<int>(mPollfds.size()) - 1;
                    evHandler->setIndex(index);
                    it->second = evHandler;
                }
                else
                {
                    int index = evHandler->getIndex();
                    pollfd &pfd = mPollfds.at(index);
                    pfd.fd = evHandler->getFD();
                    pfd.events = evHandler->getEvents();
                    pfd.revents = 0;
                    if (evHandler->isNoneEvent())
                    {
                        pfd.fd = -evHandler->getFD() - 1;
                    }
                }
            }

            void remove(const std::shared_ptr<EventHandler> &evHandler) override
            {
                auto it = mEventHandlerHash.find(evHandler->getFD());
                if (it == mEventHandlerHash.end())
                    return;

                int index = evHandler->getIndex();
                if (index < 0 || static_cast<size_t>(index) >= mPollfds.size())
                    return;

                // const pollfd &pfd = mPollfds.at(index);
                mEventHandlerHash.erase(it);
                if (static_cast<size_t>(index) == mPollfds.size() - 1)
                {
                    mPollfds.pop_back();
                }
                else
                {
                    int lastFD = mPollfds.back().fd;
                    if (lastFD < 0)
                    {
                        lastFD = -lastFD - 1;
                    }
                    mEventHandlerHash[lastFD]->setIndex(index);
                    std::swap(mPollfds.at(index), mPollfds.back());
                    mPollfds.pop_back();
                }
            }

        private:
            std::vector<pollfd> mPollfds;
        };
    }
}

#endif