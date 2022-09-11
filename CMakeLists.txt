cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)

set(ServerName Server)
set(ClientName Client)
set(ServerTestName runServerTests)

enable_testing()

project(ClientServer)

find_package(nlohmann_json 3.10.5 REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.70.0 REQUIRED COMPONENTS system regex date_time)

#Set libraries and includes needed everywhere
set(GlobalProjectLibs nlohmann_json::nlohmann_json  ${Boost_LIBRARIES})
set(GlobalProjectIncludes ${Boost_INCLUDE_DIRS})

add_subdirectory(Server)
add_subdirectory(Client)
add_subdirectory(ServerTests)