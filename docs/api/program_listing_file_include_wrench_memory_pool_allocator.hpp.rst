
.. _program_listing_file_include_wrench_memory_pool_allocator.hpp:

Program Listing for File pool_allocator.hpp
===========================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_pool_allocator.hpp>` (``include/wrench/memory/pool_allocator.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/memory/pool_allocator.hpp ------------------- -*- C++ -*- ---==//
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
   
   #ifndef WRENCH_MEMORY_POOL_ALLOCATOR_HPP
   #define WRENCH_MEMORY_POOL_ALLOCATOR_HPP
   
   #include "memory_utils.hpp"
   #include <atomic>
   #include <cstddef>
   
   namespace wrench {
   
   //==--- [single-threaded free-list] ----------------------------------------==//
   
   class Freelist {
    public:
     //==--- [traits] ---------------------------------------------------------==//
     static constexpr bool resettable = false;
   
     //==--- [construction] ---------------------------------------------------==//
   
     Freelist() = default;
   
     ~Freelist() noexcept {
       if (head_ != nullptr) {
         head_ = nullptr;
       }
     }
   
     Freelist(
       const void* start,
       const void* end,
       size_t      element_size,
       size_t      alignment) noexcept
     : head_(initialize(start, end, element_size, alignment)) {}
   
     // clang-format off
     Freelist(Freelist&& other) noexcept                    = default;
     auto operator=(Freelist&& other) noexcept -> Freelist& = default;
   
     //==--- [deleted] --------------------------------------------------------==//
   
     Freelist(const Freelist&)                    = delete;
     auto operator=(const Freelist&) -> Freelist& = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     auto pop_front() noexcept -> void* {
       Node* const popped_head = head_;
       head_                   = popped_head ? popped_head->next : nullptr;
       return static_cast<void*>(popped_head);
     }
   
     auto push_front(void* ptr) noexcept -> void {
       if (ptr == nullptr) {
         return;
       }
   
       Node* const pushed_head = static_cast<Node*>(ptr);
       pushed_head->next       = head_;
       head_                   = pushed_head;
     }
   
    private:
     struct Node {
       Node* next = nullptr; 
     };
   
     Node* head_ = nullptr; 
   
     static auto initialize(
       const void* start, const void* end, size_t element_size, size_t alignment)
       -> Node* {
       // Create the first and second elements:
       void* const first  = align_ptr(start, alignment);
       void* const second = align_ptr(offset_ptr(first, element_size), alignment);
   
       const size_t size     = uintptr_t(second) - uintptr_t(first);
       const size_t elements = (uintptr_t(end) - uintptr_t(first)) / size;
   
       // Set the head of the list:
       Node* head = static_cast<Node*>(first);
   
       // Initialize the rest of the list:
       Node* current = head;
       for (size_t i = 1; i < elements; ++i) {
         Node* next    = static_cast<Node*>(offset_ptr(current, size));
         current->next = next;
         current       = next;
       }
       assert(
         offset_ptr(current, size) <= end &&
         "Freelist initialization overflows provided arena!");
   
       current->next = nullptr;
       return head;
     }
   };
   
   //==--- [multi-threaded free list] -----------------------------------------==//
   
   class ThreadSafeFreelist {
     struct Node {
       std::atomic<Node*> next; 
     };
   
     static constexpr size_t head_ptr_alignment_bytes = 8;
   
     struct alignas(head_ptr_alignment_bytes) HeadPtr {
       int32_t  offset; 
       uint32_t tag;    
     };
   
     using AtomicHeadPtr = std::atomic<HeadPtr>;
   
    public:
     //==--- [traits] ---------------------------------------------------------==//
   
     static constexpr bool resettable = false;
   
     //==--- [construction] ---------------------------------------------------==//
   
     ThreadSafeFreelist() noexcept = default;
   
     ThreadSafeFreelist(
       const void* start,
       const void* end,
       size_t      element_size,
       size_t      alignment) noexcept {
       assert(head_.is_lock_free());
   
       void* const first  = align_ptr(start, alignment);
       void* const second = align_ptr(offset_ptr(first, element_size), alignment);
   
       // Check that the resulting pointers are in the arena, and ordered
       // correctly:
       assert(first >= start && first < end);
       assert(second >= start && second > first && second < end);
   
       const size_t size     = uintptr_t(second) - uintptr_t(first);
       const size_t elements = (uintptr_t(end) - uintptr_t(first)) / size;
   
       // Set the head to the first element, and the storage to the head.
       Node* head = static_cast<Node*>(first);
       storage_   = head;
   
       // Link the list:
       Node* current = head;
       for (size_t i = 1; i < elements; ++i) {
         Node* next    = static_cast<Node*>(offset_ptr(current, size));
         current->next = next;
         current       = next;
       }
   
       // Ensure that everything fits in the allocation arena.
       assert(current < end);
       assert(offset_ptr(current, size) <= end);
       current->next = nullptr;
   
       // Set the head pointer as the offset from the storage to the aligned head
       // element, and set the initial tag to zero.
       head_.store({static_cast<int32_t>(head - storage_), 0});
     }
   
     ThreadSafeFreelist(ThreadSafeFreelist&& other) noexcept
     : head_(other.head_.load(std::memory_order_relaxed)),
       storage_(std::move(other.storage_)) {
       other.head_.store({-1, 0}, std::memory_order_relaxed);
       other.storage_ = nullptr;
     }
   
     auto operator=(ThreadSafeFreelist&& other) noexcept -> ThreadSafeFreelist& {
       if (this != &other) {
         head_.store(
           other.head_.load(std::memory_order_relaxed), std::memory_order_relaxed);
         storage_ = std::move(other.storage_);
         other.head_.store({-1, 0}, std::memory_order_relaxed);
         other.storage_ = nullptr;
       }
       return *this;
     }
   
     //==--- [deleted] --------------------------------------------------------==//
   
     // clang-format off
     ThreadSafeFreelist(const ThreadSafeFreelist&)                    = delete;
     auto operator=(const ThreadSafeFreelist&) -> ThreadSafeFreelist& = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     auto pop_front() noexcept -> void* {
       Node* const storage = storage_;
   
       // Here we acquire to synchronize with other popping threads which may
       // succeed first, and well as with other pushing threads which may push
       // before we pop, in which case we want to try and pop the newly pushed
       // head.
       HeadPtr current_head = head_.load(std::memory_order_acquire);
   
       while (current_head.offset >= 0) {
         // If another thread tries to pop, and does it faster than here, then this
         // pointer will contain data from the application. However, the new_head
         // which we compute just now, using this next pointer, will be discarded
         // because _head will have been replaced, and hence current_head will not
         // compare equal with _head, and thus the compare_exhange will fail and we
         // will try again.
         Node* const next =
           storage[current_head.offset].next.load(std::memory_order_relaxed);
   
         // Get the new head element. If the next pointer is a nullptr, then we are
         // at the end of the list, and if we succeed then another thread cannot
         // pop, so we set the offset to -1 so that on success, if _head is
         // replaced with new_head, then other threads will not execute this loop
         // and just return a nullptr.
         const HeadPtr new_head{
           next ? int32_t(next - storage) : -1, current_head.tag + 1};
   
         // If another thread was trying to pop, and got here just before us, then
         // the _head element would have moved, and it will have a different .tag
         // value than the tag value of current_head, so this will fail, and we
         // will try again.
         //
         // This is also how we avoid the ABA problem, where another thread might
         // have popped, and then pushed, leaving the head in the same place as the
         // current_head we now have, but with different data. The tag will prevent
         // this.
         //
         // Also, we need memory_order_release here to make the change visible on
         // success, for threads which load the upated _head. We use
         // memory_order_acquire for the failure case, because we want to
         // synchronize as at the beginning of the function.
         if (head_.compare_exchange_weak(
               current_head,
               new_head,
               std::memory_order_release,
               std::memory_order_acquire)) {
           // Lastly, we need the assert here for that case that another thread
           // performed a pop between our load of _head and _head.next. In this
           // case, the next that we loaded will contain application data, and
           // therefore could be invalid. So we check that we either have a
           // nullptr, in the case that we are at the last element, or that the
           // next pointer is in the memory arena, otherwise something went wrong.
           assert(!next || next >= storage);
           break;
         }
       }
   
       // Either we have the head, and we can return it, or we ran out of elements,
       // and we have to return a nullptr.
       // clang-format off
       void* p = (current_head.offset >= 0) 
               ? storage + current_head.offset : nullptr;
       // clang-format on
       return p;
     }
   
     auto push_front(void* ptr) noexcept -> void {
       Node* const storage = storage_;
       assert(ptr && ptr >= storage);
       Node* const node = static_cast<Node*>(ptr);
   
       // Here we don't care about synchronization with stores to _head from other
       // threads which are either trying to push or to pop. If that happens, the
       // compare exchange will fail and we will just try with the newly updated
       // head.
       HeadPtr current_head = head_.load(std::memory_order_relaxed);
       HeadPtr new_head     = {int32_t(node - storage), current_head.tag + 1};
   
       // Here we use memory_order_release in the success case, so that other
       // threads can synchronize with the updated head, but we don't care about
       // synchronizing the update of the current head because as above, if another
       // thread races ahead, we will just try again. The memory ordering with
       // respect to the current_head update is not important.
       do {
         // clang-format off
         new_head.tag     = current_head.tag + 1;
         Node* const next = (current_head.offset >= 0)
                          ? (storage + current_head.offset) : nullptr;
         node->next.store(next, std::memory_order_relaxed);
         // clang-format on
       } while (!head_.compare_exchange_weak(
         current_head,
         new_head,
         std::memory_order_release,
         std::memory_order_relaxed));
     }
   
    private:
     AtomicHeadPtr head_{};            
     Node*         storage_ = nullptr; 
   };
   
   //==--- [pool allocator] ---------------------------------------------------==//
   
   // clang-format off
   
   template <
     size_t   ElementSize,
     size_t   Alignment,
     typename FreelistImpl = Freelist>
   class PoolAllocator {
    private:
     // clang-format off
     static constexpr size_t element_size = ElementSize;
     static constexpr size_t alignment    = Alignment;
     // clang-format on
   
    public:
     //==--- [traits] ---------------------------------------------------------==//
   
     static constexpr bool resettable = FreelistImpl::resettable;
   
     //==--- [construction] ---------------------------------------------------==//
   
     // clang-format off
     PoolAllocator() noexcept  = delete;
     ~PoolAllocator() noexcept = default;
     // clang-format on
   
     PoolAllocator(const void* begin, const void* end) noexcept
     : freelist_(begin, end, element_size, alignment), begin_(begin), end_(end) {}
   
     template <typename Arena>
     explicit PoolAllocator(const Arena& arena) noexcept
     : PoolAllocator(arena.begin(), arena.end()) {}
   
     PoolAllocator(PoolAllocator&& other) noexcept = default;
   
     auto operator=(PoolAllocator&& other) noexcept -> PoolAllocator& = default;
   
     //==--- [deleted] --------------------------------------------------------==//
   
     // clang-format off
     PoolAllocator(const PoolAllocator&)                    = delete;
     auto operator=(const PoolAllocator&) -> PoolAllocator& = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     auto alloc(size_t size = element_size, size_t align = alignment) noexcept
       -> void* {
       assert(size <= element_size && align <= alignment);
       return freelist_.pop_front();
     }
   
     auto free(void* ptr, size_t = element_size) noexcept -> void {
       freelist_.push_front(ptr);
     }
   
     auto owns(void* ptr) const noexcept -> bool {
       return uintptr_t(ptr) >= uintptr_t(begin_) &&
              uintptr_t(ptr) < uintptr_t(end_);
     }
   
     auto reset() noexcept -> void {
       if constexpr (resettable) {
         freelist_.reset();
       }
     }
   
    private:
     FreelistImpl freelist_;        
     const void*  begin_ = nullptr; 
     const void*  end_   = nullptr; 
   };
   
   } // namespace wrench
   
   #endif // WRENCH_MEMORY_POOL_ALLOCATOR_HPP
