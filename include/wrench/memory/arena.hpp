//==--- wrench/memory/arena.hpp ---------------------------- -*- C++ -*- ---==//
//
//                              Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  arena.hpp
/// \brief This file defines memory arena implementations.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MEMORY_ARENA_HPP
#define WRENCH_MEMORY_ARENA_HPP

#include "memory_utils.hpp"
#include <type_traits>

namespace wrench {

/// Defines a stack-based memory arena of a specific size.
/// \tparam Size The size of the stack for the arena.
template <size_t Size>
class StackArena {
  /// Defines the size of the stack for the arena.
  static constexpr size_t stack_size = Size;

 public:
  //==--- [traits] ---------------------------------------------------------==//

  /// Returns that the allocator has a constexpr size.
  static constexpr bool contexpr_size = true;

  using Ptr      = void*;       //!< Pointer type.
  using ConstPtr = const void*; //!< Const pointer type.

  //==--- [constructor] ----------------------------------------------------==//

  /// Constructor which takes the size of the arena. This is provided to arena's
  /// have the same interface.
  /// \param size The size of the arena.
  StackArena(size_t size = 0) {}

  //==--- [interface] ------------------------------------------------------==//

  /// Returns a pointer to the beginning of the arena.
  [[nodiscard]] auto begin() const noexcept -> ConstPtr {
    return static_cast<ConstPtr>(&buffer_[0]);
  }

  /// Returns a pointer to the end of the arena.
  [[nodiscard]] auto end() const noexcept -> ConstPtr {
    return static_cast<ConstPtr>(&buffer_[stack_size]);
  }

  /// Returns the size of the arena.
  [[nodiscard]] constexpr auto size() const noexcept -> size_t {
    return stack_size;
  }

 private:
  char buffer_[stack_size]; //!< The buffer for the stack.
};

/// Defines a heap-based arena.
struct HeapArena {
 public:
  //==--- [traits] ---------------------------------------------------------==//

  /// Returns that the allocator does not have a constexpr size.
  static constexpr bool constexpr_size = false;

  using Ptr      = void*; //!< Pointer type.
  using ConstPtr = void*; //!< Const pointer type.

  //==--- [construction] ---------------------------------------------------==//

  /// Initializes the arena with a specific size.
  /// \param size The size of the arena, in bytes.
  explicit HeapArena(size_t size) {
    if (size) {
      start_ = malloc(size);
      end_   = offset_ptr(start_, size);
    }
  }

  /// Destructor to release the memory.
  ~HeapArena() noexcept {
    if (start_ != nullptr) {
      std::free(start_);
      start_ = nullptr;
      end_   = nullptr;
    }
  }

  //==--- [deleted] --------------------------------------------------------==//

  // clang-format off
  /// Copy constructor -- deleted.
  HeapArena(const HeapArena&)     = delete;
  /// Move constructor -- deleted.
  HeapArena(HeapArena&&) noexcept = delete;

  /// Copy assignment operator -- deleted.
  auto operator=(const HeapArena&) -> HeapArena&     = delete;
  /// Move assignment operator -- deleted.
  auto operator=(HeapArena&&) noexcept -> HeapArena& = delete;
  // clang-format on

  //==--- [interface] ------------------------------------------------------==//

  /// Returns a pointer to the beginning of the arena.
  [[nodiscard]] auto begin() const noexcept -> ConstPtr {
    return start_;
  }

  /// Returns a pointer to the end of the arena.
  [[nodiscard]] auto end() const noexcept -> ConstPtr {
    return end_;
  }

  /// Returns the size of the arena.
  [[nodiscard]] auto size() const noexcept -> size_t {
    return uintptr_t(end_) - uintptr_t(start_);
  }

 private:
  void* start_ = nullptr; //!< Pointer to the heap data.
  void* end_   = nullptr; //!< Pointer to the heap data.
};

//==--- [aliases] ----------------------------------------------------------==//

/// Defines the default size for a stack arena.
static constexpr size_t default_stack_arena_size = 1024;

/// Defines the type for a default stack arena.
using DefaultStackArena = StackArena<default_stack_arena_size>;

/// Defines a valid type if the Arena has a contexpr size.
/// \tparam Arena The arena to base the enable on.
template <typename Arena>
using ArenaConstexprSizeEnable =
  std::enable_if_t<std::decay_t<Arena>::contexpr_size, int>;

/// Defines a valid type if the Arena does not have a contexpr size.
/// \tparam Arena The arena to base the enable on.
template <typename Arena>
using ArenaNonConstexprSizeEnable =
  std::enable_if_t<!std::decay_t<Arena>::contexpr_size, int>;

} // namespace wrench

#endif // WRNCH_MEMORY_ARENA_HPP