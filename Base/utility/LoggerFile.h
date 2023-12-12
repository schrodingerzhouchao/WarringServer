
/*
 *
 *
 */

#ifndef __UTILITY_LOGGER_FILE_H__
#define __UTILITY_LOGGER_FILE_H__

#include "Time.h"

#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <unistd.h>
#include <fstream>
#include <filesystem>

namespace utility
{
    namespace logger
    {
        class LoggerFile
        {
        public:
            LoggerFile() = default;

            ~LoggerFile()
            {
                close();
            }

            std::string threadID(const std::thread::id tid);

            void createDefaultLogPath();
            bool createCustomLogPath(std::string logPath);
            int createLogFile();
            int getFD() const { return mFd; }

            bool available() { return mFd == -1 ? false : true; }
            bool setFileSizeWithHole(size_t sz = 1024 * 1024);
            const size_t getFileSize() const;
            void setFileSizeLimite(size_t sz = 1024 * 1024);
            const size_t getFileSizeLimite() const;
            bool archive(const char *buff, const size_t len, bool sync = false);
            void closeOld() { ::close(mOldFd); }

            void close()
            {
                mRunning = false;
                ::close(mFd);
            }

        private:
            void resetNewFileCurrDataSize() { mCurrDataSize = 0; }
            std::string serialNo() { return std::to_string(mNo); }
            std::string getCurrentPath() const { return std::filesystem::current_path().string(); }
            std::string getCustomPath() const { return mPath.string(); }
            std::string getPID() const { return std::to_string(::getpid()); }
            std::string getPName() const
            {
                std::ifstream comm("/proc/self/comm");
                std::string name;
                getline(comm, name);
                return name;
            }
            std::string generateAbsoluteLogPathName()
            {
                mLogFileName = getPName() + '[' + getPID() + '-' + std::to_string(mNo) + ']' + mTimeStamp.getRough() + POSTFIX;
                return getCustomPath() + mLogFileName;
            }

        private:
            int mFd = -1;
            int mOldFd = -1;
            const std::string POSTFIX = ".log";

            std::string TID;
            /*static*/ size_t mNo = 1;
            Time mTimeStamp;

            std::string mLogFileName;

            size_t mFileSizeLimite = 0;
            size_t mCurrDataSize = 0;

            std::filesystem::path mPath{};

            bool mRunning = true;
        };

    }
}

#endif