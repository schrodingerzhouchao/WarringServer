#ifndef __UTILITY_LOGGER_LINE_MESSAGE_H__
#define __UTILITY_LOGGER_LINE_MESSAGE_H__

#include <string_view>
#include <vector>
#include "Logger.h"


namespace utility
{
    namespace logger
    {
        enum class Level
        {
            FATAL,
            ERROR,
            WARN,
            INFO,
            DEBUG
        };
        const std::vector<std::string_view> LEVELNAME =
            {"[FATAL]", "[ERROR]", "[WARN ]", "[INFO ]", "[DEBUG]"};

        enum class TAG
        {
            CONSOLE,
            TODO_MORE // net?
        };

        class LoggerLineMessage
        {
        public:
            LoggerLineMessage(Level level, char const *file, char const *function, unsigned int line)
                : mLevel{level}, mFileName{file}, mFuncName{function}, mLine{line}
            {
                mTID = std::this_thread::get_id();
                setLevel(mLevel);
            }

            ~LoggerLineMessage()
            {
                push();
            }

            static void LINK(const std::shared_ptr<Logger> &l)
            {
                mLoggerWeak = l;
            }

            LoggerLineMessage &linkLogger(const std::shared_ptr<Logger> &l)
            {
                mLoggerWeak = l;
                
                return *this;
            }

            LoggerLineMessage &operator<<(char arg);
            LoggerLineMessage &operator<<(int arg);
            LoggerLineMessage &operator<<(uint arg);
            LoggerLineMessage &operator<<(int64_t arg);
            LoggerLineMessage &operator<<(uint64_t arg);
            LoggerLineMessage &operator<<(double arg);
            LoggerLineMessage &operator<<(std::string_view arg);
            LoggerLineMessage &operator<<(std::thread::id arg);
            LoggerLineMessage &operator<<(TAG tag);
            LoggerLineMessage &operator<<(Level level);

           

        private:
            void push();
            size_t enum2uint(Level level);
            void preoccuipedStr(Level level);
            void setLevel(Level level) { preoccuipedStr(level); }

        private:
            std::ostringstream mOss;
            std::thread::id mTID;
            Time mTimestamp;

            // for show on the terminal
            std::mutex mCoutMutex;
            bool mCout = false;
            Level mLevel;
            std::string mFileName;
            std::string mFuncName;
            unsigned int mLine;
            
            static std::weak_ptr<Logger> mLoggerWeak;

        };

    }
}

//static std::shared_ptr<utility::logger::Logger> LOGGERPTR;



#define LOG_COUT utility::logger::TAG::CONSOLE

//#define LOG(LEVEL) utility::logger::LoggerLineMessage(LEVEL, __FILE__, __func__, __LINE__).linkLogger(LOGGERPTR)
#define LOG(LEVEL) utility::logger::LoggerLineMessage(LEVEL, __FILE__, __func__, __LINE__)

#define LOG_DEBUG (LOG(utility::logger::Level::DEBUG))
#define LOG_INFO (LOG(utility::logger::Level::INFO))
#define LOG_WARN (LOG(utility::logger::Level::WARN))
#define LOG_ERROR (LOG(utility::logger::Level::ERROR))
#define LOG_FATAL (LOG(utility::logger::Level::FATAL))

#endif