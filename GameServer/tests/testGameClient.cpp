#include "Base/net/TCPSocket.h"
#include "Base/utility/Timer.h"
#include "Base/utility/Logger.h"
#include "Base/utility/LoggerMessage.h"

#include "GameDataTable/GameDataCode.h"
#include "GameDataTable/GameDataCharacterAttribute.h"
#include "Base/message/GameMessage.h"

using namespace utility;
using namespace net;
using namespace message;
using namespace std;

const std::string MESSAGE_GAME_HEARTBEAT_MSG = "HEARTBEAT_MSG";
const std::string MESSAGE_GAME_MSG = "GAME_MSG";
const std::string MESSAGE_GAME_CHARACTER_MSG = "GAME_CHARACTER_MSG";

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cout << "usage " << argv[0] << " sceneID\n";
        return EXIT_FAILURE;
    }

    int sceneID = std::stoi(argv[1]);

    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");
    LOGGERPTR->setStandbyQueueMax(4);
    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    auto sockfd = socket::createTCPSocket();
    socket::TCPSocket tcpsock(sockfd);
    socket::SocketAddr sa("192.168.119.128", 50100);
    tcpsock.connect(sa);
    tcpsock.setResueAddr(true);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    char buf[1024] = {0};

    // register ds
    message::DSMessage dsMsg;
    dsMsg.MSG_NAME = MESSAGE_GAME_MSG;
    dsMsg.CODE = message::code::game::GAME_DS_REGISTER_CODE;
    dsMsg.SCENE_ID = sceneID;
    dsMsg.LOAD = 0;
    dsMsg.count();
    auto data = dsMsg.serialize();
    LOG_INFO << "To send a register ds message with " << dsMsg.GET_MSG_LEN();

    ::write(sockfd, data.data(), dsMsg.GET_MSG_LEN());
    ::memset(buf, 0, 1024);
    auto n = ::read(sockfd, buf, 1024);

    if (n > 0)
    {
        LOG_INFO << "get data from server " << n << " bytes";
        dsMsg.deserialize(buf);
        LOG_INFO << "[MSG_NAME]: " << dsMsg.MSG_NAME << " [CODE]: " << dsMsg.CODE;
        if (dsMsg.CODE == message::code::game::GAME_DS_REGISTER_SUCCESS)
        {
            LOG_INFO << dsMsg.SCENE_ID << " register success";
        }
        else
        {
            LOG_INFO << dsMsg.SCENE_ID << " register get something wrong";
        }
    }

