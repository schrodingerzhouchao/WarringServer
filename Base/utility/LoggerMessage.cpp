#include "LoggerMessage.h"

namespace utility
{
    namespace logger
    {
        LoggerLineMessage &LoggerLineMessage::operator<<(char arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(int arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(uint arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(int64_t arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(uint64_t arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(double arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(std::string_view arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(std::thread::id arg)
        {
            mOss << arg;
            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(TAG tag)
        {
            if (tag == TAG::CONSOLE)
            {
                mCout = true;
            }

            return *this;
        }
        LoggerLineMessage &LoggerLineMessage::operator<<(Level level)
        {
            setLevel(level);
            return *this;
        }

        size_t LoggerLineMessage::enum2uint(Level level)
        {
            return static_cast<size_t>(level);
        }

        void LoggerLineMessage::preoccuipedStr(Level level)
        {
            auto index = enum2uint(level);

            if (index < LEVELNAME.size())
            {
                if (index <= 2)
                    mCout = true;
                mOss << LEVELNAME.at(index)
                     << "[" << mTID << "]"
                     << "[" << mTimestamp() << "]"
                     << "[" << mFileName << ":" << mFuncName << ":" << mLine << "]";
            }
            else
            {
                mOss << "[UNDEF]"
                     << "[" << mTID << "]"
                     << "[" << mTimestamp() << "]"
                     << "[" << mFileName << ":" << mFuncName << ":" << mLine << "]";
            }
        }

        void LoggerLineMessage::push()
        {
            mOss << std::endl;
            auto lptr = mLoggerWeak.lock();
            if (lptr)
            {
                std::shared_ptr<LoggerLine> llptr;
                lptr->findLoggerLine(mTID, llptr);
                llptr->pushToBuffer(mOss.str());
            }
            else
            {
                std::cout << "not link\n";
                mCout = true;
            }
            if (mCout)
            {
                std::lock_guard<std::mutex> lk(mCoutMutex);
                std::cout << mOss.str();
            }
        }

        std::weak_ptr<Logger> LoggerLineMessage::mLoggerWeak;
    }
}