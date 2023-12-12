#include "Broker/RSBrokerHelper.h"

// #include "ServiceRegistryMessage.h"

std::string SERVICE_REGISTRY_ID_KEY = "Warring:ServReg:IDSuffix";

std::string BROKER_TEST_SERVICE_KEY = "Warring:Test";
std::string BROKER_TEST_KEY = "Warring:Test_";

std::string SERVICE_KEYS = "Warring:Service:KEYS";

std::string GATEWAY_KEYS = "Warring:Service:GATEWAYS";

std::string SERVICE_REGISTRY_CHANNEL = "Warring:ServReg:Channel";

auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");

size_t load = 0;

bool registering = true;

void onMessageFunc(std::string channnel, std::string msg)
{
    std::cout << "channel: " << channnel << " message: " << msg << std::endl;
    if (msg == "REGISTER_SUCCESS")
    {
        std::cout << "register success\n";

        registering = false;
    }
    else if (msg == "SEND_HEARTBEAT")
    {
        std::cout << "TODO should send a heatbeat\n";
        ++load;

        redis.set(BROKER_TEST_KEY, BROKER_TEST_KEY);

        redis.expire(BROKER_TEST_KEY, std::chrono::seconds(15));

        redis.zadd(GATEWAY_KEYS, BROKER_TEST_KEY, load);
    }
    else
    {
        std::cout << "unknown message\n";
    }
}

void onPMessageFunc(std::string pattern, std::string channel, std::string msg)
{
}

void onMetaFunc(sw::redis::Subscriber::MsgType type, sw::redis::OptionalString channel, long long num)
{
}

int main(int argc, const char *argv[])
{

    auto idSuffix = redis.incr(SERVICE_REGISTRY_ID_KEY);

    BROKER_TEST_KEY += std::to_string(idSuffix);

    redis.sadd(SERVICE_KEYS, BROKER_TEST_KEY);

    auto sub = redis.subscriber();

    sub.on_message(std::move(onMessageFunc));

    sub.on_pmessage(onPMessageFunc);

    sub.on_meta(onMetaFunc);

    sub.subscribe(SERVICE_REGISTRY_CHANNEL);

    while (registering)
    {
        try
        {
            sub.consume();
        }
        catch (const sw::redis::Error &err)
        {
            // Handle exceptions.
        }
    }

    std::cout << "register finished\n";

    while (true)
    {
        try
        {
            sub.consume();
        }
        catch (const sw::redis::Error &err)
        {
            // Handle exceptions.
        }
    }

    return 0;
}