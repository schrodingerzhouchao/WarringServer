#include "RSBrokerHelper.h"
#include <thread>

int main()
{
    std::unordered_map<std::string, std::string> keys;

    keys.emplace("testXKey","$");

    while (true)
    {
        // broker::RSBrokerHelper::pull()
    }
}