#if 0
    // update ds info
    for (size_t i = 0; i < 1; ++i)
    {
        message::DSMessage dsMsg2;
        dsMsg2.MSG_NAME = MESSAGE_GAME_MSG;
        dsMsg2.CODE = message::code::game::GAME_DS_UPDATE_CODE;
        dsMsg2.SCENE_ID = sceneID;
        dsMsg2.LOAD = 80 + i;
        dsMsg2.count();
        data = dsMsg2.serialize();
        LOG_INFO << "To send a update ds message with " << dsMsg2.GET_MSG_LEN();

        ::write(sockfd, data.data(), dsMsg2.GET_MSG_LEN());
        ::memset(buf, 0, 1024);
        n = ::read(sockfd, buf, 1024);

        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";
            dsMsg2.deserialize(buf);
            LOG_INFO << "[MSG_NAME]: " << dsMsg2.MSG_NAME << "[CODE]: " << dsMsg2.CODE;
        }

        message::HeartBeatPacket hbMsg;
        hbMsg.MSG_NAME = MESSAGE_GAME_HEARTBEAT_MSG;
        hbMsg.count();
        data = hbMsg.serialize();
        ::write(sockfd, data.data(), hbMsg.GET_MSG_LEN());
        ::memset(buf, 0, 1024);
        n = ::read(sockfd, buf, 1024);

        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";
            hbMsg.deserialize(buf);
            LOG_INFO << "[MSG_NAME]: " << hbMsg.MSG_NAME << "[MILLISECONDS_SINCE_EPOCH]: " << hbMsg.GET_MILLISECONDS_SINCE_EPOCH();
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
#endif

    // load character
    message::DSLoadOrUploadCharacterMessage dsLLMsg;
    dsLLMsg.MSG_NAME = MESSAGE_GAME_CHARACTER_MSG;
    dsLLMsg.CODE = message::code::game::GAME_LOAD_CHARACTER_CODE;
    dsLLMsg.SCENE_ID = sceneID;
    dsLLMsg.ACCOUNT = "zhangsan_11@123.com";
    dsLLMsg.CHARACTER_NAME = "GameManager_100";
    dsLLMsg.CHARACTER_DATA = "";

    dsLLMsg.count();
    data = dsLLMsg.serialize();
    LOG_INFO << "To send a load character ds message with " << dsLLMsg.GET_MSG_LEN();
    ::write(sockfd, data.data(), dsLLMsg.GET_MSG_LEN());
    ::memset(buf, 0, 1024);
    n = ::read(sockfd, buf, 1024);

    if (n > 0)
    {
        LOG_INFO << "get data from server " << n << " bytes";
        message::DSLoadOrUploadCharacterMessage dsLLResMsg;
        dsLLResMsg.deserialize(buf);
        LOG_INFO << "[MSG_NAME]: " << dsLLResMsg.MSG_NAME << "[CODE]: " << dsLLResMsg.CODE << "[CHARACTER_DATA_LEN]:" << dsLLResMsg.CHARACTER_DATA.size();

        message::CharacterInfo characterInfo;
        characterInfo.deserialize(dsLLResMsg.CHARACTER_DATA.data());

        LOG_INFO << characterInfo.ACCOUNT << " " << characterInfo.NAME
                 << " " << characterInfo.SCENE_ID << " " << characterInfo.SCENE_X << " " << characterInfo.SCENE_Y << " " << characterInfo.SCENE_Z;
    }

    // upload character
    message::CharacterInfo characterInfo;
    characterInfo.ACCOUNT = "zhangsan_11@123.com";
    characterInfo.NAME = "GameManager_100";
    characterInfo.LEVEL = 20;
    characterInfo.GENDER = true;
    characterInfo.OCCUPTION = game::code::occuption::GAME_DATA_CODE_OCCUPTION_WIZARD;
    characterInfo.STATE = game::code::state::GAME_DATA_CODE_STATE_WEI;
    characterInfo.EXP = 12300;
    characterInfo.HEALTH = 1000;
    characterInfo.MANA = 400;
    characterInfo.ARMOR = 150;
    characterInfo.STAMINA = 500;
    characterInfo.STRENGTH = 410;
    characterInfo.AGILITY = 320;
    characterInfo.INTELLECT = 330;
    characterInfo.SCENE_ID = 200002;
    characterInfo.SCENE_X = 1000.0;
    characterInfo.SCENE_Y = 15000.0;
    characterInfo.SCENE_Z = 90.0;
    characterInfo.count();
    message::DSLoadOrUploadCharacterMessage uploadMsg;

    uploadMsg.MSG_NAME = MESSAGE_GAME_CHARACTER_MSG;
    uploadMsg.CODE = message::code::game::GAME_UPLOAD_CHARACTER_CODE;
    uploadMsg.SCENE_ID = sceneID;
    uploadMsg.ACCOUNT = "zhangsan_11@123.com";
    uploadMsg.CHARACTER_NAME = "GameManager_100";
    uploadMsg.CHARACTER_DATA = characterInfo.serialize();

    uploadMsg.count();
    data = uploadMsg.serialize();
    LOG_INFO << "To send a upload character ds message with " << uploadMsg.GET_MSG_LEN();
    ::write(sockfd, data.data(), uploadMsg.GET_MSG_LEN());

    std::this_thread::sleep_for(std::chrono::hours(1));

    tcpsock.close();
#if 0
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
#endif
    return 0;
}