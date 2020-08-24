//==--- wrench/memory/linear_allocator.hpp ----------------- -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  linear_allocator.hpp
/// \brief This file defines a linear allocator implementation.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MEMORY_LINEAR_ALLOCATOR_HPP
#define WRENCH_MEMORY_LINEAR_ALLOCATOR_HPP

#include "memory_utils.hpp"
#include <algorithm>

namespace wrench {

/// This allocator allocates data linearly from a provided arena. While it
/// provides an interface for freeing an individual element, it's an empty
/// function, and the allocator only allows resetting all allocations from the
/// pool. It just bumps along the pointer to the next allocation address. It can
/// allocate different sizes.
class LinearAllocator {
 public:
  /// Constructor to set the \p begin and \p end of the available memory for the
  /// allocator.
  /// \param begin The start of the allocation arena.
  /// \param end   The end of the allocation arena.
  LinearAllocator(void* begin, void* end) noexcept
  : begin_(begin), size_(uintptr_t(end) - uintptr_t(begin)) {}

  /// Constructor which takes an Arena from which the allocator can allocate.
  /// \param  arena The area to allocate memory from.
  /// \tparam Arena The type of the arena.
  template <typename Arena>
  explicit LinearAllocator(const Arena& arena)
  : LinearAllocator(arena.begin(), arena.end()) {}

  /// Constructor -- defaulted.
  ~LinearAllocator() noexcept = default;

  /// Move construcor, swaps \p other with this allocator.
  /// \param other The other allocator to create this one from.
  LinearAllocator(LinearAllocator&& other) noexcept {
    swap(other);
  }

  /// Move assignment, swaps the \p other allocator with this one.
  /// \param other The other allocator to swap with this one.
  auto operator=(LinearAllocator&& other) noexcept -> LinearAllocator& {
    if (this != &other) {
      swap(other);
    }
    return *this;
  }

  //==--- [deleted] --------------------------------------------------------==//

  // clang-format off
  /// Copy constructor -- deleted to disable copying.
  LinearAllocator(const LinearAllocator&) = delete;
  /// Copy assignment -- deleted to disable copying.
  auto operator=(const LinearAllocator&)  = delete;
  // clang-format on

  //==--- [interface] ------------------------------------------------------==//

  /// Allocates \p size bytes with \p alignment.
  /// \param size      The number of bytes to allocate.
  /// \param alignment The alignment for the allocation.
  auto alloc(size_t size, size_t alignment) noexcept -> void* {
    void* const ptr     = align_ptr(current(), alignment);
    void* const curr    = offset_ptr(ptr, size);
    bool        success = curr <= end();
    set_current(success ? curr : current());
    return success ? ptr : nullptr;
  }

  /// This __does not__ free the \p ptr, since it does not allow freeing of
  /// individual allocations. This allocator only allows resetting.
  /// \param ptr The pointer to free.
  auto free(void* ptr) const noexcept -> void {}

  /// This __does not__ free the \p ptr, since it does not allow freeing of
  /// individual allocations. This allocator only allows resetting.
  /// \param ptr  The pointer to free.
  /// \param size The size to free.
  auto free(void* ptr, size_t size) const noexcept -> void {}

  /**
   * Determines if this allocator owns the \p ptr.
   * \param ptr The pointer to determine if the allocator owns.
   * \return __true__ if the allocator owns the pointer.
   */
  auto owns(void* ptr) const noexcept -> bool {
    return uintptr_t(ptr) >= uintptr_t(begin_) &&
           uintptr_t(ptr) < uintptr_t(end());
  }

  /// Resets the allocator to the begining of the allocation arena. This
  /// invalidates any allocations from the allocator, since any subsequent
  /// allocations will overwrite old allocations.
  auto reset() noexcept -> void {
    current_ = 0;
  }

 private:
  void*    begin_   = nullptr; //!< Pointer to the start of the region.
  uint32_t size_    = 0;       //!< Size of the region.
  uint32_t current_ = 0;       //!< Current allocation location.

  //==--- [utils] ----------------------------------------------------------==//

  /// Returns a pointer to the current pointer.
  auto current() const noexcept -> void* {
    return offset_ptr(begin_, current_);
  }

  /// Sets the value of _current, such that when offset from _begin, it pointer
  /// to \p current_ptr.
  /// \param current_ptr The pointer to set current to point to.
  auto set_current(void* current_ptr) noexcept -> void {
    current_ = uintptr_t(current_ptr) - uintptr_t(begin_);
  }

  /// Returns the end of the allocator's allocation arena.
  auto end() const noexcept -> void* {
    return offset_ptr(begin_, size_);
  }

  /// Swaps the \p other allocator with this one.
  /// \param other The other allocator to swap with this one.
  auto swap(LinearAllocator& other) noexcept -> void {
    std::swap(begin_, other.begin_);
    std::swap(size_, other.size_);
    std::swap(current_, other.current_);
  }
};

} // namespace wrench

#endif // WRENCH_MEMORY_LINEAR_ALLOCATOR_HPP