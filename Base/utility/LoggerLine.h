
/*
 *
 *
 */

#ifndef __UTILITY_LOGGER_LINE_H__
#define __UTILITY_LOGGER_LINE_H__

#include "LoggerBuffer.h"
#include "Timer.h"
#include <string_view>

namespace utility
{
    namespace logger
    {
        class LoggerLine
        {
        public:
            LoggerLine()
            {
                mLoggerBufferPtrA = std::make_unique<LoggerBufferNType>();
                mLoggerBufferPtrB = std::make_unique<LoggerBufferNType>();
            }
            LoggerLine(const LoggerLine &) = delete;
            LoggerLine &operator=(const LoggerLine &) = delete;
            LoggerLine(LoggerLine &&) = delete;
            LoggerLine &operator=(LoggerLine &&) = delete;

            ~LoggerLine()
            {
                auto p = mFilledLoggerBufferQueueWeak.lock();
                if (p)
                {
                    if (mLoggerBufferPtrA && (!mLoggerBufferPtrA->empty()))
                    {
                        p->push(std::move(mLoggerBufferPtrA));
                    }
                    if (mLoggerBufferPtrB && (!mLoggerBufferPtrB->empty()))
                    {
                        p->push(std::move(mLoggerBufferPtrB));
                    }
                }
                //  std::cout << "~LoggerLine()\n";
            }

            void getBufferPtrQueue(const std::shared_ptr<SafeQueue<std::unique_ptr<LoggerBufferNType>>> &filledLoggerBufferQueuePtr,
                                   const std::shared_ptr<SafeQueue<std::unique_ptr<LoggerBufferNType>>> &emptyLoggerBufferQueuePtr)
            {
                mFilledLoggerBufferQueueWeak = filledLoggerBufferQueuePtr;
                mEmptyLoggerBufferQueueWeak = emptyLoggerBufferQueuePtr;
            }

            size_t TESTgetBufferASize() { return mLoggerBufferPtrA->getSize(); }

            void pushToBuffer(const std::string &msg);
            void pushToQueue();

        private:
            void getEmptyBuffer();

        private:
            // work-LoggerBuffer and standby-LoggerBuffer
            std::unique_ptr<LoggerBufferNType> mLoggerBufferPtrA;
            std::unique_ptr<LoggerBufferNType> mLoggerBufferPtrB;

            std::weak_ptr<SafeQueue<std::unique_ptr<LoggerBufferNType>>> mFilledLoggerBufferQueueWeak;
            std::weak_ptr<SafeQueue<std::unique_ptr<LoggerBufferNType>>> mEmptyLoggerBufferQueueWeak;
        };

    }
}

#endif