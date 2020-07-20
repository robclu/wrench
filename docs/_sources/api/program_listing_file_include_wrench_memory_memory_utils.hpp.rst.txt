
.. _program_listing_file_include_wrench_memory_memory_utils.hpp:

Program Listing for File memory_utils.hpp
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_memory_utils.hpp>` (``include/wrench/memory/memory_utils.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_MEMORY_MEMORY_UTILS_HPP
   #define WRENCH_MEMORY_MEMORY_UTILS_HPP
   
   #include <cassert>
   #include <cstdint>
   
   namespace wrench {
   
   static inline auto
   offset_ptr(const void* ptr, uint32_t amount) noexcept -> void* {
     return reinterpret_cast<void*>(uintptr_t(ptr) + amount);
   }
   
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
