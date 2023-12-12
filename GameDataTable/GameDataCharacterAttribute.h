#ifndef __GAME_DATA_CHARACTER_ATTRIBUTE_H__
#define __GAME_DATA_CHARACTER_ATTRIBUTE_H__

#include "GameDataCode.h"
#include <unordered_map>
#include <tuple>

namespace game
{
    class NewerCharacterInit
    {
    public:
        static std::tuple<int, double, double, double> newerSpawnLocation(int state, int occuption)
        {
            switch (state)
            {
            case code::state::GAME_DATA_CODE_STATE_QI:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_QI_1, 2000.0, 2000.0, 90.0);
            case code::state::GAME_DATA_CODE_STATE_CHU:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_CHU_1, 2000.0, 2000.0, 90.0);
            case code::state::GAME_DATA_CODE_STATE_YAN:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_YAN_1, 2000.0, 2000.0, 90.0);
            case code::state::GAME_DATA_CODE_STATE_QIN:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_QIN_1, 2000.0, 2000.0, 90.0);
            case code::state::GAME_DATA_CODE_STATE_ZHAO:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_ZHAO_1, 2000.0, 2000.0, 90.0);
            case code::state::GAME_DATA_CODE_STATE_WEI:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_WEI_1, 2000.0, 2000.0, 90.0);
            case code::state::GAME_DATA_CODE_STATE_HAN:
                return std::make_tuple(code::scene::GAME_DATA_SCENE_HAN_1, 2000.0, 2000.0, 90.0);

            default:
                return std::make_tuple(-1, 0, 0, 0);
            }
        }

        // TODO
        // <attribute_name, value>
        static std::unordered_map<std::string, int> newerAttribute(int state, int occuption)
        {
            // TODO state and occuption's specificity

            return {{field_name::FIELD_EXP, 0},
                    {field_name::FIELD_HEALTH, 100},
                    {field_name::FIELD_MANA, 50},
                    {field_name::FIELD_ARMOR, 10},
                    {field_name::FIELD_STAMINA, 100},
                    {field_name::FIELD_STRENGTH, 10},
                    {field_name::FIELD_AGILITY, 10},
                    {field_name::FIELD_INTELLECT, 10}};
        }

        // TODO
        // <skill_id, level>
        static std::unordered_map<int, int> newerSkill(int state, int occuption)
        {
            return {};
        }

        // TODO more, backpack, equipment, quest, ...
    };
}

#endif