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

#include <wrench/memory/allocator.hpp>
#include <wrench/memory/intrusive_ptr.hpp>
#include <gtest/gtest.h>

static constexpr int x_val = 1;

struct PtrTest : public wrench::IntrusivePtrEnabled<PtrTest> {
  int x = x_val;
};

struct Deleter;
struct AllocTest : public wrench::IntrusivePtrEnabled<AllocTest, Deleter> {
  int x = x_val;
};

using Allocator = wrench::ObjectPoolAllocator<AllocTest>;
static auto allocator() noexcept -> Allocator& {
  static Allocator alloc(sizeof(AllocTest) * 10);
  return alloc;
}

struct Deleter {
  void operator()(AllocTest* p) noexcept {
    allocator().recycle(p);
  };
};

TEST(memory_intrusive_ptr, can_make_intrusive_ptr) {
  auto p = wrench::make_intrusive_ptr<PtrTest>();
  EXPECT_EQ(p->x, x_val);
}

TEST(memory_intrusive_ptr, can_allocate_intrusive_ptr) {
  auto p = wrench::allocate_intrusive_ptr<AllocTest>(allocator());
  EXPECT_EQ(p->x, x_val);
}

#endif // WRENCH_TESTS_MEMORY_INTRUSIVE_PTR_TESTS_HPP