#include "LoggerLine.h"
#include <iostream>

namespace utility
{
    namespace logger
    {
        void LoggerLine::pushToQueue()
        {
            getEmptyBuffer();
            mLoggerBufferPtrA.swap(mLoggerBufferPtrB);
            auto p = mFilledLoggerBufferQueueWeak.lock();
            if (p)
                p->push(std::move(mLoggerBufferPtrB));
        }

        void LoggerLine::pushToBuffer(const std::string &msg)
        {
            if (!mLoggerBufferPtrA->push(msg))
            {
                getEmptyBuffer();
                mLoggerBufferPtrA.swap(mLoggerBufferPtrB);
                mLoggerBufferPtrA->push(msg);
                auto p = mFilledLoggerBufferQueueWeak.lock();
                if (p)
                    p->push(std::move(mLoggerBufferPtrB));
            }

            // std::cout << mLoggerBufferPtrA->getSize() << std::endl;
        }

        void LoggerLine::getEmptyBuffer()
        {
            if (mLoggerBufferPtrB)
                return;

            auto p = mEmptyLoggerBufferQueueWeak.lock();
            if (!p)
            {
                std::cout << "get mEmptyLoggerBufferQueueWeak error\n";
            }
            else
            {
                if (!p->pop(mLoggerBufferPtrB))
                {
                    // std::cout << "EmptyLoggerBufferQueue is empty, make a new\n";
                    mLoggerBufferPtrB = std::make_unique<LoggerBufferNType>();
                }
            }
        }
    }
}