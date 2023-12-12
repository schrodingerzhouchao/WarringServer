cmake_minimum_required(VERSION 3.15)

project(testDB)

set(PROJECT_PATH /home/zc/cppwork/WarringServer)

set(DB_SERVER_PATH ${PROJECT_PATH}/DBServer)

set(DB_SERVER_DB_PATH ${DB_SERVER_PATH}/db)

set(DB_SERVER_CACHE_PATH ${DB_SERVER_PATH}/cache)

set(BASE_PATH ${PROJECT_PATH}/Base)

set(BASE_MESSAGE_PATH ${BASE_PATH}/message)

set(BASE_UTILITY_PATH ${BASE_PATH}/utility)

set(BROKER_PATH ${PROJECT_PATH}/Broker)

set(GAME_DATA_PATH &{PROJECT_PATH}/GameDataTable)

file(GLOB MESSAGES_SRC ${BASE_MESSAGE_PATH}/BaseMessage.cpp
                       ${BASE_MESSAGE_PATH}/CustomMessages.cpp
                       ${BASE_MESSAGE_PATH}/LoginMessage.cpp
                       ${BASE_MESSAGE_PATH}/GatewayMessage.cpp
                       ${BASE_MESSAGE_PATH}/GameMessage.cpp)

                     

file(GLOB TIMER_SRC ${BASE_UTILITY_PATH}/Timer.cpp)

file(GLOB LOGGER_SRC ${BASE_UTILITY_PATH}/LoggerFile.cpp 
                        ${BASE_UTILITY_PATH}/LoggerLine.cpp
                        ${BASE_UTILITY_PATH}/LoggerMessage.cpp
                        ${BASE_UTILITY_PATH}/Logger.cpp)
                                                                   
file(GLOB BROKER_SRC ${BROKER_PATH}/RSBrokerManager.cpp)

file(GLOB DB_SERVER_GATEWAY_SRC ${DB_SERVER_PATH}/DBServGateway.cpp)

file(GLOB DB_SERVER_CONF_SRC ${DB_SERVER_PATH}/DBServConf.cpp)

file(GLOB DB_SERVER_QUERY_TASK_SRC ${DB_SERVER_PATH}/QueryTask.cpp)

file(GLOB DB_SERVER_DB_SRC ${DB_SERVER_DB_PATH}/Database.cpp)

file(GLOB DB_SERVER_CACHE_SRC ${DB_SERVER_CACHE_PATH}/RCache.cpp)

file(GLOB DB_SERVER_DB_DOCUMENT_SRC ${DB_SERVER_DB_PATH}/Document.cpp)

file(GLOB DB_SERVER_DB_ACCOUNT_OP_SRC ${DB_SERVER_DB_PATH}/AccountOp.cpp)

file(GLOB DB_SERVER_DB_CHARACTER_OP_SRC ${DB_SERVER_DB_PATH}/CharacterOp.cpp)

include_directories(${PROJECT_PATH})

include_directories(${DB_SERVER_PATH})

include_directories(${BASE_PATH})

include_directories(${GAME_DATA_PATH})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")

add_executable(testRedis testRedis.cpp)

target_link_libraries(testRedis hiredis redis++ pthread)


add_executable(testMongodb testMongoDB.cpp
                            ${TIMER_SRC}
                            ${LOGGER_SRC}
                            ${DB_SERVER_DB_SRC}
                            ${DB_SERVER_DB_DOCUMENT_SRC}
                            ${DB_SERVER_DB_ACCOUNT_OP_SRC}
                            ${DB_SERVER_DB_CHARACTER_OP_SRC}
                            ${MESSAGES_SRC})

target_link_libraries(testMongodb mongocxx bsoncxx)

add_executable(testDBServGateway testDBServGateway.cpp
                                    ${MESSAGES_SRC}
                                    ${TIMER_SRC}
                                    ${LOGGER_SRC}
                                    ${BROKER_SRC}
                                    ${DB_SERVER_GATEWAY_SRC}
                                    ${DB_SERVER_CONF_SRC}
                                    ${DB_SERVER_DB_SRC}
                                    ${DB_SERVER_CACHE_SRC}
                                    ${DB_SERVER_DB_DOCUMENT_SRC}
                                    ${DB_SERVER_DB_ACCOUNT_OP_SRC}
                                    ${DB_SERVER_DB_CHARACTER_OP_SRC}
                                    ${DB_SERVER_QUERY_TASK_SRC})

target_link_libraries(testDBServGateway hiredis redis++ mongocxx bsoncxx pthread)