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
  auto p = wrench::UniquePtr<int, UniqueDeleter>{std::move(i), UniqueDeleter()};

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

TEST(memory_unique_ptr, default_construct) {
  auto p = wrench::UniquePtr<int>{};
  auto q = wrench::UniquePtr<int>{};

  EXPECT_TRUE(p == nullptr);
  EXPECT_TRUE(q == nullptr);
  EXPECT_EQ(sizeof(p), sizeof(int*));
  EXPECT_EQ(static_cast<bool>(p), false);
  EXPECT_EQ(static_cast<bool>(q), false);
}

TEST(memory_unique_ptr, nullptr_construct) {
  auto p = wrench::UniquePtr<int>{nullptr};
  auto q = wrench::UniquePtr<int>{nullptr};

  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(q, nullptr);
  EXPECT_EQ(sizeof(p), sizeof(int*));
  EXPECT_EQ(static_cast<bool>(p), false);
  EXPECT_EQ(static_cast<bool>(q), false);
}

TEST(memory_unique_ptr, derived_construct) {
  struct Base {
    int x = 0;
  };
  struct Derived : public Base {
    Derived(int v) : Base{v} {}
  };

  wrench::UniquePtr<Base> p = wrench::make_unique<Derived>(unique_test_val);
  wrench::UniquePtr<Base> q =
    wrench::UniquePtr<Derived>(new Derived(unique_test_val));

  EXPECT_TRUE(p != nullptr);
  EXPECT_EQ(sizeof(p), sizeof(Base*));
  EXPECT_EQ(static_cast<bool>(p), true);
  EXPECT_EQ(p->x, unique_test_val);

  EXPECT_TRUE(q != nullptr);
  EXPECT_EQ(sizeof(q), sizeof(Base*));
  EXPECT_EQ(static_cast<bool>(q), true);
  EXPECT_EQ(q->x, unique_test_val);
}

TEST(memory_unique_ptr, copy_constructible) {
  const bool a = std::is_copy_constructible_v<wrench::UniquePtr<int>>;
  const bool b = std::is_copy_assignable_v<wrench::UniquePtr<int>>;
  const bool c =
    std::is_copy_constructible_v<wrench::UniquePtr<int, UniqueDeleter>>;
  const bool d =
    std::is_copy_assignable_v<wrench::UniquePtr<int, UniqueDeleter>>;

  EXPECT_FALSE(a);
  EXPECT_FALSE(b);
  EXPECT_FALSE(c);
  EXPECT_FALSE(d);
}

TEST(memory_unique_ptr, move_constructible) {
  wrench::UniquePtr<int> p{new int(unique_test_val)};
  EXPECT_TRUE(p != nullptr);
  EXPECT_EQ(*p, unique_test_val);

  auto q = std::move(p);
  EXPECT_TRUE(p == nullptr);
  EXPECT_TRUE(q != nullptr);
  EXPECT_EQ(*q, unique_test_val);

  wrench::UniquePtr<int> r{new int(unique_test_val)};
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ(*r, unique_test_val);

  r = std::move(q);
  EXPECT_TRUE(q == nullptr);
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ(*r, unique_test_val);

  const bool a = std::is_nothrow_move_constructible_v<wrench::UniquePtr<int>>;
  const bool b = std::is_nothrow_move_assignable_v<wrench::UniquePtr<int>>;
  const bool c =
    std::is_nothrow_move_constructible_v<wrench::UniquePtr<int, UniqueDeleter>>;
  const bool d =
    std::is_nothrow_move_assignable_v<wrench::UniquePtr<int, UniqueDeleter>>;

  EXPECT_TRUE(a);
  EXPECT_TRUE(b);
  EXPECT_TRUE(c);
  EXPECT_TRUE(d);
}

TEST(memory_unique_ptr, reset) {
  wrench::UniquePtr<int> p{new int(unique_test_val)};
  EXPECT_TRUE(p != nullptr);
  EXPECT_EQ(*p, unique_test_val);

  p.reset(new int(unique_test_val * 2));
  EXPECT_TRUE(p != nullptr);
  EXPECT_EQ(*p, unique_test_val * 2);
}

TEST(memory_unique_ptr, release) {
  wrench::UniquePtr<int> p{new int(unique_test_val)};
  EXPECT_TRUE(p != nullptr);
  EXPECT_EQ(*p, unique_test_val);

  int* q = p.release();
  EXPECT_TRUE(q != nullptr);
  EXPECT_EQ(*q, unique_test_val);
}

TEST(memory_unique_ptr, get) {
  wrench::UniquePtr<int> p{new int(unique_test_val)};
  EXPECT_TRUE(p.get() != nullptr);
  EXPECT_EQ(*(p.get()), unique_test_val);
}

#endif // WRENCH_TESTS_MEMORY_UNIQUE_PTR_HPP