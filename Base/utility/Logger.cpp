#include "Logger.h"
#include "LoggerMessage.h"

namespace utility
{
    namespace logger
    {
        bool Logger::customPath(const std::string &logPath)
        {
            return mCurrentLoggerFile.createCustomLogPath(logPath);
        }

        bool Logger::init(const std::shared_ptr<utility::timer::TimeWheelTimer> &twTimerPtr,
                          const size_t timerSecs, const std::string &timerID, const size_t fileSize, bool sync)
        {
            mTimeWheelTimerWeak = twTimerPtr;
            mTimeoutSeconds = timerSecs;
            setTimer(timerID);

            LoggerLineMessage::LINK(shared_from_this());

            mFileSizeLimite = fileSize < mFileSizeLimite ? mFileSizeLimite : fileSize;
            mSync = sync;

            mCurrentLoggerFile.createDefaultLogPath();

            if (createFile() < 0)
            {
                std::cout << "Logger create log file error\n";
                mRunning = false;

                return false;
            }

            return true;
        }

        int Logger::createFile()
        {
            auto fd = mCurrentLoggerFile.createLogFile();
            if (fd != -1)
            {
                mCurrentLoggerFile.setFileSizeLimite(mFileSizeLimite);
            }

            return fd;
        }

        void Logger::setTimer(const std::string &id)
        {
            utility::timer::TimerTask tt(0, 0, mTimeoutSeconds, -1, id, std::bind(&Logger::timeoutNotify, this));

            auto timerPtr = mTimeWheelTimerWeak.lock();
            if (timerPtr)
                timerPtr->addTask(tt);
            else
                std::cout << "Logger get timer ptr error\n";

            mTimerTaskID = id;
        }

        void Logger::archive()
        {
            auto unique = std::make_unique<LoggerBufferNType>();
            // mFilledLoggerBufferQueuePtr->waitAndPop(std::move(unique));
            mFilledLoggerBufferQueuePtr->waitAndPop(std::move(unique), 1000);
            auto res = mCurrentLoggerFile.archive(unique->get(), unique->getSize(), mSync);
            if (!res)
            {
                auto oldFd = mCurrentLoggerFile.getFD();
                
                if (createFile() == -1)
                {
                    std::cout << "Logger create custom log file error\n";
                    return;
                }
                mCurrentLoggerFile.archive(unique->get(), unique->getSize(), mSync);

                //::close(oldFd);
                mCurrentLoggerFile.closeOld();
            }
            unique->reset();

            if (mEmptyLoggerBufferQueuePtr->size() < mStandbyQueueMax)
            {
                mEmptyLoggerBufferQueuePtr->push(std::move(unique));
            }
        }

        void Logger::registerLoggerLine(const std::thread::id &tid, const std::shared_ptr<LoggerLine> &llp)
        {
            mLoggerLineHashtablePtr->insertOrAssign(tid, llp);
            llp->getBufferPtrQueue(mFilledLoggerBufferQueuePtr, mEmptyLoggerBufferQueuePtr);
            mThreadIDHashset.emplace(tid);
        }

        void Logger::findLoggerLine(const std::thread::id &tid, std::shared_ptr<LoggerLine> &llp)
        {
            if (!mLoggerLineHashtablePtr->find(tid, llp))
            {
                llp = std::make_shared<LoggerLine>();
                mLoggerLineHashtablePtr->insert(tid, llp);
                mThreadIDHashset.emplace(tid);
            }
            llp->getBufferPtrQueue(mFilledLoggerBufferQueuePtr, mEmptyLoggerBufferQueuePtr);
        }

        void Logger::removeLoggerLine(const std::thread::id &tid)
        {
            mLoggerLineHashtablePtr->erase(tid);
            mThreadIDHashset.erase(tid);
        }

        void Logger::cancelTimer()
        {
            if (!mTimerTaskID.empty())
            {
                utility::timer::TimerTask tt;
                tt.mID = mTimerTaskID;
                tt.mCycleNumber = 0;
                auto timerPtr = mTimeWheelTimerWeak.lock();
                if (timerPtr)
                    timerPtr->addTask(tt);
            }
        }

        void Logger::timeoutPushToFile()
        {
            if (mTimeout)
            {
                for (auto &tid : mThreadIDHashset)
                {
                    std::shared_ptr<LoggerLine> llp;
                    if (mLoggerLineHashtablePtr->find(tid, llp))
                    {
                        llp->pushToQueue();
                    }
                }
                resetTimeout();
            }
        }

        void Logger::stop()
        {
            cancelTimer();
            mRunning = false;
            mCurrentLoggerFile.close();
        }

        void Logger::work()
        {
            while (mRunning)
            {
                timeoutPushToFile();
                archive();
#if 0
                std::cout << "mFilledLoggerBufferQueuePtr size " << mFilledLoggerBufferQueuePtr->size()
                          << " | mEmptyLoggerBufferQueuePtr size " << mEmptyLoggerBufferQueuePtr->size() << std::endl;
#endif
            }
        }

    }
}
