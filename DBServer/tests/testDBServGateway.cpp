#include "DBServerGateway.h"
#include "Base/utility/Logger.h"

int main()
{
    auto twTimerPtr = std::make_shared<utility::timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<utility::logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    db_server::DBServConf dbServConf;

    db_server::DBServGateway dbServGateway(dbServConf);

    dbServGateway.start();

    return 0;
}