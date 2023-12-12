cmake_minimum_required(VERSION 3.15)

project(Test)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(BASE_PATH ${PROJECT_PATH}/Base)

set(BASE_UTILITY_PATH ${PROJECT_PATH}/Base/utility)

set(BASE_THREAD_PATH ${PROJECT_PATH}/Base/thread)

set(BASE_NET_PATH ${PROJECT_PATH}/Base/net)

set(BASE_MESSAGE_PATH ${PROJECT_PATH}/Base/message)

file(GLOB TIMER_SRC ${BASE_UTILITY_PATH}/Timer.cpp)

file(GLOB LOGGER_SRC ${BASE_UTILITY_PATH}/LoggerFile.cpp 
                     ${BASE_UTILITY_PATH}/LoggerLine.cpp
                     ${BASE_UTILITY_PATH}/LoggerMessage.cpp
                     ${BASE_UTILITY_PATH}/Logger.cpp)

file(GLOB TCP_SERVER_SRC ${BASE_NET_PATH}/Socket.cpp)

file(GLOB MESSAGES_SRC ${BASE_MESSAGE_PATH}/BaseMessage.cpp
                       ${BASE_MESSAGE_PATH}/CustomMessages.cpp
                       ${BASE_MESSAGE_PATH}/LoginMessage.cpp)

include_directories(${BASE_PATH})

#include_directories(${BASE_UTILITY_PATH} ${BASE_THREAD_PATH} ${BASE_NET_PATH})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")

# test for Time
add_executable(testTime testTime.cpp)

target_link_libraries(testTime pthread)

# test for Timer
add_executable(testTimer testTimer.cpp ${TIMER_SRC} ${LOGGER_SRC})

target_link_libraries(testTimer pthread)

# test for Logger
add_executable(testLogger testLogger.cpp ${LOGGER_SRC} ${TIMER_SRC})

target_link_libraries(testLogger pthread)


# test for RThread
add_executable(testRThread testRThread.cpp)

target_link_libraries(testRThread pthread)

# test for tcp server
add_executable(testTcpServer testTcpServer.cpp ${LOGGER_SRC} ${TIMER_SRC} ${TCP_SERVER_SRC} ${MESSAGES_SRC})

target_link_libraries(testTcpServer pthread hiredis redis++)

# test for tcp client
add_executable(testTcpClient testTcpClient.cpp ${LOGGER_SRC} ${TIMER_SRC} ${TCP_SERVER_SRC} ${MESSAGES_SRC})

target_link_libraries(testTcpClient pthread)

# test for message
add_executable(testMessage testMessage.cpp ${MESSAGES_SRC})

# test for safeHash
add_executable(testSafeHasp testSafeHash.cpp)

target_link_libraries(testSafeHasp pthread)

add_executable(testCmd testCmd.cpp)