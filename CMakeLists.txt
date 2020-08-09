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

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "$ENV{BENCHMARK_ROOT}")

#===== [flags] ================================================================#

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_STANDARD 17)

#===== [install] ==============================================================#

cmake_policy(SET CMP0076 NEW)
set(headers 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wrench/memory/aligned_heap_allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wrench/memory/allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wrench/memory/arena.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wrench/memory/linear_allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wrench/memory/memory_utils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wrench/memory/pool_allocator.hpp
  include/wrench/multithreading/spinlock.hpp
  include/wrench/perf/profiler.hpp
  include/wrench/utils/portability.hpp
)

set(sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/perf/profiler.cpp
)

add_library(wrench STATIC ${sources})
add_library(wrench::wrench ALIAS wrench)
#target_sources(wrench PUBLIC "<BUILD_INTERFACE:${headers}>")
target_include_directories(wrench PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_include_directories(wrench SYSTEM PUBLIC
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# Setup package config
include(CMakePackageConfigHelpers)
set(WRENCH_CMAKE_DIR "lib/cmake/wrench")
set(version_config "${CMAKE_CURRENT_BINARY_DIR}/wrench-config-version.cmake")
set(project_config "${CMAKE_CURRENT_BINARY_DIR}/wrench-config.cmake")
set(targets_export_name wrench-targets)

write_basic_package_version_file(
  ${version_config}
  VERSION       0.0.1
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/wrench-config.cmake.in"
  ${project_config}
  INSTALL_DESTINATION ${WRENCH_CMAKE_DIR}
)

# Export targets for non-install use:
export(
  TARGETS   wrench 
  NAMESPACE wrench:: 
  FILE      ${CMAKE_CURRENT_BINARY_DIR}/${targets_export_name}.cmake
)

# Install version:
install(
  FILES ${project_config} ${version_config} DESTINATION ${WRENCH_CMAKE_DIR}
)

# Install target:
install(TARGETS wrench DESTINATION lib EXPORT ${targets_export_name})
install(
  EXPORT      ${targets_export_name}
  DESTINATION lib
  NAMESPACE   wrench::
)
install(DIRECTORY include/wrench DESTINATION include)

#===== [subdirs] ==============================================================#

# Only add these if not inside add_subdirectory()

option(WRENCH_BUILD_BENCHMARK "build benchmark" OFF)
if(${WRENCH_BUILD_BENCHMARK} OR 
  (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    add_subdirectory(benchmark)
endif()

option(WRENCH_BUILD_TESTS "build tests" OFF)
if(${WRENCH_BUILD_TESTS} OR 
  (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    add_subdirectory(tests)
endif()

option(WRENCH_BUILD_DOCS "build docs" OFF)
if(WRENCH_BUILD_DOCS)
    add_subdirectory(docs)
endif()

#==============================================================================#