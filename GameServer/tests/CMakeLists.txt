cmake_minimum_required(VERSION 3.15)

project(testGameServer)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(GAME_SERVER_PATH ${PROJECT_PATH}/GameServer)

set(BASE_PATH ${PROJECT_PATH}/Base)

set(BASE_UTILITY_PATH ${PROJECT_PATH}/Base/utility)

set(BASE_THREAD_PATH ${PROJECT_PATH}/Base/thread)

set(BASE_NET_PATH ${PROJECT_PATH}/Base/net)

set(BASE_MESSAGE_PATH ${PROJECT_PATH}/Base/message)

set(BROKER_PATH ${PROJECT_PATH}/Broker)

file(GLOB TIMER_SRC ${BASE_UTILITY_PATH}/Timer.cpp)

file(GLOB LOGGER_SRC ${BASE_UTILITY_PATH}/LoggerFile.cpp 
                     ${BASE_UTILITY_PATH}/LoggerLine.cpp
                     ${BASE_UTILITY_PATH}/LoggerMessage.cpp
                     ${BASE_UTILITY_PATH}/Logger.cpp)

file(GLOB TCP_SERVER_SRC ${BASE_NET_PATH}/Socket.cpp)

file(GLOB MESSAGES_SRC ${BASE_MESSAGE_PATH}/BaseMessage.cpp
                       ${BASE_MESSAGE_PATH}/CustomMessages.cpp)


file(GLOB GAME_SERVER_SRC ${GAME_SERVER_PATH}/GameServer.cpp)

file(GLOB GAME_SERVER_CONF_SRC ${GAME_SERVER_PATH}/GameServConf.cpp)

file(GLOB GAME_SERVER_MESSAGE_SRC ${BASE_MESSAGE_PATH}/GameMessage.cpp)

file(GLOB GAME_SERVER_MESSAGE_OPERATION_SRC ${GAME_SERVER_PATH}/GameMessageOp.cpp)

file(GLOB GAME_SERVER_TIMEOUT_TASKS_SRC ${GAME_SERVER_PATH}/GameTimeoutTasks.cpp)

file(GLOB BROKER_SRC ${BROKER_PATH}/RSBrokerManager.cpp)

include_directories(${PROJECT_PATH})

include_directories(${BASE_PATH})

include_directories(${GAME_SERVER_PATH})

include_directories(${BROKER_PATH})


set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")


add_executable(testGameServer testGameServer.cpp
                                ${BROKER_SRC}
                                ${GAME_SERVER_SRC}
                                ${GAME_SERVER_CONF_SRC}
                                ${GAME_SERVER_MESSAGE_SRC}
                                ${GAME_SERVER_MESSAGE_OPERATION_SRC}
                                ${GAME_SERVER_TIMEOUT_TASKS_SRC}
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${TCP_SERVER_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testGameServer hiredis redis++ pthread)


add_executable(testGameClient testGameClient.cpp
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${TCP_SERVER_SRC}
                                ${GAME_SERVER_MESSAGE_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testGameClient pthread)




