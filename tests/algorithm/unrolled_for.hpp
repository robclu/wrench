//==--- wrench/tests/algorithm/unrolled_for.hpp ------------ -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  unrolled_for.hpp
/// \brief This file defines tests for unrolled_for.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_TESTS_ALGORITHM_UNROLLED_FOR_HPP
#define WRENCH_TESTS_ALGORITHM_UNROLLED_FOR_HPP

#include <wrench/algorithm/unrolled_for.hpp>
#include <gtest/gtest.h>

//==--- [compile time unrolling] -------------------------------------------==//

TEST(algorithm_unrolled_for, can_compile_time_unroll) {
  constexpr auto amount = size_t{3};
  auto           sum    = size_t{0};
  auto           res    = sum;

  for (size_t i = 0; i < amount; ++i) {
    res += i;
  }

  // i takes the values of 0 - (amount - 1):
  wrench::unrolled_for<amount>([&sum](auto i) { sum += i; });
  EXPECT_EQ(sum, res);
}

#endif // RIPPLE_TESTS_ALGORITHM_UNROLLED_FOR_TESTS_HPP