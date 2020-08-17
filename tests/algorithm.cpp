//==--- wrench/tests/algorithm.cpp ------------------------- -*- C++ -*- ---==//
//
//                                  Wrench
//
//                      Copyright (c) 2020 Rob Clucas.
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  algorithm.cpp
/// \brief This file implements tests for algorithm functionality.
//
//==------------------------------------------------------------------------==//

#include "algorithm/unrolled_for_tests.hpp"

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}