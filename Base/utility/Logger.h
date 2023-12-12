
/*
 *
 *
 */

#ifndef __UTILITY_LOGGER_H__
#define __UTILITY_LOGGER_H__

#include "LoggerBuffer.h"
#include "LoggerLine.h"
#include "LoggerFile.h"
#include "Time.h"
#include "Timer.h"

#include "SafeHashmap.h"

#include <thread>
#include <string>
#include <string_view>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <unordered_set>
#include <memory>

// #include <iostream>

namespace utility
{
    namespace logger
    {
        class Logger : public std::enable_shared_from_this<Logger>
        {
        public:
            Logger() : mBackgroundThd(&Logger::work, this)
            {
                mFilledLoggerBufferQueuePtr = std::make_shared<SafeQueue<std::unique_ptr<LoggerBufferNType>>>();
                mEmptyLoggerBufferQueuePtr = std::make_shared<SafeQueue<std::unique_ptr<LoggerBufferNType>>>();

                mLoggerLineHashtablePtr = std::make_shared<SafeHashmap<std::thread::id, std::shared_ptr<LoggerLine>>>();
            }

            ~Logger()
            {
                cancelTimer();
                mCurrentLoggerFile.close();
                join();
            }

            bool customPath(const std::string &logPath);

            // it's better when equal or just greater the number of threads
            void setStandbyQueueMax(size_t num) { mStandbyQueueMax = num; }

            bool init(const std::shared_ptr<utility::timer::TimeWheelTimer> &twTimerPtr,
                      const size_t timerSecs, const std::string &timerID, const size_t fileSize = 1024 * 1024, bool sync = true);

            size_t fileSize() { return mCurrentLoggerFile.getFileSize(); }
            size_t fileLimited() { return mCurrentLoggerFile.getFileSizeLimite(); }
            bool isFull() { return fileSize() >= fileLimited(); }

            void registerLoggerLine(const std::thread::id &tid, const std::shared_ptr<LoggerLine> &llp);

            void findLoggerLine(const std::thread::id &tid, std::shared_ptr<LoggerLine> &llp);

            void removeLoggerLine(const std::thread::id &tid);

            void stop();

            void join()
            {
                //std::cout<<"Logger::join()\n";

                if (mBackgroundThd.joinable())
                    mBackgroundThd.join();
            }

        private:
            int createFile();
            void archive();
            void setFileSizeLimite(size_t sz = 1024 * 1024) { mFileSizeLimite = sz; }

            void timeoutNotify() { mTimeout = true; }
            void resetTimeout() { mTimeout = false; }
            void setTimer(const std::string &id);
            void cancelTimer();
            void timeoutPushToFile();

            void work();

        private:
            LoggerFile mCurrentLoggerFile;
            size_t mFileSizeLimite = 1024 * 1024;
            bool mSync = false;

            bool mRunning = true;

            size_t mTimeoutSeconds = 5;
            bool mTimeout = false;
            std::string mTimerTaskID;
            std::thread mBackgroundThd;
            std::weak_ptr<utility::timer::TimeWheelTimer> mTimeWheelTimerWeak;
            std::shared_ptr<SafeQueue<std::unique_ptr<LoggerBufferNType>>> mFilledLoggerBufferQueuePtr;
            std::shared_ptr<SafeQueue<std::unique_ptr<LoggerBufferNType>>> mEmptyLoggerBufferQueuePtr;

            std::shared_ptr<SafeHashmap<std::thread::id, std::shared_ptr<LoggerLine>>> mLoggerLineHashtablePtr;
            std::unordered_set<std::thread::id> mThreadIDHashset;

            size_t mStandbyQueueMax = 16;
        };

    }

}


#endif