
.. _program_listing_file_include_wrench_memory_ref_tracker.hpp:

Program Listing for File ref_tracker.hpp
========================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_ref_tracker.hpp>` (``include/wrench/memory/ref_tracker.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/memory/ref_tracker.hpp ---------------------- -*- C++ -*- ---==//
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
   //==------------------------------------------------------------------------==/
   
   #ifndef WRENCH_MEMORY_REF_TRACKER_HPP
   #define WRENCH_MEMORY_REF_TRACKER_HPP
   
   #include <atomic>
   #include <type_traits>
   #include <utility>
   
   namespace wrench {
   
   //==--- [forward declarations & aliases] -----------------------------------==//
   
   template <typename Impl>
   class RefTracker;
   
   class SingleThreadedRefTracker;
   
   class MultiThreadedRefTracker;
   
   using DefaultRefTracker =
   #if defined(WRENCH_SINGLE_THREADED)
     SingleThreadedRefTracker;
   #else
     MultiThreadedRefTracker;
   #endif
   
   template <typename T>
   static constexpr bool is_ref_tracker_v =
     std::is_base_of_v<RefTracker<std::decay_t<T>>, std::decay_t<T>>;
   
   //==--- [implementation] ---------------------------------------------------==//
   
   template <typename Impl>
   class RefTracker {
     [[nodiscard]] auto impl() noexcept -> Impl* {
       return static_cast<Impl*>(this);
     }
   
     [[nodiscard]] auto impl() const noexcept -> const Impl* {
       return static_cast<const Impl*>(this);
     }
   
    public:
     auto add_reference() noexcept -> void {
       impl()->add_reference_impl();
     }
   
     auto release() noexcept -> bool {
       return impl()->release_impl();
     }
   
     template <typename T, typename Deleter>
     auto destroy_resource(T* resource, Deleter&& deleter) noexcept -> void {
       impl()->destroy_resource_impl(resource, std::forward<Deleter>(deleter));
     }
   };
   
   //==--- [single-threaded implementation] -----------------------------------==//
   
   class SingleThreadedRefTracker : public RefTracker<SingleThreadedRefTracker> {
    public:
     using Counter = size_t;
   
     auto add_reference_impl() noexcept -> void {
       ref_count_++;
     }
   
     auto release_impl() noexcept -> bool {
       return --ref_count_ == 0;
     }
   
     template <typename T, typename Deleter>
     auto destroy_resource_impl(T* resource, Deleter&& deleter) noexcept -> void {
       deleter(resource);
     }
   
    private:
     Counter ref_count_ = 1; 
   };
   
   //==---[multi-threaded implementation] -------------------------------------==//
   
   class MultiThreadedRefTracker : public RefTracker<MultiThreadedRefTracker> {
    public:
     using Counter = std::atomic_size_t;
   
     MultiThreadedRefTracker() noexcept {
       ref_count_.store(1, std::memory_order_relaxed);
     }
   
     //==--- [move] -----------------------------------------------------------==//
   
     auto add_reference_impl() noexcept -> void {
       // Memory order relaxed because new references can only be created from
       // existing instances with the reference count, so we just care about
       // incrementing the ref atomically, not about the memory ordering here.
       ref_count_.fetch_add(1, std::memory_order_relaxed);
     }
   
     auto release() noexcept -> bool {
       // Here we need to ensure that any access from another thread __happens
       // before__ the deleting the object, though a call to `destroy` __if__ this
       // returns true.
       //
       // To ensure this, no reads/or write can be reordered to be after the
       // `fetch_sub` (i.e they happen before). Another thread might hold the last
       // reference, and before deleting, the `fetch_sub` needs to happen on
       // __this__ thread __before__ that thread deletes, which is done with
       // `memory_order_release`.
       //
       // Note the delete needs a `memory_order_acquire` before it, so that there
       // is a valid release-acquire sequence. We could use
       // memory_order_acq_release` here, but that wastes an aquire for each
       // decrement, when it's only required before deleting. We put the thread
       // fence in the destruction implementation to ensure the correct behaviour.
       return ref_count_.fetch_sub(1, std::memory_order_release) == 1;
     }
   
     template <typename T, typename Deleter>
     auto destroy_resource_impl(T* resource, Deleter&& deleter) noexcept -> void {
       // Here we need to ensure that no read or write is ordered before the
       // `fetch_sub` in the `release` call. Otherwise another thread might could
       // see a destroyed object before the reference count is zero. This is done
       // with the barrier with `memory_order_acquire`.
       std::atomic_thread_fence(std::memory_order_acquire);
       deleter(resource);
     }
   
    private:
     Counter ref_count_ = 1; 
   };
   
   } // namespace wrench
   
   #endif // WRENCH_MEMORY_REF_TRACKER_HPP
