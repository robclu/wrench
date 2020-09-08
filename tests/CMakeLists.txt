#==--- wrench/tests/CMakeLists.txt ------------------------------------------==#
#
#                      Copyright (c) 2020 Rob Clucas
#
#  This file is distributed under the MIT License. See LICENSE for details.
#
#==--------------------------------------------------------------------------==#

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(all_tests ${CMAKE_CURRENT_SOURCE_DIR}/all.cpp)
target_link_libraries(all_tests gtest_main)

add_executable(algorithm_tests ${CMAKE_CURRENT_SOURCE_DIR}/algorithm.cpp)
target_link_libraries(algorithm_tests gtest_main)

add_executable(memory_tests ${CMAKE_CURRENT_SOURCE_DIR}/memory.cpp)
target_link_libraries(memory_tests gtest_main)


add_executable(utils_tests ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp)
target_link_libraries(utils_tests gtest_main)