
.. _program_listing_file_include_wrench_memory_linear_allocator.hpp:

Program Listing for File linear_allocator.hpp
=============================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_linear_allocator.hpp>` (``include/wrench/memory/linear_allocator.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_MEMORY_LINEAR_ALLOCATOR_HPP
   #define WRENCH_MEMORY_LINEAR_ALLOCATOR_HPP
   
   #include "memory_utils.hpp"
   #include <algorithm>
   
   namespace wrench {
   
   class LinearAllocator {
    public:
     LinearAllocator(void* begin, void* end) noexcept
     : begin_(begin), size_(uintptr_t(end) - uintptr_t(begin)) {}
   
     template <typename Arena>
     explicit LinearAllocator(const Arena& arena)
     : LinearAllocator(arena.begin(), arena.end()) {}
   
     ~LinearAllocator() noexcept = default;
   
     LinearAllocator(LinearAllocator&& other) noexcept {
       swap(other);
     }
   
     auto operator=(LinearAllocator&& other) noexcept -> LinearAllocator& {
       if (this != &other) {
         swap(other);
       }
       return *this;
     }
   
     //==--- [deleted] --------------------------------------------------------==//
   
     // clang-format off
     LinearAllocator(const LinearAllocator&) = delete;
     auto operator=(const LinearAllocator&)  = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     auto alloc(size_t size, size_t alignment) noexcept -> void* {
       void* const ptr     = align_ptr(current(), alignment);
       void* const curr    = offset_ptr(p, size);
       bool        success = curr <= end();
       set_current(success ? curr : current());
       return success ? ptr : nullptr;
     }
   
     auto free(void* ptr) const noexcept -> void {}
   
     auto free(void* ptr, size_t size) const noexcept -> void {}
   
     auto reset() noexcept -> void {
       current_ = 0;
     }
   
    private:
     void*    begin_   = nullptr; 
     uint32_t size_    = 0;       
     uint32_t current_ = 0;       
   
     //==--- [utils] ----------------------------------------------------------==//
   
     [[nodiscard]] auto current() const noexcept -> void* {
       return offset_ptr(begin_, current_);
     }
   
     auto set_current(void* current_ptr) noexcept -> void {
       current_ = uintptr_t(current_ptr) - uintptr_t(begin_);
     }
   
     [[nodiscard]] auto end() const noexcept -> void* {
       return offset_ptr(begin_, size_);
     }
   
     auto swap(LinearAllocator& other) noexcept -> void {
       std::swap(begin_, other.begin_);
       std::swap(size_, other.size_);
       std::swap(current_, other.current_);
     }
   };
   
   } // namespace wrench
   
   #endif // WRENCH_MEMORY_LINEAR_ALLOCATOR_HPP
