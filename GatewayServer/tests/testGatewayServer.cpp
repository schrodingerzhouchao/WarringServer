#include "GatewayServer.h"

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " port\n";
        return EXIT_FAILURE;
    }

    auto port = std::stoul(argv[1]);

    auto twTimerPtr = std::make_shared<utility::timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<utility::logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");
    LOGGERPTR->setStandbyQueueMax(4);
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5_seconds, id);

    gateway_server::GatewayServConf gatewayConf;

    net::socket::SocketAddr sa(port);

    gateway_server::GatewayServer gatewayServ(gatewayConf, sa);

    gatewayServ.initKey();
    LOG_INFO << "Gateway server key: " << gatewayConf.BROKER_GATEWAY_KEY << " group: " << gatewayConf.BROKER_GATEWAY_GROUP;
    gatewayServ.registerService();

    gatewayServ.start(twTimerPtr);
}