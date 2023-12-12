#include "randomData.h"
#include <map>

#include <iostream>

std::vector<int> intVec;
std::vector<double> doubleVec;
std::map<int, double> dmap;
void print(bool dou)
{
    if (!dou)
    {
        for (auto i : intVec)
            std::cout << i << "\t";
        std::cout << std::endl;
    }
    else
    {
        for (auto d : doubleVec)
            std::cout << d << "\t";
        std::cout << std::endl;
    }
}

int main()
{
    utility::RandomData rd;
#if 0
    for (size_t i = 0; i < 10000; ++i)
    {
        ++dmap[rd.getPoisson(4)];
    }

    for (auto p : dmap)
    {
        std::cout << p.first << " " << std::string(p.second / 50, '*') << std::endl;
    }

    std::cout << "0 " << double(dmap[0]) / 10000 << "\n1 " << double(dmap[1]) / 10000 << "\n2 " << double(dmap[2]) / 10000
              << "\n3 " << double(dmap[3]) / 10000 << "\n4 " << double(dmap[4]) / 10000
              << "\n5 " << double(dmap[5]) / 10000 << "\n6 " << double(dmap[6]) / 10000
              << "\n7 " << double(dmap[7]) / 10000 << "\n8 " << double(dmap[8]) / 10000
              << std::endl;

    for (size_t i = 0; i < 10000; ++i)
    {
        doubleVec.push_back(rd.getExponential(1));
    }

    size_t res = 0;
    for (auto d : doubleVec)
    {
        if (d > 3)
            ++res;
    }
    double p = double(res) / 10000;
    std::cout << "when lambda = 1, p(Y>1) = " << p << std::endl;

    doubleVec.clear();
    res = 0;
    for (size_t i = 0; i < 10000; ++i)
    {
        doubleVec.push_back(rd.getExponential(0.3333));
    }
    for (auto d : doubleVec)
    {
        if (d > 3)
            ++res;
    }
    p = double(res) / 10000;
    std::cout << "when lambda = 0.33, p(Y>1) = " << p << std::endl;

    dmap.clear();
    for (size_t i = 0; i < 100000; i++)
    {
        ++dmap[rd.getNormal(50, 5)];
    }
    for (auto p : dmap)
    {
        std::cout << p.first << " " << std::string(p.second / 100, '*') << std::endl;
    }
#endif

    auto str = rd.getRandomStr(4096);

    std::cout << str << std::endl;

    return 0;
}