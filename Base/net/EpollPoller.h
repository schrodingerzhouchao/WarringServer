#ifndef __NET_EPOLL_POLLER_H__
#define __NET_EPOLL_POLLER_H__

#include "Poller.h"

#include <unistd.h>
#include <vector>
#include <memory>
#include <cstring>

namespace net
{
    namespace event
    {
        class EpollPoller : public Poller
        {
        public:
            EpollPoller()
                : mFD(::epoll_create1(EPOLL_CLOEXEC))
            {
                if (mFD < 0)
                {
                    ::perror("TODO EpollPoller::EpollPoller()");
                }

                // std::cout << "epollfd = " << mFD << std::endl;
                mEpollEvents.resize(16);
            }
            virtual ~EpollPoller() override
            {
                ::close(mFD);
            }

            size_t poll(int timeoutMs, std::vector<std::shared_ptr<EventHandler>> &eventHandlers) override
            {
#if 0
                auto pollStartPoint = utility::Time::millisecondsSinceEpoch();
#endif

                auto readyNum = ::epoll_wait(mFD, &*mEpollEvents.begin(), static_cast<int>(mEpollEvents.size()), timeoutMs);

                auto startPoint = utility::Time::millisecondsSinceEpoch();

                if (readyNum > 0)
                {
                    for (int i = 0; i < readyNum; ++i)
                    {
                        auto evHandlerIt = mEventHandlerHash.find(mEpollEvents.at(i).data.fd);
                        if (evHandlerIt != mEventHandlerHash.end())
                        {
                            auto evHandlerPtr = evHandlerIt->second;
                            evHandlerPtr->setRevents(mEpollEvents.at(i).events);
                            eventHandlers.push_back(std::move(evHandlerPtr));
                        }
                    }
                    if (static_cast<size_t>(readyNum) == mEpollEvents.size())
                    {
                        mEpollEvents.resize(readyNum * 2 + 1);
                    }

                    mTouchedEventNum += readyNum;
                }
                else if (readyNum == 0)
                {
                    // std::cout << "TODO nothing happened\n";
                }
                else
                {
                    ::perror("TODO EPollPoller::poll()");
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
                auto pollEndPoint = utility::Time::millisecondsSinceEpoch();

                mBlockedForIntervalInMilliseconds = mBlockedForTimeoutInMilliseconds += (pollEndPoint - pollStartPoint);

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
                int fd = evHandler->getFD();
                auto it = mEventHandlerHash.find(fd);
                if (it == mEventHandlerHash.end())
                    return;

                unsigned int events = on ? (evHandler->getEvents()) | EPOLLET : (evHandler->getEvents()) & (~EPOLLET);
                evHandler->setEvents(events);
            }

            // TODO
            void oneshot(int fd, bool on)
            {
            }

            void update(const std::shared_ptr<EventHandler> &evHandler) override
            {
                int fd = evHandler->getFD();
                auto it = mEventHandlerHash.find(fd);
                if (it == mEventHandlerHash.end())
                    return;

                epoll_event eevent;
                ::memset(&eevent, 0, sizeof(eevent));
                eevent.data.fd = fd;
                eevent.events = evHandler->getEvents();
                if (evHandler->getIndex() < 0)
                {
                    if (::epoll_ctl(mFD, EPOLL_CTL_ADD, evHandler->getFD(), &eevent) < 0)
                    {
                        ::perror("TODO EpollPoller::update() EPOLL_CTL_ADD");
                    }
                    else
                    {
                        it->second = evHandler;
                        it->second->setIndex(1);
                    }
                }
                else
                {
                    if (evHandler->isNoneEvent())
                    {

                        if (::epoll_ctl(mFD, EPOLL_CTL_DEL, evHandler->getFD(), &eevent) < 0)
                        {
                            ::perror("TODO EpollPoller::update() EPOLL_CTL_DEL");
                        }
                        else
                        {
                            it->second->setIndex(-1);
                        }
                    }
                    else
                    {
                        if (::epoll_ctl(mFD, EPOLL_CTL_MOD, evHandler->getFD(), &eevent) < 0)
                        {
                            ::perror("TODO EpollPoller::update()");
                        }
                        else
                        {
                            it->second = evHandler;
                        }
                    }
                }
            }

            void remove(const std::shared_ptr<EventHandler> &evHandler) override
            {
                int fd = evHandler->getFD();
                auto it = mEventHandlerHash.find(fd);
                if (it == mEventHandlerHash.end())
                    return;

                epoll_event eevent;
                ::memset(&eevent, 0, sizeof(eevent));
                eevent.data.fd = fd;
                if (::epoll_ctl(mFD, EPOLL_CTL_DEL, evHandler->getFD(), &eevent) < 0)
                {
                    ::perror("TODO EpollPoller::update()");
                }
                else
                {
                    mEventHandlerHash.erase(it);
                    evHandler->setIndex(-1);
                }
            }

        private:
            int mFD = -1;

            std::vector<epoll_event> mEpollEvents;
        };
    }
}

#endif