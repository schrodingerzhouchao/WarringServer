#ifndef __UTILITY_CMD_H__
#define __UTILITY_CMD_H__

#include <cstdio>
#include <cstring>
#include <string>

namespace utility
{
    class Cmd
    {
    public:
        ~Cmd()
        {
            pclose(mFP);
        }

        std::string operator()(const std::string &cmd)
        {
            memset(mBuff, 0, 1024);
            mFP = popen(cmd.data(), "r");
            fread(mBuff, 1, 1024, mFP);

            std::string result(mBuff);
            result.pop_back();

            return result;
        }

    private:
        FILE *mFP = nullptr;
        char mBuff[1024] = {'\0'};

        std::string mdata;
    };

}

#endif