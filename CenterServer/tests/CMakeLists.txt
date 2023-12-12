cmake_minimum_required(VERSION 3.15)

project(testCenterServer)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(CENTER_SERVER_PATH ${PROJECT_PATH}/CenterServer)

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


file(GLOB MESSAGES_SRC ${BASE_MESSAGE_PATH}/BaseMessage.cpp
                       ${BASE_MESSAGE_PATH}/CustomMessages.cpp)

file(GLOB BROKER_SRC ${BROKER_PATH}/RSBrokerManager.cpp)



file(GLOB CENTER_SERVER_CONF_SRC ${CENTER_SERVER_PATH}/CenterServConf.cpp)





include_directories(${PROJECT_PATH})

include_directories(${BASE_PATH})

include_directories(${CENTER_SERVER_PATH})

include_directories(${BROKER_PATH})


set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")


add_executable(testServiceRegistry testServiceRegistry.cpp
                                ${BROKER_SRC}
                                ${TIMER_SRC}
                                ${LOGGER_SRC}
                                ${CENTER_SERVER_CONF_SRC})
                                

target_link_libraries(testServiceRegistry hiredis redis++ pthread)




add_executable(testService testService.cpp
                            ${BROKER_SRC}
                            ${TIMER_SRC}
                            ${LOGGER_SRC})

target_link_libraries(testService hiredis redis++ pthread)
