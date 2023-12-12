#include "testTcpClient.h"

using namespace utility;
using namespace net::socket;
using namespace std;

const size_t NEEDED_LENGTH = 52;

void cliEcho(int x)
{
    auto sockfd = createTCPSocket();
    TCPSocket tcpsock(sockfd);
    SocketAddr sa("192.168.119.128", 50005);
    tcpsock.connect(sa);
    tcpsock.setResueAddr(true);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    size_t len = 0;

    for (size_t i = x; i < x + 10; ++i)
    // while (len < NEEDED_LENGTH)
    {
        std::string str = "this is a message abcdefg hijklmn opq rst uvw xyz" + std::to_string(x);

        ::write(sockfd, str.c_str(), str.length());

        char buf[1024] = {0};

        auto n = ::read(sockfd, buf, 1024);
        if (n > 0)
        {
            LOG_INFO << "get data from server " << std::string(buf, n);
            len += n;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::hours(1));

    tcpsock.close();
}

void cliLogin(int index)
{
    auto sockfd = createTCPSocket();
    TCPSocket tcpsock(sockfd);
    SocketAddr sa("192.168.119.128", 50005);
    tcpsock.connect(sa);
    tcpsock.setResueAddr(true);

    // net::IOBuffer inputBuffer(sockfd);

    for (size_t i = 0; i < 1; ++i)
    {
        /*
        LoginMessage loginMsg;
        std::string MSG_NAME{"Message Login"};
        std::string account = std::to_string(index) + "_zhangsan";
        std::string password = std::to_string(index * index) + "_password";
        size_t len = sizeof(LoginMessage);
        memcpy(loginMsg.MSG_NAME, MSG_NAME.data(), MSG_NAME.length());
        loginMsg.MSG_LEN = len;
        memcpy(loginMsg.account, account.data(), account.length());
        memcpy(loginMsg.password, password.data(), password.length());

        char data[len + 1] = {0};
        memcpy(data, &loginMsg, len);

        // std::cout << "message : " << data << std::endl;

        auto n = ::write(sockfd, data, len);

        if (n > 0)
        {
            LOG_INFO << "send data to server " << account;
        }
        */

#if 1
        char buf[1024];
        auto x = ::read(sockfd, buf, 1024);
        if (x > 0)
        {
            // std::cout << "get data from server " << x << "bytes\n";
            // std::cout << "get data from server " << std::string(buf, x) << std::endl;

            LOG_INFO << "get data from server " << std::string(buf, x);
        }
#endif

        std::this_thread::sleep_for(std::chrono::hours(1));
    }

    tcpsock.close();
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cout << "PLEASE use " << argv[0] << " num\n";
    }

    size_t num = std::stoul(argv[1]);

    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();
    // LOGGERPTR->customPath("./build/log");

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

#if 0
    std::vector<std::thread> clithds;
    for (size_t i = 100; i <= 999; ++i)
    {
        clithds.push_back(std::thread(cliEcho, i));
        // clithds.push_back(std::thread(cliLogin, i));
    }

    for (auto &t : clithds)
    {
        t.join();
    }
#endif

    cliEcho(num);

    return 0;
}
