#include "RThread.h"
#include "../utility/Time.h"
#include <iostream>

uint16_t print(int i)
{
    utility::Time time;
    auto begin = time.microsecondsSinceEpoch();
    auto ms = (i % 3) * 500;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    auto end = time.microsecondsSinceEpoch();
    std::cout << "printFunc " << i << " using " << (end - begin) << " microseconds\n";
    return i;
}

void print2(int i)
{
    utility::Time time;
    auto begin = time.microsecondsSinceEpoch();
    auto ms = (i % 3) * 500;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    auto end = time.microsecondsSinceEpoch();
    std::cout << "printFunc " << i + 10 << " using " << (end - begin) << " microseconds\n";
}

void clientWork1(size_t n, std::shared_ptr<base::thd::TaskWithFutureSafeQueueType<uint16_t>> &p)
{
    for (int i = 1; i <= n; ++i)
    {
        auto ms = (i % 5) * 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));

        base::thd::addTask(p, print, i);
    }
}

void clientWork2(size_t n, std::shared_ptr<base::thd::TaskWithFutureSafeQueueType<uint16_t>> &p)
{
    for (int i = 1; i <= n; ++i)
    {
        auto ms = (i % 5) * 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));

        base::thd::addTask(p, print2, i);
    }
}

void clientWork3(size_t n, std::shared_ptr<base::thd::TaskWithFutureSafeQueueType<uint16_t>> &p)
{
    for (int i = 1; i <= n; ++i)
    {
        auto ms = (i % 5) * 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        base::thd::addTask(p, print, i);
        // base::thd::addTask(p, print2, i);
    }
}

void getData(std::shared_ptr<utility::SafeQueue<uint16_t>> &p)
{
    uint16_t res = 0;
    while (true)
    {
        p->waitAndPop(res);
        std::cout << "getData: " << res << std::endl;
    }
}

int main()
{
    auto p = std::make_shared<base::thd::TaskWithFutureSafeQueueType<uint16_t>>();
    auto resp = std::make_shared<utility::SafeQueue<uint16_t>>();

    std::thread client(clientWork3, 10, std::ref(p));
    std::thread serv(getData, std::ref(resp));

    base::thd::RThread<uint16_t> t1;
    // base::thd::RThread<uint16_t> t2;

    // t1.command(base::RThread<uint16_t>::CommandType::CAPTURE_FUTURE_SWITCH);

    t1.linkExternalTasks(p);
    t1.linkResultsQueue(resp);

    if (!t1.isCaptureFuture())
        t1.command(base::thd::RThread<uint16_t>::CommandType::CAPTURE_FUTURE_SWITCH);
    // t2.linkExternalTasks(p);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    t1.command(base::thd::RThread<uint16_t>::CommandType::CANCEL_WHEN_COMPLETE);

    // t1.join();
    // t2.join();
    client.join();

    while (!t1.isInactive())
    {
    }
    std::cout << "now inactive\n";
    serv.join();
    return 0;
}