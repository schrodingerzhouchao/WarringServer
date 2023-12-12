
#include "message/CustomMessages.h"
#include "message/LoginMessage.h"
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <unordered_map>
#include <functional>
#include <memory>

using namespace std;

int main()
{
    std::cout << "Test message\n";
    int fds[2];
    if (::pipe(fds) == -1)
    {
        cout << "pipe() error\n";
    }
    char buff[1024] = {0};

    // test BaseMessage
    cout << "*****Test BaseMessage*****\n";
    message::BaseMessage baseMsg, baseMsgResult;

    baseMsg.MSG_NAME = "Base MESSAGE TYPE";
    baseMsg.count();
    auto baseData = baseMsg.serialize();
    cout << "data length: " << baseData.length() << ", BaseMessage LEN: " << baseMsg.GET_MSG_LEN() << endl;

    ::write(fds[1], baseData.data(), baseData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    baseMsgResult.deserialize(buff);
    cout << "Result: "
         << "NAME: " << baseMsgResult.MSG_NAME
         << ", LEN: " << baseMsgResult.GET_MSG_LEN()
         << ", BaseMessage ARRAY_LIKE_LENGTH: " << baseMsgResult.GET_ARRAY_LIKE_S_SIZE() << endl;

    // test TimestampMessage
    cout << "*****Test TimestampMessage*****\n";
    message::TimestampMessage timestampMsg, timestampMsgResult;
    timestampMsg.MSG_NAME = "TIMESTAMP MESSAGE TYPE";
    timestampMsg.count();
    auto timestampData = timestampMsg.serialize();
    cout << "data length: " << timestampData.length() << " TimestampMessage LEN: " << timestampMsg.GET_MSG_LEN() << endl;

    ::write(fds[1], timestampData.data(), timestampData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    timestampMsgResult.deserialize(buff);
    cout << "Result: "
         << "NAME: " << timestampMsgResult.MSG_NAME
         << ", LEN: " << timestampMsgResult.GET_MSG_LEN()
         << ", TimestampMessage ARRAY_LIKE_LENGTH: " << baseMsgResult.GET_ARRAY_LIKE_S_SIZE()
         << ", TIMESTAMP: " << timestampMsgResult.GET_MILLISECONDS_SINCE_EPOCH() << endl;

    // test TestVecNormalMessage
    cout << "*****Test TestVecNormalMessage*****\n";
    message::test::TestVecNormal testVecNormalMsg, testVecNormalResult;
    testVecNormalMsg.MSG_NAME = "TEST VECTOR NORMAL MESSAGE TYPE";
    std::vector<int> ints = {1, 3, 5, 7, 9, 12, 14, 16, 18, 20};
    std::vector<double> doubles = {1.1, 3.3, 5.5, 7.7, 9.9, 12.12, 14.14, 16.16, 18.18, 20.20};
    testVecNormalMsg.INTS = std::move(ints);
    testVecNormalMsg.DOUBLES = std::move(doubles);
    testVecNormalMsg.count();
    auto testVecNormalData = testVecNormalMsg.serialize();
    cout << "data length: " << testVecNormalData.length() << " TestVecNormal LEN: " << testVecNormalMsg.GET_MSG_LEN() << endl;

    ::write(fds[1], testVecNormalData.data(), testVecNormalData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    testVecNormalResult.deserialize(buff);
    cout << "Result: "
         << "NAME: " << testVecNormalResult.MSG_NAME
         << ", LEN: " << testVecNormalResult.GET_MSG_LEN()
         << ", TestVecNormal ARRAY_LIKE_LENGTH: " << testVecNormalResult.GET_ARRAY_LIKE_S_SIZE()
         << ", INTS SIZE: " << testVecNormalResult.INTS.size() << endl;
    for (const auto &x : testVecNormalResult.INTS)
    {
        cout << x << "\t";
    }
    cout << endl;
    for (const auto &x : testVecNormalResult.DOUBLES)
    {
        cout << x << "\t";
    }
    cout << endl;

    // test TestStringsMessage
    cout << "*****Test TestStringsMessage*****\n";
    message::test::TestStringsMessage stringsMsg, stringsMsgResult;
    stringsMsg.MSG_NAME = "TEST STRINGS VECTOR TYPE";
    vector<size_t> uls = {123, 321, 111111111, 222, 0, 233333333};
    vector<string> strs = {"abc", "xyz", "1zzzz23", "ws123123ad", "@!#)"};
    stringsMsg.BOOL = false;
    stringsMsg.INTS = std::move(uls);
    stringsMsg.STRINGS = std::move(strs);
    stringsMsg.count();
    auto stringsMsgData = stringsMsg.serialize();
    cout << "data length: " << stringsMsgData.length() << " TestStringsMessage LEN: " << stringsMsg.GET_MSG_LEN() << endl;

    ::write(fds[1], stringsMsgData.data(), stringsMsgData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    stringsMsgResult.deserialize(buff);
    cout << "Result: "
         << "NAME: " << stringsMsg.MSG_NAME
         << ", LEN: " << stringsMsg.GET_MSG_LEN()
         << ", TestVecNormal ARRAY_LIKE_LENGTH: " << stringsMsg.GET_ARRAY_LIKE_S_SIZE()
         << ", INTS SIZE: " << stringsMsg.INTS.size()
         << ", STRINGS SIZE: " << stringsMsg.STRINGS.size() << endl;
    cout << "BOOL: " << boolalpha << stringsMsg.BOOL << std::endl;
    for (const auto &ul : stringsMsg.INTS)
    {
        cout << ul << " ";
    }
    cout << endl;
    for (const auto &str : stringsMsg.STRINGS)
    {
        cout << str << endl;
    }

    this_thread::sleep_for(std::chrono::seconds(1));

    // test MessagesPacker
    cout << "*****Test MessagesPacker*****\n";
    message::test::MessagesPacker msgPacker, msgPackerResult;
    msgPacker.MSG_NAME = "MESSAGE PACKER TYPE";
    msgPacker.MESSAGES.emplace_back(timestampData);
    msgPacker.MESSAGES.emplace_back(testVecNormalData);
    msgPacker.MESSAGES.emplace_back(stringsMsgData);
    msgPacker.count();
    auto msgPackerData = msgPacker.serialize();
    cout << "data length: " << msgPackerData.length() << " MessagesPacker LEN: " << msgPacker.GET_MSG_LEN() << endl;

    ::write(fds[1], msgPackerData.data(), msgPackerData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    msgPackerResult.deserialize(buff);
    message::TimestampMessage msg1;
    message::test::TestVecNormal msg2;
    message::test::TestStringsMessage msg3;
    cout << msgPackerResult.MSG_NAME << " " << msgPackerResult.GET_MILLISECONDS_SINCE_EPOCH() << ", Old data:" << endl;
    for (const auto &message : msgPackerResult.MESSAGES)
    {
        cout << "message length: " << message.length() << endl;
    }

    msg1.deserialize(msgPackerResult.MESSAGES[0].data());
    cout << msg1.MSG_NAME << " " << msg1.GET_MILLISECONDS_SINCE_EPOCH()
         << ", elapsed milliseconds " << msgPackerResult.GET_MILLISECONDS_SINCE_EPOCH() - msg1.GET_MILLISECONDS_SINCE_EPOCH() << endl;
    msg2.deserialize(msgPackerResult.MESSAGES[1].data());
    cout << msg2.MSG_NAME << endl;
    for (const auto i : msg2.INTS)
    {
        cout << i << "\t";
    }
    cout << endl;
    for (const auto d : msg2.DOUBLES)
    {
        cout << d << "\t";
    }
    cout << endl;
    msg3.deserialize(msgPackerResult.MESSAGES[2].data());
    cout << msg3.MSG_NAME << " " << boolalpha << msg3.BOOL << endl;
    for (const auto i : msg3.INTS)
    {
        cout << i << " ";
    }
    cout << endl;
    for (const auto &s : msg3.STRINGS)
    {
        cout << s << endl;
    }

    // test LoginRequestMessage
    cout << "*****Test LoginRequestMessage*****\n";
    message::LoginRequestMessage loginMsg, loginMsgResult;
    loginMsg.MSG_NAME = message::code::login::MESSAGE_TYPE_LOGIN;
    loginMsg.ACCOUNT = "zhangsan@123.com";
    loginMsg.PASSWORD = "zhangsan@password";
    loginMsg.REQUEST_CODE = message::code::login::LOGIN_LOGIN_REQUEST_CODE;
    loginMsg.count();
    auto loginMsgData = loginMsg.serialize();

    ::write(fds[1], loginMsgData.data(), loginMsgData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    loginMsgResult.deserialize(buff);
    cout << "data length: " << loginMsgData.length() << " result LEN: " << loginMsgResult.GET_MSG_LEN() << endl;
    cout << "MSG_NAME:" << loginMsgResult.MSG_NAME
         << " ACCOUNT: " << loginMsgResult.ACCOUNT
         << " PASSWORD: " << loginMsgResult.PASSWORD
         << " REQUEST_CODE: " << loginMsgResult.REQUEST_CODE
         << " TIMESTAMP: " << loginMsgResult.GET_MILLISECONDS_SINCE_EPOCH() << endl;

    // test LoginCharacterMessage
    cout << "*****Test LoginCharacterMessage*****\n";
    message::LoginCharacterRequestMessage loginCharacterMsg, loginCharacterMsgResult;
    loginCharacterMsg.MSG_NAME = message::code::login::MESSAGE_TYPE_LOGIN;
    loginCharacterMsg.ACCOUNT = "wangwu@456.com";
    loginCharacterMsg.PASSWORD = "wangwu@password";
    loginCharacterMsg.REQUEST_CODE = message::code::login::LOGIN_CREATE_CHARACTER_CODE;
    loginCharacterMsg.CHARACTER_NAME = "GameManager";
    loginCharacterMsg.CHARACTER_GENDER = true;
    loginCharacterMsg.CHARACTER_OCCUPTION = 12345;
    loginCharacterMsg.CHARACTER_STATE = 54321;
    loginCharacterMsg.count();
    auto loginCharacterMsgData = loginCharacterMsg.serialize();

    ::write(fds[1], loginCharacterMsgData.data(), loginCharacterMsgData.length());
    ::memset(buff, 0, 1024);
    ::read(fds[0], buff, 1024);

    loginCharacterMsgResult.deserialize(buff);
    cout << "data length: " << loginCharacterMsgData.length() << " result LEN: " << loginCharacterMsgResult.GET_MSG_LEN() << endl;
    cout << "MSG_NAME:" << loginCharacterMsgResult.MSG_NAME
         << " ACCOUNT: " << loginCharacterMsgResult.ACCOUNT
         << " PASSWORD: " << loginCharacterMsgResult.PASSWORD
         << " REQUEST_CODE: " << loginCharacterMsgResult.REQUEST_CODE
         << " CHARACTER_NAME: " << loginCharacterMsgResult.CHARACTER_NAME
         << " CHARACTER_GENDER: " << loginCharacterMsgResult.CHARACTER_GENDER
         << " CHARACTER_OCCUPTION: " << loginCharacterMsgResult.CHARACTER_OCCUPTION
         << " CHARACTER_STATE: " << loginCharacterMsgResult.CHARACTER_STATE
         << " TIMESTAMP: " << loginCharacterMsgResult.GET_MILLISECONDS_SINCE_EPOCH() << endl;

#if 0
    std::string testToken = "zhaoliu@123.com";
    std::hash<std::string> hashFunc;
    size_t sz = hashFunc(testToken);

    cout << "testToken " << testToken << " hash value " << sz << " hash str " << to_string(sz) << endl;
#endif

    return 0;
}