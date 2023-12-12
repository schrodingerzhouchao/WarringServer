#ifndef __GAME_SERVER_SCENE_MANAGER_H__
#define __GAME_SERVER_SCENE_MANAGER_H__

#include "Broker/RSBrokerHelper.h"
#include <unordered_map>
#include <string>

namespace game_server
{
    namespace game_scene
    {
        class SceneManager
        {
        public:
            SceneManager(sw::redis::Redis &servRegRedis) : mServRegRedis(servRegRedis) {}

            int registerScene(const std::string &ip, const std::string &sceneKey,int dsPort, int load)
            {
                auto address = ip + ":" + std::to_string(dsPort); 

                if (mSceneAddresses.emplace(address, sceneKey).second)
                {
                    return broker::RSBrokerHelper::sortedAdd(mServRegRedis, sceneKey, address, load);
                }
                return -1;
            }

            int removeScene(const std::string &address)
            {
                auto it = mSceneAddresses.find(address);
                if (it == mSceneAddresses.end())
                    return -1;

                auto sceneKey = it->second;
                mSceneAddresses.erase(address);
                return broker::RSBrokerHelper::sortedRemove(mServRegRedis, sceneKey, address);
            }

            int updateScene(const std::string &address, const std::string &sceneKey, int load)
            {
                return broker::RSBrokerHelper::sortedAdd(mServRegRedis, sceneKey, address, load);
            }

            // return low load ds scend address
            std::string getLowLoadScene(const std::string &sceneKey)
            {
                std::vector<std::pair<std::string, double>> result;
                mServRegRedis.zrangebyscore(sceneKey, sw::redis::UnboundedInterval<double>{}, std::back_inserter(result));
                if (!result.empty())
                {
                    return result.at(0).first;
                }
                return {};
            }

            std::string generateSceneIDKey(const std::string &keyPrefix, int sceneID)
            {
                return keyPrefix + std::to_string(sceneID);
            }

        private:
            sw::redis::Redis &mServRegRedis;
            // <ds scene address, ds scene key>
            std::unordered_map<std::string, std::string> mSceneAddresses;
        };

    }
}

#endif