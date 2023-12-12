#include "GameServer.h"

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

    game_server::GameServConf gameServConf;

    net::socket::SocketAddr sa(port);

    game_server::GameServer gameServ(gameServConf,sa);

    gameServ.initKey();
    LOG_INFO << "Game server key: " << gameServConf.BROKER_GAME_KEY << " group: " << gameServConf.BROKER_GAME_GROUP;
    gameServ.registerService();

    gameServ.start(twTimerPtr);
}