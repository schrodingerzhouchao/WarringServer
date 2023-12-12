cmake_minimum_required(VERSION 3.15)

project(testGatewayServer)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(GATEWAY_SERVER_PATH ${PROJECT_PATH}/GatewayServer)

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


file(GLOB GATEWAY_SERVER_SRC ${GATEWAY_SERVER_PATH}/GatewayServer.cpp)

file(GLOB GATEWAY_SERVER_CONF_SRC ${GATEWAY_SERVER_PATH}/GatewayServConf.cpp)

file(GLOB GATEWAY_SERVER_MESSAGE_SRC ${BASE_MESSAGE_PATH}/GatewayMessage.cpp)

file(GLOB GATEWAY_SERVER_MESSAGE_OPERATION_SRC ${GATEWAY_SERVER_PATH}/GatewayMessageOp.cpp)

file(GLOB GATEWAY_SERVER_TIMEOUT_TASKS_SRC ${GATEWAY_SERVER_PATH}/GatewayTimeoutTasks.cpp)

file(GLOB BROKER_SRC ${BROKER_PATH}/RSBrokerManager.cpp)

include_directories(${PROJECT_PATH})

include_directories(${BASE_PATH})

include_directories(${GATEWAY_SERVER_PATH})

include_directories(${BROKER_PATH})


set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")


add_executable(testGatewayServer testGatewayServer.cpp
                                ${BROKER_SRC}
                                ${GATEWAY_SERVER_SRC}
                                ${GATEWAY_SERVER_CONF_SRC}
                                ${GATEWAY_SERVER_MESSAGE_SRC}
                                ${GATEWAY_SERVER_MESSAGE_OPERATION_SRC}
                                ${GATEWAY_SERVER_TIMEOUT_TASKS_SRC}
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${TCP_SERVER_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testGatewayServer hiredis redis++ pthread)


add_executable(testGatewayClient testGatewayClient.cpp
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${TCP_SERVER_SRC}
                                ${GATEWAY_SERVER_MESSAGE_SRC}
                                ${MESSAGES_SRC})

target_link_libraries(testGatewayClient pthread)




