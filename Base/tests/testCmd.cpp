#include "utility/Cmd.h"
#include <iostream>

int main()
{
    utility::Cmd cmd;

    auto result = cmd("ls");

    std::cout << result << std::endl;

    result = cmd("ifconfig ens33 | awk 'NR==2{print $2}'");

    std::cout << result << std::endl;
}