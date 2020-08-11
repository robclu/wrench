
.. _program_listing_file_include_wrench_memory_aligned_heap_allocator.hpp:

Program Listing for File aligned_heap_allocator.hpp
===================================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_aligned_heap_allocator.hpp>` (``include/wrench/memory/aligned_heap_allocator.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_MEMORY_ALIGNED_HEAP_ALLOCATOR_HPP
   #define WRENCH_MEMORY_ALIGNED_HEAP_ALLOCATOR_HPP
   
   #include <cstdlib>
   
   namespace wrench {
   
   class AlignedHeapAllocator {
    public:
     using Self = AlignedHeapAllocator;
   
     //==--- [construction] ---------------------------------------------------==//
   
     // clang-format off
     AlignedHeapAllocator()  = default;
     ~AlignedHeapAllocator() = default;
   
     template <typename Arena>
     AlignedHeapAllocator(const Arena& arena) {}
   
     AlignedHeapAllocator(AlignedHeapAllocator&&) noexcept    = default;
     auto operator=(AlignedHeapAllocator&&) noexcept -> Self& = default;
   
     //==--- [deleted] --------------------------------------------------------==//
   
     AlignedHeapAllocator(const AlignedHeapAllocator&)    = delete;
     auto operator=(const AlignedHeapAllocator&) -> Self& = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     auto alloc(size_t size, size_t alignment) noexcept -> void* {
       return aligned_alloc(alignment, size);
     }
   
     auto free(void* ptr) noexcept -> void {
       std::free(ptr);
     }
   
     auto free(void* ptr, size_t) noexcept -> void {
       std::free(ptr);
     }
   };
   
   } // namespace wrench
   
   #endif // WRENCH_MEMORY_ALIGNED_HEAP_ALLOCATOR_HPP
