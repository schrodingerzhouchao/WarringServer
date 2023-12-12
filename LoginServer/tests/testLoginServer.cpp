#include "LoginServer.h"
#include "Base/utility/Timer.h"
#include "Base/utility/Logger.h"
#include "Base/utility/LoggerMessage.h"

const std::string BASE_MSG = "BASE_MSG";
const std::string HEARTBEAT_MSG = "HEARTBEAT_MSG";
const std::string LOGIN_MSG = "LOGIN_MSG";
const std::string LOGIN_RESPONSE_MSG = "LOGIN_RESPONSE_MSG";

int main()
{
    auto twTimerPtr = std::make_shared<utility::timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<utility::logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");
    LOGGERPTR->setStandbyQueueMax(4);
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5_seconds, id);

    login_server::LoginServConf loginServConf;

    sw::redis::ConnectionOptions connOpts;
    connOpts.host = "127.0.0.1";
    connOpts.port = 6379;

    if (!loginServConf.readConf(connOpts, "testKey"))
    {
        LOG_ERROR << "read conf error";

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

        return EXIT_FAILURE;
    }

    net::socket::SocketAddr sa(50006);

    login_server::LoginServer loginServ(loginServConf, sa);

    

    loginServ.start(twTimerPtr);
}