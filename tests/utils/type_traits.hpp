//==--- wrench/tests/util/type_traits.hpp ------------------ -*- C++ -*- ---==//
//
//                                 Wrench
//
//                      Copyright (c) 2020 Rob Clucas.
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  type_traits.hpp
/// \brief This file implements tests for type traits.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_TESTS_UTIL_TYPE_TRAITS_HPP
#define WRENCH_TESTS_UTIL_TYPE_TRAITS_HPP

#include <wrench/utils/type_traits.hpp>
#include <gtest/gtest.h>

template <typename A>
struct BaseA {};

template <typename A, typename B>
struct BaseB {};

template <typename A, typename... Bs>
struct BaseVar {};

template <size_t V>
struct BaseV {};

struct TestA : BaseA<int> {};
struct TestB : BaseB<int, float> {};
struct TestVar : BaseVar<int, int, float, double> {};
struct TestV : BaseV<4> {};

TEST(type_traits, base_of_template_detects_derived) {
  const bool a = wrench::is_base_of_template_v<BaseA, TestA>;
  const bool b = wrench::is_base_of_template_v<BaseB, TestB>;
  const bool c = wrench::is_base_of_template_v<BaseVar, TestVar>;
  const bool d = wrench::is_base_of_nontype_template_v<BaseV, TestV>;

  EXPECT_TRUE(a);
  EXPECT_TRUE(b);
  EXPECT_TRUE(c);
  EXPECT_TRUE(d);
}

#endif // WRENCH_TESTS_UTIL_TYPE_TRAITS_HPP
