//==--- wrench/memory/aligned_heap_allocator.hpp ----------- -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Wrench
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  aligned_heap_allocator.hpp
/// \brief This file defines an allocator which uses aligned_alloc for
///        allocation.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MEMORY_ALIGNED_HEAP_ALLOCATOR_HPP
#define WRENCH_MEMORY_ALIGNED_HEAP_ALLOCATOR_HPP

#include <cstdlib>

namespace wrench {

/// This type implements an allocator which allocates aligned memory on the
/// heap. It's the slowest allocator, and should therefore always be used as
/// the last resort allocator.
class AlignedHeapAllocator {
 public:
  /// Defines the type of the allocator.
  using Self = AlignedHeapAllocator;

  //==--- [construction] ---------------------------------------------------==//

  // clang-format off
  /// Default constructor.
  AlignedHeapAllocator()  = default;
  /// Destructor -- defaulted.
  ~AlignedHeapAllocator() = default;

  /// Constructor which takes an Arena, which is provided for compatability with
  /// other allocators.
  /// \param  arena The area to allocate memory from. Unused by this allocator.
  /// \tparam Arena The type of the arena.
  template <typename Arena>
  AlignedHeapAllocator(const Arena& arena) {}

  /// Move construcor -- defaulted.
  AlignedHeapAllocator(AlignedHeapAllocator&&) noexcept    = default;
  /// Move assignment -- defaulted.
  auto operator=(AlignedHeapAllocator&&) noexcept -> Self& = default;

  //==--- [deleted] --------------------------------------------------------==//

  /// Copy constructor -- deleted since allocators can't be moved.
  AlignedHeapAllocator(const AlignedHeapAllocator&)    = delete;
  /// Copy assignment -- deleted since allocators can't be copied.
  auto operator=(const AlignedHeapAllocator&) -> Self& = delete;
  // clang-format on

  //==--- [interface] ------------------------------------------------------==//

  /// Allocates \p size bytes of memory with \p align alignment.
  /// \param size      The size of the memory to allocate.
  /// \param alignment The alignment of the allocation.
  auto alloc(size_t size, size_t alignment) noexcept -> void* {
    return aligned_alloc(alignment, size);
  }

  /// Frees the memory pointed to by ptr.
  /// \param ptr The pointer to the memory to free.
  auto free(void* ptr) noexcept -> void {
    std::free(ptr);
  }

  /// Frees the memory pointed to by \p ptr.
  /// \param ptr The pointer to the memory to free.
  auto free(void* ptr, size_t) noexcept -> void {
    std::free(ptr);
  }
};

} // namespace wrench

#endif // WRENCH_MEMORY_ALIGNED_HEAP_ALLOCATOR_HPP