#include "QueryTask.h"

#include <regex>

namespace db_server
{
    namespace task
    {
        std::string QueryTask::getMillisecondsSinceEpoch()
        {
            std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
            return std::to_string(millis.count());
        }

        std::string QueryTask::generateToken(const std::string &account)
        {
            std::hash<std::string> hashFunc;
            return std::to_string(hashFunc(account + getMillisecondsSinceEpoch()));
        }

        std::string QueryTask::getLowLoadService(const std::string &serviceKey)
        {
            std::vector<std::pair<std::string, double>> result;
            mServRegRedis.zrangebyscore(serviceKey, sw::redis::UnboundedInterval<double>{}, std::back_inserter(result));
            for (const auto &pair : result)
            {
                if (mServRegRedis.ttl(pair.first) >= mConf.SERVICE_LOSE_THRESHOLD_IN_SECONDS / 2)
                {
                    return pair.first;
                }
            }
            return {};
        }

        std::string QueryTask::getLowDS(const std::string &dsKey)
        {
            std::vector<std::pair<std::string, double>> result;
            mServRegRedis.zrangebyscore(dsKey, sw::redis::UnboundedInterval<double>{}, std::back_inserter(result));
            if (!result.empty())
            {
                return result.at(0).first;
            }
            return {};
        }

        std::optional<std::pair<std::string, int>> QueryTask::getServiceAddress(const std::string &serviceKey)
        {
            if (serviceKey.empty())
                return std::nullopt;

            std::vector<std::string> vec;

            std::regex reg(":");
            std::regex_token_iterator pos(serviceKey.begin(), serviceKey.end(), reg, -1);
            decltype(pos) end;
            for (; pos != end; ++pos)
            {
                vec.push_back(*pos);
            }

            assert(vec.size() >= 2);

            return std::make_pair(vec.at(vec.size() - 2), std::stoi(vec.back()));
        }
    }
}

namespace db_server
{
    namespace task
    {
        void QueryTask::getLoginResponseMsg(message::message_carrier::LoginMessage &loginMsgCarrier)
        {
            LOG_INFO << "DBServGateman read message carrier: TYPE: " << loginMsgCarrier.TYPE
                     << " SERVICE: " << loginMsgCarrier.SERVICE
                     << " NEXT_SERVICE: " << loginMsgCarrier.NEXT_SERVICE
                     << " ID: " << loginMsgCarrier.CLIENT_ID
                     << " ADDRESS: " << loginMsgCarrier.CLIENT_ADDRESS
                     << " MESSAGE_LEN: " << loginMsgCarrier.MESSAGE.length();

            if (loginMsgCarrier.TYPE != mConf.MESSAGE_LOGIN_LOGIN_MSG)
            {
                loginMsgCarrier.TYPE = mConf.MESSAGE_LOGIN_ERROR_MSG;
                return;
            }

            message::LoginRequestMessage loginReqMsg;
            loginReqMsg.deserialize(loginMsgCarrier.MESSAGE.data());

            LOG_INFO << "DBServGateman read message: MSG_NAME: " << loginReqMsg.MSG_NAME
                     << " REQUEST_CODE: " << loginReqMsg.REQUEST_CODE
                     << " ACCOUNT: " << loginReqMsg.ACCOUNT
                     << " PASSWORD: " << loginReqMsg.PASSWORD;

            // next service is login service, means db server will response the login server's request
            loginMsgCarrier.NEXT_SERVICE = loginMsgCarrier.SERVICE;
            loginMsgCarrier.SERVICE = mConf.BROKER_DB_4_LOGIN_KEY;

            message::LoginResponseMessage loginResMsg;
            // loginResMsg.MSG_NAME = loginMsgCarrier.TYPE;
            loginResMsg.MSG_NAME = mConf.MESSAGE_LOGIN_LOGIN_RESPONSE_MSG;

            switch (loginReqMsg.REQUEST_CODE)
            {
            case message::code::login::LOGIN_REGISTER_REQUEST_CODE: // register a new accout
            {
                auto result = mDB.addNewAccount(loginReqMsg.ACCOUNT, loginReqMsg.PASSWORD, loginMsgCarrier.CLIENT_ADDRESS);
                if (result == 1)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_REGISTER_RESULT_SUCCESS;
                }
                else if (result == 0)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_REGISTER_ERROR_ACCOUNT_EXISTED;
                }
                else
                {
                    LOG_WARN << "Login task query db get exception";
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_FROM_DB;
                }
                LOG_INFO << "Login task query db get response: " << loginResMsg.RESPONSE_CODE;
                break;
            }

