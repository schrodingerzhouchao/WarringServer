
#include "db/Database.h"

#include <iostream>

const std::string MONGO_DB_URL = std::string("mongodb://192.168.119.128:27017/?minPoolSize=0&maxPoolSize=4");

int main()
{
    std::cout << "Hello MongoDB\n";

    db_server::db::Database db(MONGO_DB_URL);

    db.initDBCollName("warring", "accounts", "characters");

    db.check();

    db.createIndex();

#if 0
    // add new accounts
    for (size_t i = 0; i < 10; ++i)
    {
        std::string account = "account_" + std::to_string(i) + "@email.com";
        std::string password = "password_" + std::to_string(i);
        std::string address = "192.168.1.2:123" + std::to_string(2 * i);
        auto insertAccountResult = db.addNewAccount(account, password, address);

        std::cout << "insertAccountResult: " << insertAccountResult << std::endl;
    }
#endif

#if 0
    // add new characters
    for (size_t i = 0; i < 10; ++i)
    {
        std::string account = "account_" + std::to_string(i) + "@email.com";

        schema::player::CharacterBaseInfo characterBaseInfo;
        characterBaseInfo.mName = "zhangsan_" + std::to_string(i + 25);
        characterBaseInfo.mLevel = 1;
        characterBaseInfo.mOccuption = std::to_string(i * i);
        characterBaseInfo.mState = std::to_string(i * i * i);
        auto addCharacterResult = db.addNewCharacter(account, characterBaseInfo);

        std::cout << "addCharacterResult: " << addCharacterResult << std::endl;
    }

#endif

#if 0
    // remove characters
    for (size_t i = 0; i < 5; ++i)
    {
        std::string account = "account_" + std::to_string(i) + "@email.com";
        std::string character;
        if (i % 2 == 0)
            character = "zhangsan_" + std::to_string(i + 0);
        else
            character = "zhangsan_" + std::to_string(i + 10);

        auto removeResult = db.removeCharacter(account, character);
        std::cout << removeResult << std::endl;
    }
#endif

#if 1
    for (size_t i = 0; i < 10; ++i)
    {
        std::string account = "account_" + std::to_string(i) + "@email.com";
        std::cout << account << " has " << db.getCharacterNum(account) << " characters\n";

        auto characters = db.getAllCharacterName(account);
        if (characters.has_value())
        {
            for (const auto &name : characters.value())
            {
                std::cout << name << " ";
            }
            std::cout << std::endl;
        }

        auto characterInfos = db.getAllCharacterBaseInfo(account);
        if (characterInfos.has_value())
        {
            for (const auto &info : characterInfos.value())
            {
                std::cout << info.mName << " " << info.mLevel << " " << info.mOccuption << " " << info.mState << std::endl;
            }
            std::cout << "--------------------------------------------------------------------------------------\n";
        }
    }

    for (size_t i = 0; i < 10; ++i)
    {
        std::string account = "account_" + std::to_string(i) + "@email.com";
        std::string password = "password_" + std::to_string(i * i);

        // std::cout << "matchAccountResult:" << db.matchAccount(account, password) << std::endl;
        std::cout << "matchAccountAndUpdateLoginStateResult:" << db.matchAccountAndUpdateLoginState(account, password, true) << std::endl;
    }

#endif

    auto baseInfo = db.getOneCharacterBaseInfoFill2Message("zhangsan_11@123.com", "GameManager_100");
    if (baseInfo.has_value())
    {
        std::cout << baseInfo.value().CHARACTER_NAME << std::endl;
        std::cout << baseInfo.value().CHARACTER_LEVEL << std::endl;
        std::cout << std::boolalpha << baseInfo.value().CHARACTER_GENDER << std::endl;
        std::cout << baseInfo.value().CHARACTER_OCCUPTION << std::endl;
        std::cout << baseInfo.value().CHARACTER_STATE << std::endl;
    }

    auto characterInfo = db.getOneCharacterInfoFill2Message("zhangsan_11@123.com", "GameManager_100");
    if (characterInfo.has_value())
    {
        std::cout << characterInfo.value().ACCOUNT << std::endl;
        std::cout << characterInfo.value().NAME << std::endl;
        std::cout << characterInfo.value().LEVEL << std::endl;
        std::cout << characterInfo.value().GENDER << std::endl;
        std::cout << characterInfo.value().OCCUPTION << std::endl;
        std::cout << characterInfo.value().STATE << std::endl;
        std::cout << characterInfo.value().EXP << std::endl;
        std::cout << characterInfo.value().HEALTH << std::endl;
        std::cout << characterInfo.value().MANA << std::endl;
        std::cout << characterInfo.value().ARMOR << std::endl;
        std::cout << characterInfo.value().STAMINA << std::endl;
        std::cout << characterInfo.value().STRENGTH << std::endl;
        std::cout << characterInfo.value().AGILITY << std::endl;
        std::cout << characterInfo.value().INTELLECT << std::endl;
        std::cout << characterInfo.value().SCENE_ID << std::endl;
        std::cout << characterInfo.value().SCENE_X << std::endl;
        std::cout << characterInfo.value().SCENE_Y << std::endl;
        std::cout << characterInfo.value().SCENE_Z << std::endl;
    }

    message::CharacterInfo uploadCharacter;
    uploadCharacter.ACCOUNT = "zhangsan_11@123.com";
    uploadCharacter.NAME = "GameManager_100";
    uploadCharacter.LEVEL = 10;
    uploadCharacter.GENDER = false;
    uploadCharacter.OCCUPTION = 100111;
    uploadCharacter.STATE = 100011;
    uploadCharacter.EXP = 1230;
    uploadCharacter.HEALTH = 2000;
    uploadCharacter.MANA = 1000;
    uploadCharacter.ARMOR = 500;
    uploadCharacter.STAMINA = 1500;
    uploadCharacter.STRENGTH = 1600;
    uploadCharacter.AGILITY = 1600;
    uploadCharacter.INTELLECT = 2500;
    uploadCharacter.SCENE_ID = 200001;
    uploadCharacter.SCENE_X = 5000.0;
    uploadCharacter.SCENE_Y = 4500.0;
    uploadCharacter.SCENE_Z = 90.0;

    std::cout << db.updateCharacter(uploadCharacter) << std::endl;

    return 0;
}
