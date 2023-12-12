#include "LoggerFile.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

namespace utility
{
    namespace logger
    {
        std::string LoggerFile::threadID(const std::thread::id tid)
        {
            size_t intid = *(size_t *)&tid;
            TID = std::to_string(intid);
            return TID;
        }

        void LoggerFile::createDefaultLogPath()
        {
            if (mPath.empty())
            {
                createCustomLogPath(getCurrentPath() + "/log/");
            }
        }

        bool LoggerFile::createCustomLogPath(std::string logPath)
        {
            if (logPath.back() != '/')
            {
                logPath += '/';
            }

            mPath = logPath;
            if (std::filesystem::create_directories(mPath))
            {
                return true;
            }
            else if (std::filesystem::exists(mPath))
            {
                return true;
            }
            else
            {
                mPath = std::filesystem::current_path();

                return false;
            }
        }

        int LoggerFile::createLogFile()
        {
            const std::string logPathName = generateAbsoluteLogPathName();
            mOldFd = mFd;
            mFd = ::open(logPathName.c_str(), O_RDWR | O_CREAT | O_APPEND,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (mFd != -1)
            {
                mCurrDataSize = 0;
                ++mNo;
            }
            return mFd;
        }

        bool LoggerFile::setFileSizeWithHole(size_t sz)
        {
            auto res = ::ftruncate(mFd, sz);
            if (res == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        const size_t LoggerFile::getFileSize() const
        {
            struct stat sb;
            auto res = fstat(mFd, &sb);
            if (res == -1)
                return -1;
            return sb.st_size;
        }
        void LoggerFile::setFileSizeLimite(size_t sz)
        {
            mFileSizeLimite = sz;
        }
        const size_t LoggerFile::getFileSizeLimite() const
        {
            return mFileSizeLimite;
        }

        bool LoggerFile::archive(const char *buff, const size_t len, bool sync)
        {
            //std::cout << "LoggerFile::archive write to fd: " << mFd << std::endl;

            if (mCurrDataSize + len > mFileSizeLimite)
                return false;
            // std::cout << "intend to  write " << len << " bytes\n";

            size_t curr = 0;
            while (mRunning)
            {
                size_t remain = len - curr;

                auto sz = ::write(mFd, buff, remain);
                if (sz == -1)
                {
                    // std::cout << "write error\n";
                    ::perror("write");
                    break;
                }
                if (sz == 0)
                {
                    // std::cout << "write down\n";
                    break;
                }
                // std::cout << "actually wrote " << sz << " bytes\n";
                curr += sz;
                mCurrDataSize += sz;
                if (sync)
                    ::fdatasync(mFd);
            }

            return true;
        }

        // size_t LoggerFile::mNo = 1;
    }
}