#ifndef __UTILITY_TIME_H__
#define __UTILITY_TIME_H__

#include <string>
#include <utility>
#include <chrono>
#include <atomic>
#include <iomanip>

namespace utility
{
    class Time
    {
    public:
        Time()
        {
            mRoughDate = getRough();
            mSecondsSinceEpoch = secondsSinceEpoch();
        }
        ~Time() = default;

        // get now (YYYY-MM-DD-HH:MM:SS.MICROS)
        std::string operator()()
        {
            auto microseconds = microsecondsSinceEpoch() % 1000000;
            auto rough = getRough();

            std::ostringstream outStr;
            outStr << rough << "." << std::setw(6) << std::setfill('0') << std::to_string(microseconds);

            return outStr.str();
        }

        // get now (YYYY-MM-DD-HH:MM:SS)
        std::string getRough()
        {
            auto timePoint = std::chrono::system_clock::now();
            time_t tt = std::chrono::system_clock::to_time_t(timePoint);
            tm *t = localtime(&tt);
            char buffer[32] = {0};
            strftime(buffer, sizeof(buffer), "%F-%T", t);
            return std::string(buffer);
        }

        // get now lazily if @lazy is true (YYYY-MM-DD-HH:MM:SS.MICROS)
        // reuse the rough part if in the same second
        std::string getNow(bool lazy = true)
        {
            auto microseconds = microsecondsSinceEpoch() % 1000000;
            std::ostringstream outStr;

            if (lazy && (mSecondsSinceEpoch == secondsSinceEpoch()))
            {
                outStr << mRoughDate << "." << std::setw(6) << std::setfill('0') << std::to_string(microseconds);
            }
            else
            {
                outStr << getRough() << "." << std::setw(6) << std::setfill('0') << std::to_string(microseconds);
            }

            return outStr.str();
        }

        static time_t secondsSinceEpoch()
        {
            std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch());
            return s.count();
        }

        static time_t millisecondsSinceEpoch()
        {
            std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
            return millis.count();
        }

        static time_t microsecondsSinceEpoch()
        {
            std::chrono::microseconds micros = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch());
            return micros.count();
        }

    private:
        std::string mRoughDate{"YYYY-MM-DD-HH:MM:SS"};
        std::atomic<time_t> mSecondsSinceEpoch{0};
    };

    template <typename T>
    class Chronometer
    {
    public:
        Chronometer()
        {
            mTimepoint = std::chrono::steady_clock::now();
        }

        time_t getElapsed() const
        {
            auto dur = std::chrono::steady_clock::now() - mTimepoint;
            return std::chrono::duration_cast<T>(dur).count();
        }

    private:
        std::chrono::steady_clock::time_point mTimepoint;
    };
}

#endif