#ifndef __GAME_DATA_CODES_H__
#define __GAME_DATA_CODES_H__

#include <string>

namespace game
{
    namespace code
    {
        namespace state
        {
            const int GAME_DATA_CODE_STATE_QI = 100011;
            const int GAME_DATA_CODE_STATE_CHU = 100012;
            const int GAME_DATA_CODE_STATE_YAN = 100013;
            const int GAME_DATA_CODE_STATE_QIN = 100014;
            const int GAME_DATA_CODE_STATE_ZHAO = 100015;
            const int GAME_DATA_CODE_STATE_WEI = 100016;
            const int GAME_DATA_CODE_STATE_HAN = 100017;
        }
        namespace occuption
        {
            const int GAME_DATA_CODE_OCCUPTION_FARMER = 100111;
            const int GAME_DATA_CODE_OCCUPTION_HANDICRAFTSMAN = 100112;
            const int GAME_DATA_CODE_OCCUPTION_HUNTER = 100113;
            const int GAME_DATA_CODE_OCCUPTION_EXPLORER = 100114;
            const int GAME_DATA_CODE_OCCUPTION_SOLDIER = 100115;
            const int GAME_DATA_CODE_OCCUPTION_VENDOR = 100116;
            const int GAME_DATA_CODE_OCCUPTION_SCHOLAR = 100117;
            const int GAME_DATA_CODE_OCCUPTION_DOCTOR = 100118;
            const int GAME_DATA_CODE_OCCUPTION_WIZARD = 100119;
        }
        namespace scene
        {
            const int GAME_DATA_SCENE_QI_1 = 200001;
            const int GAME_DATA_SCENE_QI_2 = 200002;
            const int GAME_DATA_SCENE_QI_3 = 200003;
            const int GAME_DATA_SCENE_QI_4 = 200004;
            const int GAME_DATA_SCENE_QI_5 = 200005;
            const int GAME_DATA_SCENE_QI_6 = 200006;
            const int GAME_DATA_SCENE_QI_7 = 200007;
            const int GAME_DATA_SCENE_QI_8 = 200008;
            const int GAME_DATA_SCENE_QI_9 = 200009;
            const int GAME_DATA_SCENE_QI_10 = 200010;

            const int GAME_DATA_SCENE_CHU_1 = 200101;
            const int GAME_DATA_SCENE_CHU_2 = 200102;
            const int GAME_DATA_SCENE_CHU_3 = 200103;
            const int GAME_DATA_SCENE_CHU_4 = 200104;
            const int GAME_DATA_SCENE_CHU_5 = 200105;
            const int GAME_DATA_SCENE_CHU_6 = 200106;
            const int GAME_DATA_SCENE_CHU_7 = 200107;
            const int GAME_DATA_SCENE_CHU_8 = 200108;
            const int GAME_DATA_SCENE_CHU_9 = 200109;
            const int GAME_DATA_SCENE_CHU_10 = 200110;

            const int GAME_DATA_SCENE_YAN_1 = 200201;
            const int GAME_DATA_SCENE_YAN_2 = 200202;

            const int GAME_DATA_SCENE_QIN_1 = 200301;
            const int GAME_DATA_SCENE_QIN_2 = 200302;

            const int GAME_DATA_SCENE_ZHAO_1 = 200401;
            const int GAME_DATA_SCENE_ZHAO_2 = 200402;

            const int GAME_DATA_SCENE_WEI_1 = 200501;
            const int GAME_DATA_SCENE_WEI_2 = 200502;

            const int GAME_DATA_SCENE_HAN_1 = 200601;
            const int GAME_DATA_SCENE_HAN_2 = 200602;

            // TODO more
        }
        namespace xx
        {
            // TODO more
        }

    }

    namespace field_name
    {
        const std::string FIELD_ACCOUNT = "account";
        const std::string FIELD_PASSWORD = "password";

        const std::string FIELD_CHARACTER_BASE_INFO = "base_info";

        const std::string FIELD_CHARACTER_NAME = "name";
        const std::string FIELD_CHARACTER_LEVEL = "level";
        const std::string FIELD_CHARACTER_GENDER = "gender";
        const std::string FIELD_CHARACTER_OCCUPTION = "occuption";
        const std::string FIELD_CHARACTER_STATE = "state";

        const std::string FIELD_ATTRIBUTE = "attribute";

        const std::string FIELD_EXP = "exp";
        const std::string FIELD_HEALTH = "health";
        const std::string FIELD_MANA = "mana";
        const std::string FIELD_ARMOR = "armor";
        const std::string FIELD_STAMINA = "stamina";
        const std::string FIELD_STRENGTH = "strength";
        const std::string FIELD_AGILITY = "agility";
        const std::string FIELD_INTELLECT = "intellect";

        const std::string FIELD_LOCATION = "location";

        const std::string FIELD_SCENE_ID = "scene_id";
        const std::string FIELD_SCENE_X = "scene_x";
        const std::string FIELD_SCENE_Y = "scene_y";
        const std::string FIELD_SCENE_Z = "scene_z";

        const std::string FIELD_CHARACTER_DATA = "character_data";

    }
}

#endif
