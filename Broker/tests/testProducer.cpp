#include "RSBrokerHelper.h"
#include "Base/message/CustomMessages.h"
#include <iostream>
#include <thread>
#include <random>

using namespace sw::redis;
using namespace std;

std::random_device seeder;
const auto seed = seeder.entropy() ? seeder() : time(nullptr);
std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
uniform_int_distribution<int> dist(100, 3000);

const std::string TEST_KEY_1 = "test:key1";
const std::string TEST_KEY_2 = "test:key2";

// const std::string TEST_GROUP_1 = "group1";
// const std::string TEST_GROUP_2 = "group2";

// const std::string TEST_CONSUMER_1 = "consumer1";
// const std::string TEST_CONSUMER_2 = "consumer2";

void producer(size_t begin, size_t end, Redis &redis, const std::string &key)
{
    for (size_t i = begin; i <= end; ++i)
    {
        message::test::CLoginMessage cLoginMsg;
        cLoginMsg.requestCode = 10;
        cLoginMsg.account = "zhangsan_" + std::to_string(i) + "_" + key;
        cLoginMsg.password = "zhang_passwd_" + std::to_string(i * i) + "_" + key;
        cLoginMsg.count();

        message::message_carrier::BaseMessage msgCarrier;
        msgCarrier.TYPE = std::to_string(i);
        msgCarrier.MESSAGE = cLoginMsg.serialize();

        broker::RSBrokerHelper::push(redis, key, msgCarrier);

        std::this_thread::sleep_for(std::chrono::milliseconds(dist(eng)));
    }
}

int main()
{
    ConnectionOptions connection_options;
    connection_options.host = "127.0.0.1";

    ConnectionPoolOptions pool_options;
    pool_options.size = 4;

    Redis redis(connection_options, pool_options);

    thread t1(producer, 1, 10, std::ref(redis), std::ref(TEST_KEY_1));
    //thread t2(producer, 21, 40, std::ref(redis), std::ref(TEST_KEY_2));
    // thread t3(producer, 51, 60, std::ref(redis), std::ref(TEST_KEY_2));
    // thread t4(producer, 31, 40, std::ref(redis), std::ref(TEST_KEY_1));

    t1.join();
    //t2.join();
    // t3.join();
    // t4.join();

    return 0;
}