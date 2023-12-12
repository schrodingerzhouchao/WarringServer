#include <sw/redis++/redis++.h>

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <regex>

int main()
{
    auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");

    // string commands
    redis.set("key", "val");
    auto val = redis.get("key");
    if (val)
    {
        std::cout << "val = " << *val << std::endl;
    }
    redis.del("key");

    // list commands
    std::vector<std::string> vec = {"a", "b", "c"};
    redis.rpush("list", vec.begin(), vec.end());

    redis.rpush("list", {"x", "y", "z"});

    vec.clear();
    redis.lrange("list", 0, -1, std::back_inserter(vec));

    if (!vec.empty())
    {
        std::cout << "list = ";
        for (const auto &str : vec)
        {
            std::cout << str << ", ";
        }
        std::cout << std::endl;
    }
    redis.del("list");

    // hash commands
    redis.hset("hash", "field1", "val1");
    redis.hset("hash", std::make_pair("field2", "val2"));

    std::unordered_map<std::string, std::string> m = {{"field3", "val3"}, {"field4", "val4"}};
    redis.hmset("hash", m.cbegin(), m.cend());

    m.clear();
    redis.hgetall("hash", std::inserter(m, m.begin()));

    if (!m.empty())
    {
        std::cout << "hash = " << std::endl;
        for (const auto &kv : m)
        {
            std::cout << "\t" << kv.first << ", " << kv.second << std::endl;
        }
    }

    std::vector<sw::redis::OptionalString> vals;
    redis.hmget("hash", {"field3", "field2"}, std::back_inserter(vals));

    if (!vals.empty())
    {
        std::cout << "vals = ";
        for (const auto &val : vals)
        {
            if (val)
                std::cout << *val << ", ";
        }
        std::cout << std::endl;
    }
    redis.del("hash");

    // set commands
    redis.sadd("set", "m1");
    std::unordered_set<std::string> set = {"m2", "m3"};
    redis.sadd("set", set.cbegin(), set.cend());
    redis.sadd("set", {"m4", "m5"});

    set.clear();
    redis.smembers("set", std::inserter(set, set.begin()));

    if (redis.sismember("set", "m4"))
    {
        std::cout << "m4 exists in set" << std::endl;
    }

    if (!set.empty())
    {
        std::cout << "set = ";
        for (const auto &s : set)
        {
            std::cout << s << ", ";
        }
        std::cout << std::endl;
    }
    redis.del("set");

    // sorted set commands
    redis.zadd("sorted_set", "m1", 1.3);
    std::unordered_map<std::string, double> scores = {{"m2", 2.4}, {"m3", 0.1}};
    redis.zadd("sorted_set", scores.cbegin(), scores.cend());

    std::vector<std::pair<std::string, double>> zset_result1;
    std::vector<std::pair<std::string, double>> zset_result2;
    std::vector<std::pair<std::string, double>> zset_result3;
    redis.zrangebyscore("sorted_set", sw::redis::UnboundedInterval<double>{}, std::back_inserter(zset_result1));
    redis.zrangebyscore("sorted_set", sw::redis::BoundedInterval<double>(0.1, 1.4, sw::redis::BoundType::CLOSED), std::back_inserter(zset_result2));
    redis.zrangebyscore("sorted_set", sw::redis::BoundedInterval<double>(0.1, 1.4, sw::redis::BoundType::LEFT_OPEN), std::back_inserter(zset_result3));

    if (!(zset_result1.empty() || zset_result2.empty() || zset_result3.empty()))
    {
        std::cout << "zset_result1 = ";
        for (const auto &s : zset_result1)
        {
            std::cout << s.first << ", " << s.second << " | ";
        }
        std::cout << std::endl;
        std::cout << "zset_result2 = ";
        for (const auto &s : zset_result2)
        {
            std::cout << s.first << ", " << s.second << " | ";
        }
        std::cout << std::endl;
        std::cout << "zset_result3 = ";
        for (const auto &s : zset_result3)
        {
            std::cout << s.first << ", " << s.second << " | ";
        }
        std::cout << std::endl;
    }
    redis.del("sorted_set");

    // scripting commands
    auto num = redis.eval<long long>("return 1", {}, {});
    std::cout << "num = " << num << std::endl;

    std::atomic_uint64_t ID = 1;
    // redis.setnx("ID", std::to_string(ID));
    redis.set("ID", std::to_string(ID));

    std::vector<std::thread> thds;
    std::mutex mtx;

    for (size_t i = 0; i < 50; ++i)
    {
        thds.push_back(std::thread([&]()
                                   {
                        //auto result = redis.getset("ID", std::to_string(++ID));
                        auto result = redis.incr("-ID");
                        std::cout << result << std::endl; }));
    }

    for (auto &t : thds)
    {
        t.join();
    }

#if 0
    auto sub = redis.subscriber();

    sub.on_message([](std::string channel, std::string msg)
                   {
                       // Process message of PMESSAGE type.
                       std::cout<<"on_message(): channel: "<<channel
                                <<" message: "<<msg<<std::endl; });
    sub.on_pmessage([](std::string pattern, std::string channel, std::string msg)
                    {
                        // Process message of PMESSAGE type.
                        std::cout<<"on_pmessage(): channel: "<<channel
                                <<" pattern: "<<pattern
                                 <<" message: "<<msg<<std::endl; });
    sub.on_meta([](sw::redis::Subscriber::MsgType type, sw::redis::OptionalString channel, long long num)
                {
                        // Process message of PMESSAGE type.
                        if(channel.has_value())
                        {
                            std::cout<<"on_meta(): channel: "<<channel.value()
                                <<" num: "<<num<<std::endl; 
                        } });

    sub.subscribe("channel1");
    sub.subscribe({"channel2", "channel3"});

    sub.psubscribe("pattern1*");

    while (true)
    {
        try
        {
            sub.consume();
        }
        catch (const sw::redis::TimeoutError &e)
        {
            // Try again.
            continue;
        }
        catch (const sw::redis::Error &err)
        {
            // Handle other exceptions.
        }
    }
#endif

    std::unordered_map<std::string, size_t> loads = {{"service1", 1234}, {"service2", 213}, {"service3", 12221}, {"service4", 0}};

    redis.zadd("sorted_set", loads.begin(), loads.end());
    std::vector<std::pair<std::string, double>> loadResult;
    redis.zrangebyscore("sorted_set", sw::redis::UnboundedInterval<double>{}, std::back_inserter(loadResult));
    // redis.zrangebyscore("sorted_set", sw::redis::BoundedInterval<size_t>(0.1, 1.4, sw::redis::BoundType::CLOSED), std::back_inserter(loadResult));
    // redis.zrangebyscore("sorted_set", sw::redis::BoundedInterval<size_t>(0.1, 1.4, sw::redis::BoundType::LEFT_OPEN), std::back_inserter(loadResult));

    for (auto &pair : loadResult)
    {
        std::cout << pair.first << " : " << pair.second << std::endl;
    }

    std::unordered_map<std::string, std::string> tesetServiceKeys;
    tesetServiceKeys.emplace("k1", "1");
    tesetServiceKeys.emplace("k2", "10");

    auto it = tesetServiceKeys.insert_or_assign("k1", "10");

    std::cout << std::boolalpha << it.second << " " << tesetServiceKeys["k1"] << std::endl;
    if (!it.second)
    {
        tesetServiceKeys["k1"] = "10";
    }
    std::cout << tesetServiceKeys["k1"] << std::endl;

    tesetServiceKeys.emplace("k3", "1210");
    tesetServiceKeys.emplace("k4", "123");
    tesetServiceKeys.emplace("k5", "101");

    for (auto &key : tesetServiceKeys)
    {
        if (key.second == "123")
        {
            tesetServiceKeys.erase(key.first);
        }
    }

    for (auto &key : tesetServiceKeys)
    {
        std::cout << key.first << ":" << key.second << std::endl;
    }

    std::string keyWithAddress = "test:key:key1:192.168.1.1:123456";
#if 0
    auto pos = keyWithAddress.find_last_of(':');
    
    if (pos < keyWithAddress.length())
    {
        auto port = keyWithAddress.substr(pos + 1);
        
        std::cout << port << std::endl;
        std::cout << keyWithAddress << std::endl;
    }
#endif

    std::vector<std::string> splictVec;

    std::regex reg(":");
    std::regex_token_iterator pos(keyWithAddress.begin(), keyWithAddress.end(), reg, -1);

    decltype(pos) end;
    for (; pos != end; ++pos)
    {
        // std::cout << pos->str() << std::endl;
        splictVec.push_back(*pos);
    }

    for (const auto &field : splictVec)
    {
        std::cout << field << std::endl;
    }

    auto addressRes = std::make_pair(splictVec.at(splictVec.size() - 2), std::stoi(splictVec.back()));
    std::cout << addressRes.first << " " << addressRes.second << std::endl;

    return 0;
}