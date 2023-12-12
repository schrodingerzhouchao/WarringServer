#include "Base/net/TCPSocket.h"
#include "Base/utility/Timer.h"
#include "Base/utility/Logger.h"
#include "Base/utility/LoggerMessage.h"

#include "Base/message/GatewayMessage.h"

using namespace utility;
using namespace net;
using namespace message;
using namespace std;

std::string MESSAGE_GATEWAY_HEARTBEAT_MSG = "HEARTBEAT_MSG";

int main()
{
    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    auto sockfd = socket::createTCPSocket();
    socket::TCPSocket tcpsock(sockfd);
    socket::SocketAddr sa("192.168.119.128", 50007);
    tcpsock.connect(sa);
    tcpsock.setResueAddr(true);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (size_t i = 0; i < 10; ++i)
    {
        message::TimestampMessage hbMsg;
        hbMsg.MSG_NAME = MESSAGE_GATEWAY_HEARTBEAT_MSG;
        hbMsg.count();

        LOG_INFO << "To send a heart beat message packet with " << hbMsg.GET_MSG_LEN();

        char buf[1024] = {0};

        auto data = hbMsg.serialize();
        ::write(sockfd, data.data(), hbMsg.GET_MSG_LEN());
        ::memset(buf, 0, 1024);
        auto n = ::read(sockfd, buf, 1024);

        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";
            hbMsg.deserialize(buf);
            LOG_INFO << "[MSG_NAME]: " << hbMsg.MSG_NAME << " [MILLISECONDS SINCE EPOCH]: " << hbMsg.GET_MILLISECONDS_SINCE_EPOCH();
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    tcpsock.close();

    if (LOGGERPTR)
    {
        LOGGERPTR->stop();
        LOGGERPTR->join();
    }

    if (twTimerPtr)
    {
        twTimerPtr->stop();
        twTimerPtr->join();
    }

    return 0;
}