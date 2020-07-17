#==--- wrench/CMakeLists.txt ------------------------------------------------==#
#
#                      Copyright (c) 2020 Rob Clucas
#
#  This file is distributed under the MIT License. See LICENSE for details.
#
#==--------------------------------------------------------------------------==#

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(wrench)

# Allow clang-format to configure from the build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#===== [packages] =============================================================#

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

#===== [subdirs] ==============================================================#

add_subdirectory(docs)

#==============================================================================#