#include "Base/net/TCPSocket.h"
#include "Base/utility/Timer.h"
#include "Base/utility/Logger.h"
#include "Base/utility/LoggerMessage.h"
#include "Base/message/CustomMessages.h"
#include "Base/message/LoginMessage.h"

#include "GameDataTable/GameDataCode.h"

using namespace utility;
using namespace net;
using namespace message;
using namespace std;

const std::string BASE_MSG = "LOGIN_BASE_MSG";
const std::string HEARTBEAT_MSG = "HEARTBEAT_MSG";
const std::string LOGIN_MSG = "LOGIN_LOGIN_MSG";
const std::string LOGIN_CHARACTER_MSG = "LOGIN_LOGIN_CHARACTER_MSG";

int main(int argc, const char *argv[])
{
    if (argc != 4)
    {
        return -1;
    }

    size_t testIndex = std::stoul(argv[1]);
    size_t loopNum = std::stoul(argv[2]);
    size_t characterIndex = std::stoul(argv[3]);

    auto twTimerPtr = std::make_shared<timer::TimeWheelTimer>();
    auto LOGGERPTR = std::make_shared<logger::Logger>();
    LOGGERPTR->customPath("/home/zc/cppwork/WarringServer/tmp/log");
    LOGGERPTR->setStandbyQueueMax(4);

    auto id = twTimerPtr->generateID();
    LOGGERPTR->init(twTimerPtr, 5, id);

    auto sockfd = socket::createTCPSocket();
    socket::TCPSocket tcpsock(sockfd);
    socket::SocketAddr sa("192.168.119.128", 50006);
    tcpsock.connect(sa);
    tcpsock.setResueAddr(true);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (size_t i = 0; i < loopNum; ++i)
    {
        // try login
        message::LoginRequestMessage loginReqMsg;
        loginReqMsg.MSG_NAME = LOGIN_MSG;
        loginReqMsg.REQUEST_CODE = code::login::LOGIN_LOGIN_REQUEST_CODE;
        loginReqMsg.ACCOUNT = "zhangsan_" + std::to_string(testIndex * 10 + loopNum) + "@123.com";
        loginReqMsg.PASSWORD = "password_" + std::to_string(testIndex * 100 + loopNum);

        loginReqMsg.count();

        LOG_INFO << "Try to login ACCOUNT:" << loginReqMsg.ACCOUNT << " PASSWORD: " << loginReqMsg.PASSWORD;

        auto data = loginReqMsg.serialize();

        ::write(sockfd, data.data(), loginReqMsg.GET_MSG_LEN());

        char buf[1024] = {0};

        auto n = ::read(sockfd, buf, 1024);
        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";

            message::LoginResponseMessage loginResponseMsg;
            loginResponseMsg.deserialize(buf);

            LOG_INFO << "[MSG_NAME]: " << loginResponseMsg.MSG_NAME << " [REQUEST_RESULT]: " << loginResponseMsg.RESPONSE_CODE << " [REQUEST_TOKEN]: " << loginResponseMsg.TOKEN
                     << " [CHARACTER NUM]: " << loginResponseMsg.CHARACTER_BASE_INFO_MESSAGES.size();

            message::CharacterBaseInfo baseInfo;
            for (auto &msg : loginResponseMsg.CHARACTER_BASE_INFO_MESSAGES)
            {
                baseInfo.deserialize(msg.data());
                LOG_INFO << " [CHARACTER INFO]: [NAME]: " << baseInfo.CHARACTER_NAME
                         << " [LEVEL]: " << baseInfo.CHARACTER_LEVEL
                         << " [GENDER]: " << baseInfo.CHARACTER_GENDER
                         << " [OCCUPTION]: " << baseInfo.CHARACTER_OCCUPTION
                         << " [STATE]: " << baseInfo.CHARACTER_STATE;
            }
        }

        // try to register
        loginReqMsg.REQUEST_CODE = message::code::login::LOGIN_REGISTER_REQUEST_CODE;
        loginReqMsg.count();

        LOG_INFO << "Try to register ACCOUNT:" << loginReqMsg.ACCOUNT << " PASSWORD: " << loginReqMsg.PASSWORD;

        data = loginReqMsg.serialize();

        ::write(sockfd, data.data(), loginReqMsg.GET_MSG_LEN());

        ::memset(buf, 0, 1024);

        n = ::read(sockfd, buf, 1024);
        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";

            message::LoginResponseMessage loginResponseMsg;
            loginResponseMsg.deserialize(buf);

            LOG_INFO << "[MSG_NAME]: " << loginResponseMsg.MSG_NAME << " [REQUEST_RESULT]: " << loginResponseMsg.RESPONSE_CODE << " [REQUEST_TOKEN]: " << loginResponseMsg.TOKEN;
        }

        // try to create character
        message::LoginCharacterRequestMessage loginCharacterReqMsg;
        loginCharacterReqMsg.initAccount(loginReqMsg, LOGIN_CHARACTER_MSG);
        loginCharacterReqMsg.REQUEST_CODE = message::code::login::LOGIN_CREATE_CHARACTER_CODE;
        loginCharacterReqMsg.CHARACTER_NAME = "GameManager_" + std::to_string(characterIndex);
        loginCharacterReqMsg.CHARACTER_GENDER = false;
        loginCharacterReqMsg.CHARACTER_OCCUPTION = game::code::occuption::GAME_DATA_CODE_OCCUPTION_FARMER;
        loginCharacterReqMsg.CHARACTER_STATE = game::code::state::GAME_DATA_CODE_STATE_QI;
        loginCharacterReqMsg.count();

        LOG_INFO << "Try to create a new character : ACCOUNT:" << loginCharacterReqMsg.ACCOUNT << " CHARACTER_NAME: " << loginCharacterReqMsg.CHARACTER_NAME;

        data = loginCharacterReqMsg.serialize();

        ::write(sockfd, data.data(), loginCharacterReqMsg.GET_MSG_LEN());

        ::memset(buf, 0, 1024);

        n = ::read(sockfd, buf, 1024);
        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";

            message::LoginResponseMessage loginResponseMsg;
            loginResponseMsg.deserialize(buf);

            LOG_INFO << "[MSG_NAME]: " << loginResponseMsg.MSG_NAME << " [REQUEST_RESULT]: " << loginResponseMsg.RESPONSE_CODE << " [REQUEST_TOKEN]: " << loginResponseMsg.TOKEN;
        }
