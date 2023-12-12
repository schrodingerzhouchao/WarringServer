#include "utility/Logger.h"
#include "utility/LoggerMessage.h"
#include "utility/Timer.h"

#include <random>

using namespace utility::timer;
using namespace utility::logger;

void func(const size_t index, size_t testInt, std::string testMessage)
{
    std::random_device mSeeder;

    for (size_t i = 0; i < 5000; ++i)
    {
#if 0
        const auto seed = mSeeder.entropy() ? mSeeder()
                                            : std::chrono::steady_clock::now().time_since_epoch().count();
        std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
        std::uniform_int_distribution<int> dist(0, testInt);

        //int x = dist(eng);
#endif
        LOG_INFO << "Test INFO message [" << std::to_string(index) << "]"
                 << testMessage << " " << std::to_string(i);

        LOG_DEBUG << "Test WARN message [" << std::to_string(index) << "]"
                 << testMessage << " " << std::to_string(i);

        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main()
{
    auto twTimerPtr = std::make_shared<TimeWheelTimer>();

    auto LOGGERPTR = std::make_shared<Logger>();

    LOGGERPTR->setStandbyQueueMax(8);
    //LOGGERPTR->customPath("./temp/log");

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5_seconds, id);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::vector<std::thread> thds;

    for (size_t i = 0; i < 4; ++i)
    {
        thds.push_back(std::thread(func, i, 123456, std::string("this is a logger message")));
    }

    for (auto &t : thds)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(100));

    if (LOGGERPTR)
    {
        LOGGERPTR->stop();
        LOGGERPTR->join();
    }

    if (twTimerPtr)
    {
        twTimerPtr->stop();
        twTimerPtr->join();
    }

    return 0;
}