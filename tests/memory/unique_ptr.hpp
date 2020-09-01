//==--- wrench/tests/memory/unique_ptr.hpp ----------------- -*- C++ -*- ---==//
//
//                                 Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  unique_ptr.hpp
/// \brief This file implements tests for unique ptr.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_TESTS_MEMORY_UNIQUE_PTR_HPP
#define WRENCH_TESTS_MEMORY_UNIQUE_PTR_HPP

#include <wrench/memory/allocator.hpp>
#include <wrench/memory/unique_ptr.hpp>
#include <gtest/gtest.h>

static constexpr int unique_test_val = 1;

using UniqueAllocator = wrench::ObjectPoolAllocator<int>;

static auto unique_allocator() noexcept -> UniqueAllocator& {
  static UniqueAllocator alloc(sizeof(int) * 10);
  return alloc;
}

struct UniqueDeleter {
  void operator()(int* p) noexcept {
    unique_allocator().recycle(p);
  };
};

using Ptr        = wrench::UniquePtr<int>;
using DeleterPtr = wrench::UniquePtr<int, Deleter>;

TEST(memory_unique_ptr, can_make_unique_ptr) {
  auto p = wrench::make_unique<int>(unique_test_val);
  EXPECT_EQ(*p, unique_test_val);
}

TEST(memory_unique_ptr, can_allocate_with_unqiue_ptr) {
  int* i = unique_allocator().create<int>(unique_test_val);
  *i     = unique_test_val;
  auto p = wrench::UniquePtr<int, UniqueDeleter>(std::move(i), UniqueDeleter());

  EXPECT_EQ(*p, unique_test_val);
  EXPECT_EQ(sizeof(p), sizeof(int*));

  UniqueDeleter d;

  p.reset();
  EXPECT_EQ(static_cast<bool>(p), false);
  EXPECT_EQ(p.get(), nullptr);

  int* j = unique_allocator().create<int>(unique_test_val);
  *j     = unique_test_val;
  p      = wrench::UniquePtr<int, UniqueDeleter>(std::move(j), d);
  EXPECT_EQ(*p, unique_test_val);
  EXPECT_EQ(sizeof(p), sizeof(int*));
}

#endif // WRENCH_TESTS_MEMORY_UNIQUE_PTR_HPP