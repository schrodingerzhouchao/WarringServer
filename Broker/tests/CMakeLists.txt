cmake_minimum_required(VERSION 3.15)

project(testRedisStream)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(BROKER_PATH ${PROJECT_PATH}/Broker)

set(BASE_PATH ${PROJECT_PATH}/Base)

set(BASE_UTILITY_PATH ${BASE_PATH}/utility)

set(BASE_MESSAGE_PATH ${BASE_PATH}/message)

file(GLOB BROKER_SRC ${BROKER_PATH}/RSBrokerManager.cpp)


file(GLOB LOGGER_HEADER ${BASE_UTILITY_PATH}/LoggerFile.h
                        ${BASE_UTILITY_PATH}/LoggerLine.h
                        ${BASE_UTILITY_PATH}/LoggerMessage.h
                        ${BASE_UTILITY_PATH}/Logger.h
                        ${BASE_UTILITY_PATH}/LoggerBuffer.h)


file(GLOB LOGGER_SRC ${BASE_UTILITY_PATH}/LoggerFile.cpp
                     ${BASE_UTILITY_PATH}/LoggerLine.cpp
                     ${BASE_UTILITY_PATH}/LoggerMessage.cpp
                     ${BASE_UTILITY_PATH}/Logger.cpp)

file(GLOB MESSAGES_SRC ${BASE_MESSAGE_PATH}/BaseMessage.cpp
                       ${BASE_MESSAGE_PATH}/CustomMessages.cpp)

file(GLOB TIMER_SRC ${BASE_UTILITY_PATH}/Timer.cpp)


include_directories(${PROJECT_PATH})

include_directories(${BROKER_PATH})

include_directories(${BASE_PATH})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")


add_library(logger SHARED ${LOGGER_HEADER} ${LOGGER_SRC})

add_executable(testProducer testProducer.cpp
                            #${LOGGER_SRC}
                            ${TIMER_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testProducer hiredis redis++ pthread logger)

add_executable(testConsumer testConsumer.cpp
                            #${LOGGER_SRC}
                            ${TIMER_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testConsumer hiredis redis++ pthread logger)

add_executable(testManager testManager.cpp
                            ${BROKER_SRC}
                            #${LOGGER_SRC}
                            ${TIMER_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testManager hiredis redis++ pthread logger)

add_executable(testHelper testHelper.cpp
                            ${BROKER_SRC}
                            #${LOGGER_SRC}
                            ${TIMER_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testHelper hiredis redis++ pthread logger)

add_executable(testCenter testCenter.cpp
                            ${BROKER_SRC}
                            #${LOGGER_SRC}
                            ${TIMER_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testCenter hiredis redis++ pthread logger)

add_executable(testService testService.cpp
                            ${BROKER_SRC}
                            #${LOGGER_SRC}
                            ${TIMER_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testService hiredis redis++ pthread logger)