#include "utility/SafeHashmap.h"
#include "utility/Time.h"
#include <iostream>
#include <thread>

void testInsert(utility::SafeHashmap<std::string, std::string> &testMaps, size_t i)
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(i % 10));
    testMaps.insert(std::to_string(i) + "_testKey", std::to_string(i) + "_testValue");
}
void testFind(utility::SafeHashmap<std::string, std::string> &testMaps, size_t i)
{
    size_t x = i % 100;
    std::string key = std::to_string(x) + "_testKey";
    std::string val = {};
    std::string expectedVal = std::to_string(x) + "_testValue";
    if (testMaps.find(key, val))
    {
        if (val != expectedVal)
        {
            std::cout << key << " des not get the expected " << expectedVal << std::endl;
        }
    }
    else
    {
        std::cout << "No such key: " << key << std::endl;
    }
}

void testNull()
{
}

void readAll(const std::string &str)
{
    std::cout << str << std::endl;
}

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " #num\n";
        return EXIT_FAILURE;
    }

    size_t bucketNum = std::stoul(argv[1]);

    utility::SafeHashmap<std::string, std::string> testMaps(bucketNum, std::hash<std::string>());
    std::unordered_map<std::string, std::string> testMaps2;

    std::vector<std::thread> thds;

    // multi insert
    auto startPoint = utility::Time::microsecondsSinceEpoch();
    for (size_t i = 0; i < 10000; ++i)
    {
        thds.push_back(std::thread(testInsert, std::ref(testMaps), i));
        // thds.emplace_back(std::thread(testInsert, std::ref(testMaps), i));
    }
    for (auto &t : thds)
    {
        // if (t.joinable())
        t.join();
    }
    auto endInsertPoint = utility::Time::microsecondsSinceEpoch();
    std::cout << "use " << bucketNum << " buckets, elapsed time was " << endInsertPoint - startPoint << " microseconds for multi insert\n";

    thds.clear();

    // single insert
    startPoint = utility::Time::microsecondsSinceEpoch();
    for (size_t i = 0; i < 10000; ++i)
    {
        thds.push_back(std::thread(testNull));
        testMaps2.insert(std::make_pair(std::to_string(i) + "_testKey", std::to_string(i) + "_testValue"));
    }
    for (auto &t : thds)
    {
        // if (t.joinable())
        t.join();
    }
    endInsertPoint = utility::Time::microsecondsSinceEpoch();
    std::cout << "use # buckets, elapsed time was " << endInsertPoint - startPoint << " microseconds for single insert\n";

    thds.clear();

    // multi find
    startPoint = utility::Time::microsecondsSinceEpoch();
    for (size_t i = 0; i < 10000; ++i)
    {
        thds.push_back(std::thread(testFind, std::ref(testMaps), i));
    }
    for (auto &t : thds)
    {
        // if (t.joinable())
        t.join();
    }
    auto endPoint = utility::Time::microsecondsSinceEpoch();
    std::cout << "use " << bucketNum << " buckets, elapsed time was " << endPoint - startPoint << " microseconds for multi find\n";

    thds.clear();

    // single find
    startPoint = utility::Time::microsecondsSinceEpoch();
    for (size_t i = 0; i < 10000; ++i)
    {
        thds.push_back(std::thread(testNull));
        size_t x = i % 100;
        std::string key = std::to_string(x) + "_testKey";
        std::string val = {};
        std::string expectedVal = std::to_string(x) + "_testValue";
        auto it = testMaps2.find(key);
        if (it != testMaps2.end())
        {
            val = it->second;
            if (val != expectedVal)
            {
                std::cout << key << " des not get the expected " << expectedVal << std::endl;
            }
        }
        else
        {
            std::cout << "No such key: " << key << std::endl;
        }
    }
    for (auto &t : thds)
    {
        // if (t.joinable())
        t.join();
    }
    endPoint = utility::Time::microsecondsSinceEpoch();
    std::cout << "use # buckets, elapsed time was " << endPoint - startPoint << " microseconds for single find\n";

    startPoint = utility::Time::microsecondsSinceEpoch();
    for (size_t i = 0; i < 10000; ++i)
    {
        std::string key = std::to_string(i) + "_testKey";
        std::string val = {};
        std::string expectedVal = std::to_string(i) + "_testValue";
        if (testMaps.find(key, val))
        {
            if (val != expectedVal)
            {
                std::cout << key << " des not get the expected " << expectedVal << std::endl;
            }
        }
        else
        {
            std::cout << "No such key: " << key << std::endl;
        }
    }
    endPoint = utility::Time::microsecondsSinceEpoch();
    std::cout << "use " << bucketNum << " buckets, elapsed time was " << endPoint - startPoint << " microseconds for find\n";

    startPoint = utility::Time::microsecondsSinceEpoch();
    for (size_t i = 0; i < 10000; ++i)
    {
        std::string key = std::to_string(i) + "_testKey";
        std::string val = {};
        std::string expectedVal = std::to_string(i) + "_testValue";
        auto it = testMaps2.find(key);
        if (it != testMaps2.end())
        {
            val = it->second;
            if (val != expectedVal)
            {
                std::cout << key << " des not get the expected " << expectedVal << std::endl;
            }
        }
        else
        {
            std::cout << "No such key: " << key << std::endl;
        }
    }
    endPoint = utility::Time::microsecondsSinceEpoch();
    std::cout << "use # buckets, elapsed time was " << endPoint - startPoint << " microseconds for find\n";
}