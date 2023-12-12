#include "utility/Timer.h"
#include "utility/Logger.h"
#include "utility/LoggerMessage.h"

#include <sys/eventfd.h>

#include <functional>
#include <mutex>
#include <condition_variable>

using namespace utility;
using namespace utility::timer;

const std::string PRINT = "print";
const std::string ADD = "add";
const std::string MINUS = "minus";

class TestTask
{
public:
    TestTask(int i, const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr)
        : mInt(i),
          mTimerWeak(timerPtr),
          mExecThread(&TestTask::run, this)
    {
    }

    ~TestTask()
    {
        if (mExecThread.joinable())
            mExecThread.join();
    }

    void init()
    {
        LOG_INFO << "TestTask::init()";

        std::function<void()> funcPrint = std::bind(&TestTask::pushTaskID, this, std::ref(PRINT));
        std::function<void()> funcAdd = std::bind(&TestTask::pushTaskID, this, std::ref(ADD));
        std::function<void()> funcMinus = std::bind(&TestTask::pushTaskID, this, std::ref(MINUS));

        auto twTimerPtr = mTimerWeak.lock();
        if (twTimerPtr)
        {
            auto id = twTimerPtr->generateID();
            utility::timer::TimerTask tt1(0_hours, 1_minutes, 0_seconds, -1, id, funcPrint);

            id = twTimerPtr->generateID();
            utility::timer::TimerTask tt2(0_hours, 0_minutes, 2_seconds, -1, id, funcAdd);

            id = twTimerPtr->generateID();
            utility::timer::TimerTask tt3(0_hours, 0_minutes, 3_seconds, -1, id, funcMinus);

            twTimerPtr->addTask(tt1);
            // twTimerPtr->addTask(tt2);
            // twTimerPtr->addTask(tt3);
        }
    }

    void testPrint()
    {
        LOG_INFO << "TestTask <-----" << mInt << "-----> ::testPrint() ";
    }

    void testAdd()
    {
        ++mInt;
        // mInt = 100;
    }

    void testMinus()
    {
        --mInt;
        // mInt = -100;
    }

    void pushTaskID(const std::string &id)
    {
        // LOG_INFO << "TestTask::pushTaskID";
        std::unique_lock lock(mMutex);
        mFuncIDsToBeexecuted.push(id);
        mCond.notify_one();
    }

    void run()
    {
        LOG_DEBUG << "TestTask::run()";
        // register
        mFuncs.emplace(PRINT, std::bind(&TestTask::testPrint, this));
        mFuncs.emplace(ADD, std::bind(&TestTask::testAdd, this));
        mFuncs.emplace(MINUS, std::bind(&TestTask::testMinus, this));

        for (;;)
        {
            std::string id = {};
            {
                std::unique_lock lock(mMutex);
                mCond.wait(lock, [this]
                           { return !mFuncIDsToBeexecuted.empty(); });

                id = mFuncIDsToBeexecuted.front();
                mFuncIDsToBeexecuted.pop();
            }

            auto it = mFuncs.find(id);
            if (it != mFuncs.end())
            {
                LOG_DEBUG << "TestTask::run() get func and run it";
                it->second();
            }
        }
    }

private:
    int mInt = 0;

    std::weak_ptr<utility::timer::TimeWheelTimer> mTimerWeak;

    std::thread mExecThread;

    std::queue<std::string> mFuncIDsToBeexecuted;
    std::unordered_map<std::string, std::function<void()>> mFuncs;

    std::condition_variable mCond;
    std::mutex mMutex;
};

const size_t N = 10000;

time_t startPoint = 0, endPoint = 0;

void printSomething(size_t i)
{
    if (i == 0)
        startPoint = utility::Time::millisecondsSinceEpoch();
    if (i == N - 1)
        endPoint = utility::Time::millisecondsSinceEpoch();
    LOG_INFO << "xxxx----xxxx";
}

int main()
{
    std::shared_ptr<utility::timer::TimeWheelTimer> twTimerPtr = std::make_shared<utility::timer::TimeWheelTimer>();

    auto LOGGERPTR = std::make_shared<logger::Logger>();
    LOGGERPTR->setStandbyQueueMax(8);
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    // LOG_INFO << "Test Timer";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // TestTask testTask(0, twTimerPtr);

    // testTask.init();

    for (size_t i = 0; i < N; ++i)
    {
        auto id = twTimerPtr->generateID();
        TimerTask tt(0_hours, 0_minutes, 2_seconds, 1, id, std::bind(printSomething, i));
        twTimerPtr->addTask(tt);

        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // LOGGERPTR->join();
    while (startPoint == 0 || endPoint == 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "finish " << N << " timer tasks which registered in same time take " << endPoint - startPoint << " milliseconds\n";

    twTimerPtr->join();

    return 0;
}