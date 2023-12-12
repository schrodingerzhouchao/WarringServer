#ifndef __REDIS_SERVICE_REGISTRY_MESSAGE_H__
#define __REDIS_SERVICE_REGISTRY_MESSAGE_H__

#include <string>
#include <vector>
#include <cassert>

namespace center_server
{
    namespace registry
    {

        struct RegHeartbeatMessage
        {
            size_t LOAD_NUM = 0;

            void FILL_2_VEC(std::vector<std::pair<std::string, std::string>> &destVector) const
            {
                destVector.clear();
                destVector.emplace_back(std::make_pair("LOAD_NUM", std::to_string(LOAD_NUM)));
            }

            void FILL_2_THIS(const std::vector<std::pair<std::string, std::string>> &sourceVector)
            {
                assert(TAG_NUM == sourceVector.size());

                size_t i = 0;
                LOAD_NUM = std::stoul(sourceVector.at(i).second);
            }

        private:
            const size_t TAG_NUM = 1;
        };
    }
}

#endif