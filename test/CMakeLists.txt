#project(test)
#cmake_minimum_required(VERSION 2.8)
#file(GLOB_RECURSE DATABASE_SRCS
#         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#         ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
#         ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
#add_executable(test ${DATABASE_SRCS})
#add_dependencies(test executors)
#target_link_libraries(test executors)
#set_property(TARGET test PROPERTY CXX_STANDARD 14)

add_executable(test_all all.cpp)
add_dependencies(test_all executors)
target_link_libraries(test_all executors)
add_test(test.all test_all)
set_property(TARGET test_all PROPERTY CXX_STANDARD 14)
