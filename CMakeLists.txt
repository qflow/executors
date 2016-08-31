cmake_minimum_required(VERSION 2.8.11)
include(GenerateExportHeader)
project(executors)
option(EXECUTORS_BUILD_TESTS "Build executors tests." ON)
if(EXECUTORS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
add_definitions(-DEXECUTORS_LIBRARY)
file(GLOB DATABASE_SRCS
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/*.h
         ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/util/*.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_library(executors STATIC ${DATABASE_SRCS})
generate_export_header(executors)
target_include_directories(executors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(executors pthread)
endif()
set_property(TARGET executors PROPERTY CXX_STANDARD 14)