#if 1
        // try to enter game
        loginCharacterReqMsg.REQUEST_CODE = message::code::login::LOGIN_CHARACTER_ENTER_GAME_CODE;
        loginCharacterReqMsg.count();

        LOG_INFO << "Try to enter game:" << loginCharacterReqMsg.ACCOUNT << " CHARACTER: " << loginCharacterReqMsg.CHARACTER_NAME;

        data = loginCharacterReqMsg.serialize();

        ::write(sockfd, data.data(), loginCharacterReqMsg.GET_MSG_LEN());

        ::memset(buf, 0, 1024);

        n = ::read(sockfd, buf, 1024);
        if (n > 0)
        {
            LOG_INFO << "get data from server " << n << " bytes";

            message::LoginResponseMessage loginResponseMsg;
            loginResponseMsg.deserialize(buf);

            LOG_INFO << "[MSG_NAME]: " << loginResponseMsg.MSG_NAME << " [REQUEST_RESULT]: " << loginResponseMsg.RESPONSE_CODE << " [REQUEST_TOKEN]: " << loginResponseMsg.TOKEN
                     << " [GATEWAY IP]: " << loginResponseMsg.GAME_GATEWAY_IP << "[GATEWAY PORT] : " << loginResponseMsg.GAME_GATEWAY_PORT
                     << " [DS IP]: " << loginResponseMsg.GAME_DS_IP << "[DS PORT] : " << loginResponseMsg.GAME_DS_PORT;
        }
#endif
    }

    size_t intervalHeartbeat = 5;
    size_t increment = 0;

    for (size_t i = 0; i < loopNum; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(intervalHeartbeat + increment));

        message::HeartBeatPacket hbMsg;
        hbMsg.MSG_NAME = HEARTBEAT_MSG;
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

        increment += 10;
    }

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