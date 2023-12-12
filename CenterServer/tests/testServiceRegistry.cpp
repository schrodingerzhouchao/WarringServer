#include "ServiceRegistry.h"

int main()
{
    auto twTimerPtr = std::make_shared<utility::timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<utility::logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");
    LOGGERPTR->setStandbyQueueMax(1);
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5_seconds, id);

    center_server::CenterServConf conf;

    center_server::registry::ServiceRegistry sr(conf);

    sr.initKeys();

    sr.run();

    return 0;
}