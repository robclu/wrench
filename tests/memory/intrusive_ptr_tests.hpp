//==--- wrench/tests/memory/intrusive_ptr_tests.hpp -------- -*- C++ -*- ---==//
//
//                                 Wrench
//
//                      Copyright (c) 2020 Rob Clucas.
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  intrusive_ptr_tests.hpp
/// \brief This file implements tests for intrusive ptr.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_TESTS_MEMORY_INTRUSIVE_PTR_TESTS_HPP
#define WRENCH_TESTS_MEMORY_INTRUSIVE_PTR_TESTS_HPP

#include <wrench/memory/intrusive_ptr.hpp>
#include <gtest/gtest.h>

struct PtrTest : public wrench::IntrusivePtrEnabled<PtrTest> {
  int x = 0;
};

TEST(memory_intrusive_ptr, can_make_intrusive_ptr) {
  auto p = wrench::make_intrusive_ptr<PtrTest>();
  EXPECT_EQ(p->x, 0);
}

#endif // WRENCH_TESTS_MEMORY_INTRUSIVE_PTR_TESTS_HPP