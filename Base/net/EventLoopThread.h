#ifndef __NET_EVENT_LOOP_THREAD_H__
#define __NET_EVENT_LOOP_THREAD_H__

#include "EventLoop.h"

#include <thread>

namespace net
{
    namespace event
    {
        class EventLoopThread
        {
        public:
            EventLoopThread(PollerType which)
                : mEventloop(std::make_shared<EventLoop>(which)),
                  mThread(&EventLoopThread::work, this)
            {
                mID = mThread.get_id();
            }

            EventLoopThread(const EventLoopThread &) = delete;
            EventLoopThread &operator=(const EventLoopThread &) = delete;
            EventLoopThread(EventLoopThread &&) = delete;
            EventLoopThread &operator=(EventLoopThread &&) = delete;

            ~EventLoopThread()
            {
                mEventloop->quit();
                if (mThread.joinable())
                    mThread.join();
            }

            void addEvent(const std::shared_ptr<EventHandler> &evHandler)
            {
                mEventloop->registerEventHandler(evHandler);
                mEventloop->update(evHandler);
                mEventloop->wakeup();
            }

            void removeEvent(const std::shared_ptr<EventHandler> &evHandler)
            {
                mEventloop->remove(evHandler);
                mEventloop->wakeup();
            }

            std::shared_ptr<EventLoop> getEventLoop() const { return mEventloop; }
            size_t getEventSize() const { return mEventloop->getEventSize(); }
            double getLastRate() const { return mEventloop->getLastRate(); }
            bool isLooping() const { return mEventloop->isRunning(); }

            std::thread::id getThreadID() const { return mID; }

            void injector(std::function<void()> func)
            {
                if (func)
                {
                    func();
                }
            }

        private:
            void work()
            {
                mEventloop->loop();
            }

        private:
            std::shared_ptr<EventLoop> mEventloop;
            std::thread mThread;
            std::thread::id mID;
        };
    }
}

#endif