//==--- wrench/memory/memory_utils.hpp --------------------- -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  memory_utils.hpp
/// \brief This file defines utility functions for memory related operations.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MEMORY_MEMORY_UTILS_HPP
#define WRENCH_MEMORY_MEMORY_UTILS_HPP

#include <cassert>
#include <cstdint>

namespace wrench {

/// Returns a new ptr offset by \p amount from \p ptr.
///
/// \note This does __not__ ensure alignemt. If the pointer needs to be aligned,
///       then pass the result to `align()`.
///
/// \param ptr    The pointer to offset.
/// \param amount The amount to offset ptr by.
static inline auto
offset_ptr(const void* ptr, uint32_t amount) noexcept -> void* {
  return reinterpret_cast<void*>(uintptr_t(ptr) + amount);
}

/// Returns a pointer with an address aligned to \p alignment. This will fail at
/// runtime if the \p alignemnt is not a power of two.
/// \param ptr       The pointer to align.
/// \param alignment The alignment to ensure.
static inline auto
align_ptr(const void* ptr, size_t alignment) noexcept -> void* {
  assert(
    !(alignment & (alignment - 1)) &&
    "Alignment must be a power of two for linear allocation!");
  return reinterpret_cast<void*>(
    (uintptr_t(ptr) + alignment - 1) & ~(alignment - 1));
}

} // namespace wrench

#endif // WRENCH_MEMORY_MEMORY_UTILS_HPP