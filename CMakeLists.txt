cmake_minimum_required(VERSION 2.8.11)
project(executors)
add_definitions(-DEXECUTORS_LIBRARY)
file(GLOB_RECURSE DATABASE_SRCS
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(executors STATIC ${DATABASE_SRCS})
set_property(TARGET executors PROPERTY CXX_STANDARD 14)