            case message::code::login::LOGIN_LOGIN_REQUEST_CODE: // login account
            {
                auto result = mDB.matchAccountAndUpdateLoginState(loginReqMsg.ACCOUNT, loginReqMsg.PASSWORD);
                if (result == 1)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_LOGIN_RESULT_SUCCESS;
                    // loginResMsg.TOKEN = generateToken(loginReqMsg.ACCOUNT); // TODO assign it when selected character enter game world
                    auto charactersBaseInfo = mDB.getAllCharacterBaseInfoFill2Message(loginReqMsg.ACCOUNT);
                    if (charactersBaseInfo.has_value() && charactersBaseInfo.value().size() > 0)
                    {
                        for (auto &baseInfo : charactersBaseInfo.value())
                        {
                            baseInfo.count();
                            loginResMsg.CHARACTER_BASE_INFO_MESSAGES.emplace_back(baseInfo.serialize());
                        }
                    }
                }
                else if (result == 0)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_LOGIN_ERROR_ACCOUNT_PASSWORD_MISMATCH;
                }
                else if (result == 2)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_LOGIN_ERROR_ACCOUNT_ALREADY_ON;
                }
                else
                {
                    LOG_WARN << "Login task query db get exception";
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_FROM_DB;
                }
                LOG_INFO << "Login task query db get response: " << loginResMsg.RESPONSE_CODE;
                break;
            }

            default:
                // TODO ack the error message type

                break;
            }

            loginResMsg.count();
            loginMsgCarrier.MESSAGE = loginResMsg.serialize();
        }

        void QueryTask::getLoginCharacterResponseMsg(message::message_carrier::LoginMessage &loginMsgCarrier)
        {
            LOG_INFO << "DBServGateman read message carrier: TYPE: " << loginMsgCarrier.TYPE
                     << " SERVICE: " << loginMsgCarrier.SERVICE
                     << " NEXT_SERVICE: " << loginMsgCarrier.NEXT_SERVICE
                     << " ID: " << loginMsgCarrier.CLIENT_ID
                     << " ADDRESS: " << loginMsgCarrier.CLIENT_ADDRESS
                     << " MESSAGE_LEN: " << loginMsgCarrier.MESSAGE.length();

            if (loginMsgCarrier.TYPE != mConf.MESSAGE_LOGIN_LOGIN_CHARACTER_MSG)
            {
                loginMsgCarrier.TYPE = mConf.MESSAGE_LOGIN_ERROR_MSG;
                return;
            }

            message::LoginCharacterRequestMessage loginCharacterReqMsg;
            loginCharacterReqMsg.deserialize(loginMsgCarrier.MESSAGE.data());

            LOG_INFO << "DBServGateman read message: MSG_NAME: " << loginCharacterReqMsg.MSG_NAME
                     << " REQUEST_CODE: " << loginCharacterReqMsg.REQUEST_CODE
                     << " ACCOUNT: " << loginCharacterReqMsg.ACCOUNT
                     << " PASSWORD: " << loginCharacterReqMsg.PASSWORD
                     << " [CHARACTER_NAME] " << loginCharacterReqMsg.CHARACTER_NAME
                     << " [CHARACTER_GENDER] " << loginCharacterReqMsg.CHARACTER_GENDER
                     << " [CHARACTER_OCCUPTION] " << loginCharacterReqMsg.CHARACTER_OCCUPTION
                     << " [CHARACTER_STATE] " << loginCharacterReqMsg.CHARACTER_STATE;

            // next service is login service, means db server will response the login server's request
            loginMsgCarrier.NEXT_SERVICE = loginMsgCarrier.SERVICE;
            loginMsgCarrier.SERVICE = mConf.BROKER_DB_4_LOGIN_KEY;

            message::LoginResponseMessage loginResMsg;
            // loginResMsg.MSG_NAME = loginMsgCarrier.TYPE;
            loginResMsg.MSG_NAME = mConf.MESSAGE_LOGIN_LOGIN_RESPONSE_MSG;

            switch (loginCharacterReqMsg.REQUEST_CODE)
            {
            case message::code::login::LOGIN_CREATE_CHARACTER_CODE: // create a new character
            {
                auto result = mDB.addNewCharacter(loginCharacterReqMsg);
                if (result == 1)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_CREATE_CHARACTER_RESULT_SUCCESS;
                }
                else if (result == 0)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_CREATE_CHARACTER_ERROR_EXISTED;
                }
                else
                {
                    LOG_WARN << "Login task query db get exception";
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_FROM_DB;
                }
                LOG_INFO << "Login task query db get response: " << loginResMsg.RESPONSE_CODE;
                break;
            }

            case message::code::login::LOGIN_DELETE_CHARACTER_CODE: // delete a character
            {
                auto result = mDB.removeCharacter(loginCharacterReqMsg.ACCOUNT, loginCharacterReqMsg.CHARACTER_NAME);
                if (result == 1)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_DELETE_CHARACTER_RESULT_SUCCESS;
                }
                else if (result == 0)
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_DELETE_CHARACTER_ERROR_NO_EXISTED;
                }
                else
                {
                    LOG_WARN << "Login task query db get exception";
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_FROM_DB;
                }
                LOG_INFO << "Login task query db get response: " << loginResMsg.RESPONSE_CODE;
                break;
            }
            case message::code::login::LOGIN_CHARACTER_ENTER_GAME_CODE: // enter game world
            {
                // auto result = mDB.getOneCharacterBaseInfoFill2Message(loginCharacterReqMsg.ACCOUNT, loginCharacterReqMsg.CHARACTER_NAME);
                auto result = mDB.getOneCharacterInfoFill2Message(loginCharacterReqMsg.ACCOUNT, loginCharacterReqMsg.CHARACTER_NAME);
                if (result.has_value())
                {
                    const std::string characterKey = mCache.generateCharacterKey(loginCharacterReqMsg.ACCOUNT, loginCharacterReqMsg.CHARACTER_NAME);

                    const std::string sceneKey = mConf.SERVICE_DS_PREFIX + std::to_string(result.value().SCENE_ID);

                    LOG_INFO << "sceneKey: " << sceneKey;

                    auto ttlResult = mCache.setTTL(characterKey, 300);
                    if (ttlResult == 1)
                    {
                        auto lowGatewayAddress = getLowLoadService(mConf.GATEWAY_KEYS);
                        auto lowSceneAddress = getLowLoadService(sceneKey);

                        LOG_INFO << "lowGatewayAddress: " << lowGatewayAddress << " lowSceneAddress:" << lowSceneAddress;

                        if (lowGatewayAddress.empty() || lowSceneAddress.empty() || (!getServiceAddress(lowGatewayAddress).has_value()) || (!getServiceAddress(sceneKey).has_value()))
                        {
                            loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_NO_SUCH_SERVICE;
                        }
                        else
                        {
                            loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_CHARACTER_ENTERING;

                            loginResMsg.GAME_GATEWAY_IP = getServiceAddress(lowGatewayAddress).value().first;
                            loginResMsg.GAME_GATEWAY_PORT = getServiceAddress(lowGatewayAddress).value().second;
                            loginResMsg.GAME_DS_IP = getServiceAddress(lowSceneAddress).value().first;
                            loginResMsg.GAME_DS_PORT = getServiceAddress(lowSceneAddress).value().second;
                        }
                    }
                    else if (ttlResult == 0)
                    {
                        std::unordered_map<std::string, std::string> characterInfo;

                        characterInfo.emplace(game::field_name::FIELD_ACCOUNT, result.value().ACCOUNT);
                        characterInfo.emplace(game::field_name::FIELD_CHARACTER_NAME, result.value().NAME);
                        characterInfo.emplace(game::field_name::FIELD_CHARACTER_LEVEL, std::to_string(result.value().LEVEL));
                        characterInfo.emplace(game::field_name::FIELD_CHARACTER_GENDER, result.value().GENDER ? "true" : "false");
                        characterInfo.emplace(game::field_name::FIELD_CHARACTER_OCCUPTION, std::to_string(result.value().OCCUPTION));
                        characterInfo.emplace(game::field_name::FIELD_CHARACTER_STATE, std::to_string(result.value().STATE));
                        characterInfo.emplace(game::field_name::FIELD_EXP, std::to_string(result.value().EXP));
                        characterInfo.emplace(game::field_name::FIELD_HEALTH, std::to_string(result.value().HEALTH));
                        characterInfo.emplace(game::field_name::FIELD_MANA, std::to_string(result.value().MANA));
                        characterInfo.emplace(game::field_name::FIELD_ARMOR, std::to_string(result.value().ARMOR));
                        characterInfo.emplace(game::field_name::FIELD_STAMINA, std::to_string(result.value().STAMINA));
                        characterInfo.emplace(game::field_name::FIELD_STRENGTH, std::to_string(result.value().STRENGTH));
                        characterInfo.emplace(game::field_name::FIELD_AGILITY, std::to_string(result.value().AGILITY));
                        characterInfo.emplace(game::field_name::FIELD_INTELLECT, std::to_string(result.value().INTELLECT));
                        characterInfo.emplace(game::field_name::FIELD_SCENE_ID, std::to_string(result.value().SCENE_ID));
                        characterInfo.emplace(game::field_name::FIELD_SCENE_X, std::to_string(result.value().SCENE_X));
                        characterInfo.emplace(game::field_name::FIELD_SCENE_Y, std::to_string(result.value().SCENE_Y));
                        characterInfo.emplace(game::field_name::FIELD_SCENE_Z, std::to_string(result.value().SCENE_Z));

                        result.value().count();
                        auto characterData = result.value().serialize();
                        characterInfo.emplace(game::field_name::FIELD_CHARACTER_DATA, characterData);

                        mCache.addCharacter(characterKey, characterInfo);
                        mCache.setTTL(characterKey, 300);

                        auto lowGatewayAddress = getLowLoadService(mConf.GATEWAY_KEYS);
                        auto lowSceneAddress = getLowLoadService(sceneKey);

                        LOG_INFO << "lowGatewayAddress: " << lowGatewayAddress << " lowSceneAddress:" << lowSceneAddress;

                        if (lowGatewayAddress.empty() || lowSceneAddress.empty() || (!getServiceAddress(lowGatewayAddress).has_value()) || (!getServiceAddress(sceneKey).has_value()))
                        {
                            loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_NO_SUCH_SERVICE;
                        }
                        else
                        {
                            loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_CHARACTER_ENTERING;

                            loginResMsg.GAME_GATEWAY_IP = getServiceAddress(lowGatewayAddress).value().first;
                            loginResMsg.GAME_GATEWAY_PORT = getServiceAddress(lowGatewayAddress).value().second;
                            loginResMsg.GAME_DS_IP = getServiceAddress(lowSceneAddress).value().first;
                            loginResMsg.GAME_DS_PORT = getServiceAddress(lowSceneAddress).value().second;
                        }
                    }
                    else
                    {
                        loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_ERROR_FROM_DB;
                    }
                }
                else
                {
                    loginResMsg.RESPONSE_CODE = message::code::login::LOGIN_CHARACTER_ERROR_ACCOUNT_MISMATCH;
                }
                LOG_INFO << "Login task query db get response: " << loginResMsg.RESPONSE_CODE;
                break;
            }

            default:
                break;
            }

            loginResMsg.count();
            loginMsgCarrier.MESSAGE = loginResMsg.serialize();
        }

        void QueryTask::getGameResponseMsg(message::message_carrier::GameMessage &gameMsgCarrier)
        {
            LOG_INFO << "DBServGateman read message carrier: TYPE: " << gameMsgCarrier.TYPE
                     << " SERVICE: " << gameMsgCarrier.SERVICE
                     << " NEXT_SERVICE: " << gameMsgCarrier.NEXT_SERVICE
                     << " CLIENT_ID: " << gameMsgCarrier.CLIENT_ID
                     << " MESSAGE_LEN: " << gameMsgCarrier.MESSAGE.length();

            if (gameMsgCarrier.TYPE != mConf.MESSAGE_GAME_MSG)
            {
                gameMsgCarrier.TYPE = mConf.MESSAGE_GAME_ERROR_MSG;
                return;
            }

            message::DSMessage dsMsg;
            dsMsg.deserialize(gameMsgCarrier.MESSAGE.data());

            LOG_INFO << "DBServGateman read message: MSG_NAME: " << dsMsg.MSG_NAME
                     << " REQUEST_CODE: " << dsMsg.CODE
                     << " SCENE_ID: " << dsMsg.SCENE_ID
                     << " LOAD: " << dsMsg.LOAD;
        }

        void QueryTask::getGameCharacterResponseMsg(message::message_carrier::GameMessage &gameMsgCarrier)
        {
            LOG_INFO << "DBServGateman read message carrier: TYPE: " << gameMsgCarrier.TYPE
                     << " SERVICE: " << gameMsgCarrier.SERVICE
                     << " NEXT_SERVICE: " << gameMsgCarrier.NEXT_SERVICE
                     << " CLIENT_ID: " << gameMsgCarrier.CLIENT_ID
                     << " MESSAGE_LEN: " << gameMsgCarrier.MESSAGE.length();

            if (gameMsgCarrier.TYPE != mConf.MESSAGE_GAME_CHARACTER_MSG)
            {
                gameMsgCarrier.TYPE = mConf.MESSAGE_GAME_ERROR_MSG;
                return;
            }

            message::DSLoadOrUploadCharacterMessage dsMsg;
            dsMsg.deserialize(gameMsgCarrier.MESSAGE.data());

            LOG_INFO << "DBServGateman read message: MSG_NAME: " << dsMsg.MSG_NAME
                     << " CODE: " << dsMsg.CODE
                     << " ACCOUNT: " << dsMsg.ACCOUNT
                     << " CHARACTER: " << dsMsg.CHARACTER_NAME;

            message::DSLoadOrUploadCharacterMessage dsResMsg;
            dsResMsg.MSG_NAME = mConf.MESSAGE_GAME_RESPONSE_MSG;

            switch (dsMsg.CODE)
            {
            case message::code::game::GAME_LOAD_CHARACTER_CODE:
            {
                // next service is ds service, means db server will response the ds server's request
                gameMsgCarrier.NEXT_SERVICE = gameMsgCarrier.SERVICE;
                gameMsgCarrier.SERVICE = mConf.BROKER_DB_4_GAME_KEY;

                dsResMsg.MSG_NAME = mConf.MESSAGE_GAME_CHARACTER_MSG;

                const std::string key = mCache.generateCharacterKey(dsMsg.ACCOUNT, dsMsg.CHARACTER_NAME);
                // message::CharacterInfo characterInfo;
                std::string characterData;
                auto result = mCache.getCharacterData(key, characterData);
                if (result > 0)
                {
                    dsResMsg.CODE = message::code::game::GAME_LOAD_CHARACTER_SUCCESS;
                    dsResMsg.ACCOUNT = dsMsg.ACCOUNT;
                    dsResMsg.CHARACTER_NAME = dsMsg.CHARACTER_NAME;
                    dsResMsg.CHARACTER_DATA = characterData;

                    dsResMsg.count();
                    gameMsgCarrier.MESSAGE = dsResMsg.serialize();

                    LOG_INFO << "get DSLoadOrUploadCharacterMessage from cache, size: " << dsResMsg.GET_MSG_LEN();
                }
                else
                {
                    auto characterInfoOptional = mDB.getOneCharacterInfoFill2Message(dsMsg.ACCOUNT, dsMsg.CHARACTER_NAME);
                    if (characterInfoOptional.has_value())
                    {
                        characterInfoOptional.value().count();
                        auto data = characterInfoOptional.value().serialize();
                        LOG_INFO << "get character info data size: " << data.size();

                        dsResMsg.CODE = message::code::game::GAME_LOAD_CHARACTER_SUCCESS;
                        dsResMsg.SCENE_ID = characterInfoOptional.value().SCENE_ID;
                        dsResMsg.ACCOUNT = dsMsg.ACCOUNT;
                        dsResMsg.CHARACTER_NAME = dsMsg.CHARACTER_NAME;
                        dsResMsg.CHARACTER_DATA = data;

                        dsResMsg.count();
                        gameMsgCarrier.MESSAGE = dsResMsg.serialize();

                        LOG_INFO << "get DSLoadOrUploadCharacterMessage from db, size: " << dsResMsg.GET_MSG_LEN();
                    }
                    else
                    {
                        dsResMsg.CODE = message::code::game::GAME_LOAD_CHARACTER_ERROR;
                        dsResMsg.count();
                        gameMsgCarrier.MESSAGE = dsResMsg.serialize();

                        LOG_INFO << "no get character info";
                    }
                }

                break;
            }
            case message::code::game::GAME_UPLOAD_CHARACTER_CODE:
            {
                gameMsgCarrier.NEXT_SERVICE = "";
                gameMsgCarrier.SERVICE = mConf.BROKER_DB_4_GAME_KEY;

                const std::string characterKey = mCache.generateCharacterKey(dsMsg.ACCOUNT, dsMsg.CHARACTER_NAME);

                std::unordered_map<std::string, std::string> characterInfo;
                characterInfo.emplace(game::field_name::FIELD_CHARACTER_DATA, dsMsg.CHARACTER_DATA);
                mCache.addCharacter(characterKey, characterInfo);

                // test upload character info to db immediately
                message::CharacterInfo characterInfoMsg;
                characterInfoMsg.deserialize(dsMsg.CHARACTER_DATA.data());
                mDB.updateCharacter(characterInfoMsg, false);

                break;
            }
            case message::code::game::GAME_LAST_UPLOAD_CHARACTER_CODE:
            {
                gameMsgCarrier.NEXT_SERVICE = "";
                gameMsgCarrier.SERVICE = mConf.BROKER_DB_4_GAME_KEY;

                const std::string characterKey = mCache.generateCharacterKey(dsMsg.ACCOUNT, dsMsg.CHARACTER_NAME);

                std::unordered_map<std::string, std::string> characterInfo;
                characterInfo.emplace(game::field_name::FIELD_CHARACTER_DATA, dsMsg.CHARACTER_DATA);
                mCache.addCharacter(characterKey, characterInfo);
                mCache.setTTL(characterKey);

                // test upload character info to db immediately
                message::CharacterInfo characterInfoMsg;
                characterInfoMsg.deserialize(dsMsg.CHARACTER_DATA.data());
                mDB.updateCharacter(characterInfoMsg, true);

                break;
            }

            default:
                break;
            }
        }

    }
}
