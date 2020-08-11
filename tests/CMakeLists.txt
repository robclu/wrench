#==--- wrench/tests/CMakeLists.txt ------------------------------------------==#
#
#                      Copyright (c) 2020 Rob Clucas
#
#  This file is distributed under the MIT License. See LICENSE for details.
#
#==--------------------------------------------------------------------------==#

include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(
  memory_tests ${CMAKE_CURRENT_SOURCE_DIR}/memory.cpp
)
target_link_libraries(memory_tests gtest_main)