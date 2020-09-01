//==--- wrench/tests/memory.cpp ---------------------------- -*- C++ -*- ---==//
//
//                                  Wrench
//
//                      Copyright (c) 2020 Rob Clucas.
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  memory.cpp
/// \brief This file implements tests for memory functionality.
//
//==------------------------------------------------------------------------==//

#include "memory/intrusive_ptr.hpp"
#include "memory/unique_ptr.hpp"

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}