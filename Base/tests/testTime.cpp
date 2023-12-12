#include "utility/Time.h"
#include <iostream>
#include <thread>
#include <fstream>

#include <unistd.h>

#include <filesystem>

using utility::Time;

int main()
{
    std::ifstream comm("/proc/self/comm");
    std::string name;
    getline(comm, name);


    auto pid = ::getpid();

    std::cout << "proc name " << name << "pid " << pid << std::endl;

    std::cout<<"cwd "<<std::filesystem::current_path()<<std::endl;

    std::filesystem::path dir;
    dir= "/home/zc/temp/log";

    auto res = std::filesystem::create_directories(dir);
    if(res)
    {
        std::cout<<"success\n";
        std::cout<<"new dir name:"<<dir.c_str()<<std::endl;
    }
    else if(std::filesystem::exists(dir))
    {
        std::cout<<"exist\n";

        std::filesystem::remove(dir);
    }
    else
    {
        std::cout<<"other error\n";
    }

    

    Time t;

    auto now1 = t();
    auto now2 = t.getRough();
    auto now3 = t.getNow();

    std::cout << "[Normal]: " << now1 << std::endl;
    std::cout << "[Rough] : " << now2 << std::endl;
    std::cout << "[Lazy]  : " << now3 << std::endl;

    auto begin1 = t.microsecondsSinceEpoch();

    for (size_t i = 0; i < 1000; ++i)
    {
        auto now = t();
    }

    auto end1 = t.microsecondsSinceEpoch();

    auto begin2 = t.microsecondsSinceEpoch();

    for (size_t i = 0; i < 1000; ++i)
    {
        auto now = t.getNow();
    }

    auto end2 = t.microsecondsSinceEpoch();

    std::cout << "[Normal] use " << end1 - begin1 << " microseconds for 1000 loops\n"
              << "[Lazy]   use " << end2 - begin2 << " microseconds for 1000 loops\n";

    for (size_t i = 0; i < 20; ++i)
    {
        std::cout << t.getNow() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(51));
    }

    return 0;
}