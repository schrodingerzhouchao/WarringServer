#include "GameMessageOp.h"

namespace game_server
{
    void GameMessageOp::messageFunc(const std::shared_ptr<net::TcpConnection> &tcpConnptr)
    {
        if (tcpConnptr == nullptr)
        {
            return;
        }

        message::MESSAGE_LENGTH_TYPE len = 0;
        auto tid = std::this_thread::get_id();

        // try to get a copy of message::internet::BaseMessage's MSG_LEN
        auto msg = tcpConnptr->tryGet(sizeof(len), false);
        if (msg.empty())
        {
            return;
        }

        ::memcpy(&len, msg.data(), sizeof(len));

        if (len >= sizeof(len))
        {
            // try to fetch a particular message
            msg = tcpConnptr->tryGet(len, true);
            if (!msg.empty())
            {
                message::TimestampMessage baseMsg;
                baseMsg.deserialize(msg.data());
                auto msgType = baseMsg.MSG_NAME;

                // check thread id is registered
                if (mMessageTypes.count(tid) == 0)
                {
                    LOG_FATAL << "NO SUCH THREAD ID " << tid;
                    return;
                }
                // check message type is registered
                if (mMessageTypes[tid].count(msgType) == 0)
                {
                    LOG_ERROR << "NO SUCH MESSAGE TYPE " << msgType;
                    return;
                }

                mMessageTypes[tid][msgType]->deserialize(msg.data());

                if (typeid(*mMessageTypes[tid][msgType]) == typeid(message::DSMessage))
                {
                    message::DSMessage *dsMsg = dynamic_cast<message::DSMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << dsMsg->MSG_NAME
                             << " [MESSAGE CODE]" << dsMsg->CODE << " [SCENE_ID]" << dsMsg->SCENE_ID
                             << " [SCENE PORT]" << dsMsg->DS_PORT
                             << " [MESSAGES_LEM]" << dsMsg->SCENE_DATA.size()
                             << " [LOAD]" << dsMsg->LOAD << " [MILLISECONDS SINCE EPOCH]" << dsMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(dsMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::DSMessage dsResMsg;
                    dsResMsg.MSG_NAME = dsMsg->MSG_NAME;

                    switch (dsMsg->CODE)
                    {
                    case message::code::game::GAME_DS_REGISTER_CODE:
                    {
                        auto sceneKey = mSceneManager.generateSceneIDKey(mConf.SERVICE_DS_PREFIX, dsMsg->SCENE_ID);
                        auto result = mSceneManager.registerScene(tcpConnptr->getIP(), sceneKey, dsMsg->DS_PORT, dsMsg->LOAD);
                        if (result >= 0)
                        {
                            dsResMsg.CODE = message::code::game::GAME_DS_REGISTER_SUCCESS;
                        }
                        else
                        {
                            dsResMsg.CODE = message::code::game::GAME_DS_REGISTER_ERROR;
                        }

                        break;
                    }
                    case message::code::game::GAME_DS_UPDATE_CODE:
                    {
                        auto sceneKey = mSceneManager.generateSceneIDKey(mConf.SERVICE_DS_PREFIX, dsMsg->SCENE_ID);
                        auto result = mSceneManager.updateScene(tcpConnptr->getIPPort(), sceneKey, dsMsg->LOAD);
                        if (result >= 0)
                        {
                            dsResMsg.CODE = message::code::game::GAME_DS_UPDATE_SUCCESS;
                        }
                        else
                        {
                            dsResMsg.CODE = message::code::game::GAME_DS_UPDATE_ERROR;
                        }

                        break;
                    }
                    case message::code::game::GAME_DS_QUIT_CODE:
                    {
                        auto result = mSceneManager.removeScene(tcpConnptr->getIPPort());

                        if (result >= 0)
                        {
                            dsResMsg.CODE = message::code::game::GAME_DS_QUIT_SUCCESS;
                        }
                        else
                        {
                            dsResMsg.CODE = message::code::game::GAME_DS_QUIT_ERROR;
                        }

                        break;
                    }

                    default:
                    {
                        dsResMsg.CODE = message::code::game::GAME_DS_UNKNOWN_ERROR;

                        break;
                    }
                    }
                    dsResMsg.SCENE_ID = dsMsg->SCENE_ID;
                    dsResMsg.SCENE_DATA = "";
                    dsResMsg.count();
                    auto data = dsResMsg.serialize();
                    tcpConnptr->send(data);
                }
                else if (typeid(*mMessageTypes[tid].at(msgType)) == typeid(message::DSLoadOrUploadCharacterMessage))
                {
                    message::DSLoadOrUploadCharacterMessage *dsMsg = dynamic_cast<message::DSLoadOrUploadCharacterMessage *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << dsMsg->MSG_NAME
                             << " [MESSAGE CODE]" << dsMsg->CODE << " [ACCOUNT]" << dsMsg->ACCOUNT << " [CHARACTER]" << dsMsg->CHARACTER_NAME
                             << " [MESSAGES_LEM]" << dsMsg->CHARACTER_DATA.size()
                             << " [MILLISECONDS SINCE EPOCH]" << dsMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(dsMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    message::message_carrier::GameMessage gameMsgCarrier;
                    gameMsgCarrier.TYPE = dsMsg->MSG_NAME;
                    gameMsgCarrier.SERVICE = mConf.BROKER_GAME_KEY;
                    gameMsgCarrier.NEXT_SERVICE = mConf.BROKER_DB_KEY;
                    gameMsgCarrier.CLIENT_ID = tcpConnptr->getName();
                    gameMsgCarrier.CLIENT_ADDRESS = tcpConnptr->getIPPort();
                    gameMsgCarrier.MESSAGE = msg;

                    broker::RSBrokerHelper::push(mBrokerRedis, mConf.BROKER_DB_KEY, gameMsgCarrier);
                }
                else if (typeid(*mMessageTypes[tid].at(msgType)) == typeid(message::HeartBeatPacket))
                {
                    message::HeartBeatPacket *hbMsg = dynamic_cast<message::HeartBeatPacket *>(mMessageTypes[tid].at(msgType).get());
                    LOG_INFO << tcpConnptr->getName() << " details: [MESSAGE_NAME]" << hbMsg->MSG_NAME
                             << " [MILLISECONDS SINCE EPOCH]" << hbMsg->GET_MILLISECONDS_SINCE_EPOCH()
                             << " [TID]" << std::this_thread::get_id();

                    tcpConnptr->setCurrentTimestamp(hbMsg->GET_MILLISECONDS_SINCE_EPOCH());

                    // TODO control heartbeat frequency? if (mTcpServer.getEventLoop(tid)->isIntervalOut())

                    // reply heartbeat to client
                    message::HeartBeatPacket repHBMsg;
                    repHBMsg.MSG_NAME = mConf.MESSAGE_GAME_HEARTBEAT_MSG;
                    repHBMsg.count();
                    auto data = repHBMsg.serialize();
                    tcpConnptr->send(data);
                }
                else
                {
                    // TODO restrict the client who send dangerous messages
                    LOG_INFO << tcpConnptr->getName() << " is undefined message type";
                }
            }
        }
    }

}