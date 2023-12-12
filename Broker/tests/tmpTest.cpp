#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;

struct Base
{
    virtual void printA() = 0;
    virtual void printB() = 0;
};

struct D1 : public Base
{
    int a = 10;
    string b = "abc";

    void printA() override { cout << a << endl; }
    void printB() override { cout << b << endl; }
};

struct D2 : public Base
{
    double a = 10.1234;
    bool b = true;

    void printA() override { cout << a << endl; }
    void printB() override { cout << std::boolalpha << b << endl; }
};

int main()
{
    thread_local unordered_map<string, unique_ptr<Base>> maps;
    maps.emplace("D1", make_unique<D1>());
    maps.emplace("D2", make_unique<D2>());

    maps["D1"]->printA();
    maps["D2"]->printA();
    maps["D1"]->printB();
    maps["D2"]->printB();

    return 0;
}