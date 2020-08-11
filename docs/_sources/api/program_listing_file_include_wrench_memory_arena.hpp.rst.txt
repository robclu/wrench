
.. _program_listing_file_include_wrench_memory_arena.hpp:

Program Listing for File arena.hpp
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_arena.hpp>` (``include/wrench/memory/arena.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_MEMORY_ARENA_HPP
   #define WRENCH_MEMORY_ARENA_HPP
   
   #include "memory_utils.hpp"
   #include <type_traits>
   
   namespace wrench {
   
   template <size_t Size>
   class StackArena {
     static constexpr size_t stack_size = Size;
   
    public:
     //==--- [traits] ---------------------------------------------------------==//
   
     static constexpr bool contexpr_size = true;
   
     using Ptr      = void*;       
     using ConstPtr = const void*; 
   
     //==--- [constructor] ----------------------------------------------------==//
   
     StackArena(size_t size = 0) {}
   
     //==--- [interface] ------------------------------------------------------==//
   
     [[nodiscard]] auto begin() const noexcept -> ConstPtr {
       return static_cast<ConstPtr>(&buffer_[0]);
     }
   
     [[nodiscard]] auto end() const noexcept -> ConstPtr {
       return static_cast<ConstPtr>(&buffer_[stack_size]);
     }
   
     [[nodiscard]] constexpr auto size() const noexcept -> size_t {
       return stack_size;
     }
   
    private:
     char buffer_[stack_size]; 
   };
   
   struct HeapArena {
    public:
     //==--- [traits] ---------------------------------------------------------==//
   
     static constexpr bool constexpr_size = false;
   
     using Ptr      = void*; 
     using ConstPtr = void*; 
   
     //==--- [construction] ---------------------------------------------------==//
   
     explicit HeapArena(size_t size) {
       if (size) {
         start_ = malloc(size);
         end_   = offset_ptr(start_, size);
       }
     }
   
     ~HeapArena() noexcept {
       if (start_ != nullptr) {
         std::free(start_);
         start_ = nullptr;
         end_   = nullptr;
       }
     }
   
     //==--- [deleted] --------------------------------------------------------==//
   
     // clang-format off
     HeapArena(const HeapArena&)     = delete;
     HeapArena(HeapArena&&) noexcept = delete;
   
     auto operator=(const HeapArena&) -> HeapArena&     = delete;
     auto operator=(HeapArena&&) noexcept -> HeapArena& = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     [[nodiscard]] auto begin() const noexcept -> ConstPtr {
       return start_;
     }
   
     [[nodiscard]] auto end() const noexcept -> ConstPtr {
       return end_;
     }
   
     [[nodiscard]] auto size() const noexcept -> size_t {
       return uintptr_t(end_) - uintptr_t(start_);
     }
   
    private:
     void* start_ = nullptr; 
     void* end_   = nullptr; 
   };
   
   //==--- [aliases] ----------------------------------------------------------==//
   
   static constexpr size_t default_stack_arena_size = 1024;
   
   using DefaultStackArena = StackArena<default_stack_arena_size>;
   
   template <typename Arena>
   using ArenaConstexprSizeEnable =
     std::enable_if_t<std::decay_t<Arena>::contexpr_size, int>;
   
   template <typename Arena>
   using ArenaNonConstexprSizeEnable =
     std::enable_if_t<!std::decay_t<Arena>::contexpr_size, int>;
   
   } // namespace wrench
   
   #endif // WRNCH_MEMORY_ARENA_HPP
