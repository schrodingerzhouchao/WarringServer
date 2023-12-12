#ifndef __UTILITY_WIDGETS_H__
#define __UTILITY_WIDGETS_H__

#ifndef NAMEOF
#define NAMEOF(x) (x, #x)
#endif

#include <string>
#include <thread>
#include "Time.h"

namespace utility
{
    class Widgets
    {
    public:
        template <typename T>
        static std::string getTypeName(const T &t)
        {
            return typeid(t).name();
        }

        static std::string getTID()
        {
            auto tid = std::this_thread::get_id();
            size_t intid = *(size_t *)&tid;
            return std::to_string(intid);
        }

        static std::string getTimestamp()
        {
            return std::to_string(Time::microsecondsSinceEpoch());
        }

    private:
    };
}

#endif