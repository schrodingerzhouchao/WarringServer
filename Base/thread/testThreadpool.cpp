#if 0
#include "Threadpool.h"
#include "../utility/Logger.h"
#include "../utility/LoggerLine.h"
#include "../utility/randomData.h"

using namespace utility;
using namespace utility::timer;
using namespace utility::logger;

uint16_t print(int i, const std::shared_ptr<utility::timer::TimeWheelTimer> &timerPtr,
               const std::shared_ptr<utility::SafeQueue<std::unique_ptr<utility::logger::LoggerBuffer4096Type>>> &filledQueuePtr,
               const std::shared_ptr<utility::SafeQueue<std::unique_ptr<utility::logger::LoggerBuffer4096Type>>> &emptyQueuePtr,
               const size_t secs)
{

    utility::RandomData rd;
    auto servtime = rd.getUniformInt(1, 5);
    // service time
    std::this_thread::sleep_for(std::chrono::milliseconds(servtime));
    // std::cout << "thread " << std::this_thread::get_id() << "\t" << i << std::endl;
    utility::logger::LoggerLine LOGGERLINE;
    LOGGERLINE.getTimewheelTimer(timerPtr);
    LOGGERLINE.getBufferPtrQueue(filledQueuePtr, emptyQueuePtr);
    LOGGERLINE.setTimerSeconds(secs);
    std::string varName = NAMEOF(LOGGERLINE);
    auto id = utility::Accessories::classAndVar(LOGGERLINE, varName);
    LOGGERLINE.setTimer(id);

    // if (i % 1 == 0)
    LOG_INFO << "\t" << i << LOG_END;
    LOG_DEBUG << LOG_COUT << "\t" << i << LOG_END;
#if 0
    else
    {
        if (i % 3 == 0)
            LOG_DEBUG << LOG_COUT << "\t" << i << LOG_END;
        if (i % 5 == 0)
            LOG_WARN << LOG_COUT << "\t" << i << LOG_END;
        if (i % 10 == 0)
            LOG_FATAL << LOG_COUT << "\t" << i << LOG_END;
    }
#endif
    auto waittime = rd.getUniformInt(20, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(waittime));

    return waittime / servtime;
}

int main()
{
    std::shared_ptr<TimeWheelTimer> timerPtr = std::make_shared<TimeWheelTimer>();

    Logger logger;
    logger.setFileSizeLimite(1024 * 100);
    logger.setSync(true);
    logger.createFile();

    auto filledQueuePtr = logger.filledLoggerBufferQueuePtr();
    auto emptyQueuePtr = logger.emptyLoggerBufferQueuePtr();


    base::thd::Threadpool thdpool;
    thdpool.getTimewheelTimer(timerPtr);
    std::string varName = NAMEOF(thdpool);
    auto id = utility::Accessories::classAndVar(thdpool, varName);
    thdpool.setTimer(id, base::thd::Threadpool::TimerType::ADJUST_THREAD_NUMBER);

    for (size_t i = 1; i <= 1000; ++i)
    {
        //thdpool.addTask(print, i, std::ref(timerPtr), std::ref(filledQueuePtr), std::ref(emptyQueuePtr), 5);
        thdpool.addTask(print, i, timerPtr, filledQueuePtr, emptyQueuePtr, 5);
    }

    thdpool.join();
    logger.join();
    timerPtr->join();
    return 0;
}

#endif

#include "Threadpool.h"
#include "../utility/Logger.h"
#include "../utility/LoggerMessage.h"

using namespace utility;
using namespace utility::timer;
using namespace utility::logger;
using namespace base::thd;

uint16_t testLoggerLine(const std::shared_ptr<Logger> &LOGGERPTR,
                        const size_t secs, const size_t x)
{
    LOG_DEBUG << "adasdqweqd sasda sdqweqwsdaq " << std::to_string(x);
    LOG_INFO << "ASDB 1111111111111 " << std::to_string(x);
    LOG_ERROR << LOG_COUT << "erqw129iuqugquiwge qwesdqwe " << std::to_string(x);
    LOG_WARN << "897987sdqwe8qnnkHIOHIO IHQIWOIOIHHIHOIHIOHIQWE " << std::to_string(x);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    return 0;
}

int main()
{

    std::shared_ptr<TimeWheelTimer> timerPtr = std::make_shared<TimeWheelTimer>();
    LOGGERPTR = std::make_shared<Logger>();
    LOGGERPTR->init(timerPtr, 5, "LOGGERPTR", 1024 * 100, true);

    Threadpool thdpool;
    thdpool.getTimewheelTimer(timerPtr);

    auto id = timerPtr->generateID();
    thdpool.setTimer(id, base::thd::Threadpool::TimerType::ADJUST_THREAD_NUMBER);

    for (size_t i = 1; i <= 100; ++i)
    {
        thdpool.addTask(testLoggerLine, std::ref(LOGGERPTR), 5, i);
    }

    thdpool.join();

    LOGGERPTR->join();
    timerPtr->join();


    return 0;
}