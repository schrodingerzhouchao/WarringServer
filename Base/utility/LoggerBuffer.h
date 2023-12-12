
/*
 *
 *
 */

#ifndef __UTILITY_LOGGER_BUFFER_H__
#define __UTILITY_LOGGER_BUFFER_H__

#include <string>
#include <cstring>
#include <iostream>

namespace utility
{
    namespace logger
    {
        const size_t SMALLBUFFERSIZE = 4096;
        const size_t LARGEBUFFERSIZE = 4096 * 10;

        constexpr size_t SUITABLEBUFFSIZE(size_t N = 4096)
        {
            if (N >= LARGEBUFFERSIZE)
                return LARGEBUFFERSIZE;
            else if (N <= SMALLBUFFERSIZE)
                return SMALLBUFFERSIZE;
            return N;
        }

        template <size_t N>
        class LoggerBuffer
        {
        public:
            constexpr LoggerBuffer()
                : mCapacity{SUITABLEBUFFSIZE(N)}
            {
                bzero();
                mCurrent = mBuffer;
                mCursor = mBuffer;
            }
            LoggerBuffer(const LoggerBuffer &) = delete;
            LoggerBuffer &operator=(const LoggerBuffer &) = delete;
            LoggerBuffer(LoggerBuffer &&) = delete;
            LoggerBuffer &operator=(LoggerBuffer &&) = delete;

            ~LoggerBuffer()
            {
                bzero();
                mCurrent = nullptr;
                mCursor = nullptr;
            }

            void setThreadID(const std::string id) { mThreadID = id; }
            std::string getThreadID() const { return mThreadID; }
            bool empty() const { return mCurrent == mBuffer; }
            size_t getCapacity() const { return mCapacity; }
            size_t getSize() const { return static_cast<size_t>(mCurrent - mBuffer); }

            void reset()
            {
                bzero();
                mCurrent = mBuffer;
            }

            void setCursor(size_t pos = 0)
            {
                if (pos > mCapacity)
                    mCursor = end();
                else
                    mCursor = mBuffer + pos;
            }

            bool push(std::string_view msg)
            {
                auto len = msg.size();
                if (getAvail() > len)
                {
                    ::memcpy(mCurrent, msg.data(), len);
                    mCurrent += len;
                    return true;
                }

                return false;
            }

            std::string popLine()
            {
                if (mCursor == end())
                    return std::string();
                std::string_view sv(mCursor);
                std::string res;
                auto pos = sv.find('\n');
                if (pos == std::string_view::npos)
                {
                    res = std::string(mCursor, getAvail()) + "\n";
                    reset();
                    return res;
                }
                res = std::string(mCursor, pos + 1);
                memset(mCursor, 0, pos + 1);
                mCursor += (pos + 1);
                return res;
            }
            int popAll(int fd)
            {
                // TODO
                return 0;
            }

            char *get()
            {
                return mBuffer;
            }

            void testPtint()
            {
                std::cout << std::string(mBuffer) << std::endl;
            }

        private:
            int getAvail() { return static_cast<int>(end() - mCurrent); }
            void bzero() { ::memset(mBuffer, 0, mCapacity); }
            char *end() { return mBuffer + mCapacity; }

        private:
            std::string mThreadID;
            const size_t mCapacity;

            char mBuffer[SUITABLEBUFFSIZE(N)];
            char *mCurrent;
            char *mCursor;
        };
    
    }
}

using LoggerBufferNType = utility::logger::LoggerBuffer<40960>;

#endif