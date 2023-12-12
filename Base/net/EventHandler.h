#ifndef __NET_EVENT_HANDLER_H__
#define __NET_EVENT_HANDLER_H__

#include <poll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>

namespace net
{
    namespace event
    {
        const int NONE_EVENT = 0;
        const int READ_EVENT = POLLIN | POLLPRI;
        const int WRITE_EVENT = POLLOUT;

        using EventFuncType = std::function<void()>;

        class EventHandler
        {

        public:
            EventHandler(int fd) : mFD{fd}, mEvents{NONE_EVENT}
            {
            }
            ~EventHandler()
            {
            }

            void doEvnet()
            {
                if ((mRevents & POLLHUP) && !(mRevents & POLLIN))
                {
                    if (mCloseFunc)
                        mCloseFunc();
                }
                if (mRevents & (POLLNVAL | POLLERR))
                {
                    if (mErrorFunc)
                        mErrorFunc();
                }
                if (mRevents & (POLLIN | POLLPRI | POLLRDHUP))
                {
                    if (mReadFunc)
                        mReadFunc();
                }
                if (mRevents & POLLOUT)
                {
                    if (mWriteFunc)
                        mWriteFunc();
                }
            }

            void setIndex(int index) { mIndex = index; }
            int getIndex() const { return mIndex; }

            const int getFD() const { return mFD; }

            void setReadFunc(EventFuncType func) { mReadFunc = std::move(func); }
            void setWriteFunc(EventFuncType func) { mWriteFunc = std::move(func); }
            void setCloseFunc(EventFuncType func) { mCloseFunc = std::move(func); }
            void setErrorFunc(EventFuncType func) { mErrorFunc = std::move(func); }

            void enableReading() { mEvents |= READ_EVENT; }
            void disableReading() { mEvents &= ~READ_EVENT; }
            void enableWriting() { mEvents |= WRITE_EVENT; }
            void disableWriting() { mEvents &= ~WRITE_EVENT; }
            void disableAll() { mEvents = NONE_EVENT; }
            bool isReadable() const { return mEvents & READ_EVENT; }
            bool isWriteable() const { return mEvents & WRITE_EVENT; }
            bool isNoneEvent() const { return mEvents == NONE_EVENT; }

            void setEvents(int events) { mEvents = events; }
            void setRevents(int revents) { mRevents = revents; }
            unsigned int getEvents() const { return mEvents; }
            unsigned int getRevents() const { return mRevents; }

        private:
            const int mFD;
            unsigned int mEvents;
            unsigned int mRevents;
            int mIndex = -1;

            // bool mIsValid = false;

            EventFuncType mReadFunc;
            EventFuncType mWriteFunc;
            EventFuncType mCloseFunc;
            EventFuncType mErrorFunc;

            // std::weak_ptr<EventLoop> mEventLoop;
        };
    }
}

#endif