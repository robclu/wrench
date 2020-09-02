//==--- wrench/tests/all.cpp ------------------------------- -*- C++ -*- ---==//
//
//                                  Wrench
//
//                      Copyright (c) 2020 Rob Clucas.
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  all.cpp
/// \brief This file implements all tests.
//
//==------------------------------------------------------------------------==//

#include "algorithm/algorithm.hpp"
#include "memory/memory.hpp"

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}