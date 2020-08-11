
.. _program_listing_file_include_wrench_memory_allocator.hpp:

Program Listing for File allocator.hpp
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_allocator.hpp>` (``include/wrench/memory/allocator.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/memory/allocator.hpp ------------------------ -*- C++ -*- ---==//
   //
   //                                  Wrench
   //
   //                      Copyright (c) 2020 Rob Clucas
   //
   //  This file is distributed under the MIT License. See LICENSE for details.
   //
   //==------------------------------------------------------------------------==//
   //
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_MEMORY_ALLOCATOR_HPP
   #define WRENCH_MEMORY_ALLOCATOR_HPP
   
   #include "aligned_heap_allocator.hpp"
   #include "arena.hpp"
   #include "pool_allocator.hpp"
   #include <mutex>
   
   namespace wrench {
   
   struct VoidLock {
     auto lock() noexcept -> void {}
     auto unlock() noexcept -> void {}
   };
   
   //==--- [forward declarations & aliases] -----------------------------------==//
   
   template <
     typename PrimaryAllocator,
     typename Arena             = HeapArena,
     typename FallbackAllocator = AlignedHeapAllocator,
     typename LockingPolicy     = VoidLock>
   class Allocator;
   
   template <
     typename T,
     typename LockingPolicy = VoidLock,
     typename Arena         = HeapArena>
   using ObjectPoolAllocator = Allocator<
     PoolAllocator<sizeof(T), std::max(alignof(T), alignof(Freelist)), Freelist>,
     Arena,
     AlignedHeapAllocator,
     LockingPolicy>;
   
   template <typename T, typename Arena = HeapArena>
   using ThreadSafeObjectPoolAllocator = Allocator<
     PoolAllocator<
       sizeof(T),
       std::max(alignof(T), alignof(ThreadSafeFreelist)),
       ThreadSafeFreelist>,
     Arena,
     AlignedHeapAllocator,
     VoidLock>;
   
   //==--- [implementation] ---------------------------------------------------==//
   
   template <
     typename PrimaryAllocator,
     typename Arena,
     typename FallbackAllocator,
     typename LockingPolicy>
   class Allocator {
     static_assert(
       std::is_trivially_constructible_v<FallbackAllocator>,
       "Fallback allocator must be trivially constructible!");
   
    public:
     //==--- [constants] ------------------------------------------------------==//
   
     static constexpr bool contexpr_arena_size = Arena::contexpr_size;
   
     //==--- [aliases] --------------------------------------------------------==//
   
     using Guard = std::lock_guard<LockingPolicy>;
   
     //==--- [construction] ---------------------------------------------------==//
   
     template <typename... Args>
     Allocator(size_t size, Args&&... args)
     : arena_(size), primary_(arena_, std::forward<Args>(args)...) {}
   
     ~Allocator() noexcept = default;
   
     // clang-format off
     Allocator(Allocator&& other) noexcept                   = default;
     auto operator=(Allocator&& other) noexcept -> Allocator& = default;
   
     //==--- [deleted] --------------------------------------------------------==//
   
     Allocator(const Allocator&)                    = delete;
     auto operator=(const Allocator&) -> Allocator& = delete;
     // clang-format on
   
     //==--- [alloc/free interface] -------------------------------------------==//
   
     auto alloc(size_t size, size_t alignment = alignof(std::max_align_t)) noexcept
       -> void* {
       Guard g(lock_);
       void* ptr = primary_.alloc(size, alignment);
       if (ptr == nullptr) {
         ptr = fallback_.alloc(size, alignment);
       }
       return ptr;
     }
   
     auto free(void* ptr) noexcept -> void {
       if (ptr == nullptr) {
         return;
       }
   
       Guard g(lock_);
       if (primary_.owns(ptr)) {
         primary_.free(ptr);
         return;
       }
   
       fallback_.free(ptr);
     }
   
     auto free(void* ptr, size_t size) noexcept -> void {
       if (ptr == nullptr) {
         return;
       }
   
       Guard g(lock_);
       if (primary_.owns(ptr)) {
         primary_.free(ptr, size);
         return;
       }
       fallback_.free(ptr, size);
     }
   
     auto reset() noexcept -> void {
       Guard g(lock_);
       primary_.reset();
     }
   
     //==--- [create/recycle interface] ---------------------------------------==//
   
     template <typename T, typename... Args>
     auto create(Args&&... args) noexcept -> T* {
       constexpr size_t size      = sizeof(T);
       constexpr size_t alignment = alignof(T);
       void* const      ptr       = alloc(size, alignment);
   
       return new (ptr) T(std::forward<Args>(args)...);
     }
   
     template <typename T>
     auto recycle(T* ptr) noexcept -> void {
       if (ptr == nullptr) {
         return;
       }
   
       ptr->~T();
       free(static_cast<void*>(ptr), sizeof(T));
     }
   
    private:
     Arena             arena_;    
     PrimaryAllocator  primary_;  
     FallbackAllocator fallback_; 
     LockingPolicy     lock_;     
   };
   
   } // namespace wrench
   
   #endif // WRENCH_MEMORY_ALLOCATOR_HP
