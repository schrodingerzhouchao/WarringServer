cmake_minimum_required(VERSION 3.15)

project(testLoginServer)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(LOGIN_SERVER_PATH ${PROJECT_PATH}/LoginServer)

set(BASE_PATH ${PROJECT_PATH}/Base)

set(BASE_UTILITY_PATH ${PROJECT_PATH}/Base/utility)

set(BASE_THREAD_PATH ${PROJECT_PATH}/Base/thread)

set(BASE_NET_PATH ${PROJECT_PATH}/Base/net)

set(BASE_MESSAGE_PATH ${PROJECT_PATH}/Base/message)

set(BROKER_PATH ${PROJECT_PATH}/Broker)

set(GAME_DATA_PATH &{PROJECT_PATH}/GameDataTable)

file(GLOB TIMER_SRC ${BASE_UTILITY_PATH}/Timer.cpp)

file(GLOB LOGGER_SRC ${BASE_UTILITY_PATH}/LoggerFile.cpp 
                     ${BASE_UTILITY_PATH}/LoggerLine.cpp
                     ${BASE_UTILITY_PATH}/LoggerMessage.cpp
                     ${BASE_UTILITY_PATH}/Logger.cpp)

file(GLOB TCP_SERVER_SRC ${BASE_NET_PATH}/Socket.cpp)

file(GLOB MESSAGES_SRC ${BASE_MESSAGE_PATH}/BaseMessage.cpp
                       ${BASE_MESSAGE_PATH}/CustomMessages.cpp)


file(GLOB LOGIN_SERVER_SRC ${LOGIN_SERVER_PATH}/LoginServer.cpp)

file(GLOB LOGIN_SERVER_CONF_SRC ${LOGIN_SERVER_PATH}/LoginServConf.cpp)

file(GLOB LOGIN_SERVER_MESSAGE_SRC ${BASE_MESSAGE_PATH}/LoginMessage.cpp)

file(GLOB LOGIN_SERVER_MESSAGE_OPERATION_SRC ${LOGIN_SERVER_PATH}/LoginMessageOp.cpp)

file(GLOB LOGIN_SERVER_TIMEOUT_TASKS_SRC ${LOGIN_SERVER_PATH}/LoginTimeoutTasks.cpp)

file(GLOB BROKER_SRC ${BROKER_PATH}/RSBrokerManager.cpp)

include_directories(${PROJECT_PATH})

include_directories(${BASE_PATH})

include_directories(${LOGIN_SERVER_PATH})

include_directories(${BROKER_PATH})

include_directories(${GAME_DATA_PATH})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")


add_executable(testLoginServer testLoginServer.cpp
                                ${BROKER_SRC}
                                ${LOGIN_SERVER_SRC}
                                ${LOGIN_SERVER_CONF_SRC}
                                ${LOGIN_SERVER_MESSAGE_SRC}
                                ${LOGIN_SERVER_MESSAGE_OPERATION_SRC}
                                ${LOGIN_SERVER_TIMEOUT_TASKS_SRC}
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${TCP_SERVER_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testLoginServer hiredis redis++ pthread)


add_executable(testLoginClient testLoginClient.cpp
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${TCP_SERVER_SRC}
                                ${LOGIN_SERVER_MESSAGE_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testLoginClient pthread)

add_executable(testFakeDBServ testFakeDBServ.cpp
                                ${BROKER_SRC}
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${LOGIN_SERVER_MESSAGE_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testFakeDBServ hiredis redis++ pthread)

