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
/// \file  ref_tracker.hpp
/// \brief This file defines an interface for a reference tracker and some
///        implementations of the tracker interface for different contexts.
//
//==------------------------------------------------------------------------==/

#ifndef WRENCH_MEMORY_REF_TRACKER_HPP
#define WRENCH_MEMORY_REF_TRACKER_HPP

#include <atomic>
#include <type_traits>
#include <utility>

namespace wrench {

//==--- [forward declarations & aliases] -----------------------------------==//

/// Forward declaration of reference tracking interface.
/// \tparam Impl The implementation of the interface.
template <typename Impl>
class RefTracker;

/// Forward declaration of a single-threaded reference tracker.
class SingleThreadedRefTracker;

/// Forward declaration of a multi-threaded reference tracker.
class MultiThreadedRefTracker;

/// Defines the type of the default reference tracker. The tracker is multi
/// threaded unless wrench is explicitly compiled for single threaded use.
using DefaultRefTracker =
#if defined(WRENCH_SINGLE_THREADED)
  SingleThreadedRefTracker;
#else
  MultiThreadedRefTracker;
#endif

/// Returns true if the type T is an implementation of the RefCounter
/// interface.
/// \tparam T The type to check if is a reference counter.
template <typename T>
static constexpr bool is_ref_tracker_v =
  std::is_base_of_v<RefTracker<std::decay_t<T>>, std::decay_t<T>>;

//==--- [implementation] ---------------------------------------------------==//

/// The RefTracker class defines an interface for reference counting, which can
/// be specialized for different contexts, and which allows the type requiring
/// tracking to clean up the resource as well.
///
/// \tparam Impl The implementation of the interface.
template <typename Impl>
class RefTracker {
  /// Returns a pointer to the implementation.
  wrench_no_discard auto impl() noexcept -> Impl* {
    return static_cast<Impl*>(this);
  }

  /// Returns a const pointer to the implementation.
  wrench_no_discard auto impl() const noexcept -> const Impl* {
    return static_cast<const Impl*>(this);
  }

 public:
  /// Adds a reference to the count.
  auto add_reference() noexcept -> void {
    impl()->add_reference_impl();
  }

  /// Decrements the referene count, returning true if the count is zero, and
  /// the resource can be released. If this returns true, then the resource
  /// should be destroyed through a call to `destroy()`.
  auto release() noexcept -> bool {
    return impl()->release_impl();
  }

  /// Destroys the resource \p resource, using the \p deleter, which should
  /// have a signature of:
  ///
  /// ~~~~~~~~~~~~~~~~~~~~
  /// auto deleter = [] (auto* resource) -> void {
  /// // delete the resource
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~
  ///
  /// \param  resource The resource to destroy.
  /// \param  deleter  The deleter for the resource.
  /// \tparam T        The type of the resource.
  /// \tparam Deleter  The type of the deleter.
  template <typename T, typename Deleter>
  auto destroy_resource(T* resource, Deleter&& deleter) noexcept -> void {
    impl()->destroy_resource_impl(resource, std::forward<Deleter>(deleter));
  }
};

//==--- [single-threaded implementation] -----------------------------------==//

/// This type implements a reference tracker which is not thread safe and is
/// designed for single threaded use. It can be embedded inside a class for
/// intrusive reference tracking.
///
/// This implements the RefTracker interface.
class SingleThreadedRefTracker : public RefTracker<SingleThreadedRefTracker> {
 public:
  /// Defines the type of the counter.
  using Counter = size_t;

  /// Adds a reference to the count.
  auto add_reference_impl() noexcept -> void {
    ref_count_++;
  }

  /// Decrements the referene count, returning true if the count is zero, and
  /// the resource can be released.
  auto release_impl() noexcept -> bool {
    return --ref_count_ == 0;
  }

  /// Destroys the resource \p resource, using the \p deleter, which should
  /// have a signature of:
  ///
  /// ~~~~~~~~~~~~~~~~~~~~
  /// auto destructor = [] (auto* resource) -> void {
  /// // free(resource);
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~
  ///
  /// \param  resource The resource to destroy.
  /// \param  deleter  The deleter for the resource.
  /// \tparam T        The type of the resource.
  /// \tparam Deleter  The type of the deleter.
  template <typename T, typename Deleter>
  auto destroy_resource_impl(T* resource, Deleter&& deleter) noexcept -> void {
    deleter(resource);
  }

 private:
  Counter ref_count_ = 1; //!< The reference count.
};

//==---[multi-threaded implementation] -------------------------------------==//

/// This type implements a reference tracker which is thread safe and is
/// designed for multi-threaded use. It can be embedded inside a class for
/// intrusive reference tracking.
///
/// This implements the RefTracker interface.
class MultiThreadedRefTracker : public RefTracker<MultiThreadedRefTracker> {
 public:
  /// Defines the type of the counter.
  using Counter = std::atomic_size_t;

  /// Constructor to initialize the reference count.
  MultiThreadedRefTracker() noexcept {
    ref_count_.store(1, std::memory_order_relaxed);
  }

  //==--- [move] -----------------------------------------------------------==//

  /// Adds to the reference count.
  auto add_reference_impl() noexcept -> void {
    // Memory order relaxed because new references can only be created from
    // existing instances with the reference count, so we just care about
    // incrementing the ref atomically, not about the memory ordering here.
    ref_count_.fetch_add(1, std::memory_order_relaxed);
  }

  /// Decrements the refernce count, and returns true if the resource being
  /// tracked with the reference count can be released. If this returns true,
  /// then the resource should be deleted by calling `destroy()`.
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

  /// Destroys the resource \p resource, using the \p deleter, which should
  /// have a signature of:
  ///
  /// ~~~~~~~~~~~~~~~~~~~~
  /// auto destructor = [] (auto* resource) -> void {
  /// // free(resource);
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~
  ///
  /// This creates a thread fence before deleting the resource so that there is
  /// a valid release-acquire sequence with `release()` and any access from
  /// other threads __happens before__ theis delete.
  ///
  /// Calling this when `release()` does not return `true` will invalidate all
  /// referenced objects.
  ///
  /// \param  resource The resource to destroy.
  /// \param  deleter  The deleter for the resource.
  /// \tparam T        The type of the resource.
  /// \tparam Deleter  The type of the deleter.
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
  Counter ref_count_ = 1; //!< The reference count.
};

} // namespace wrench

#endif // WRENCH_MEMORY_REF_TRACKER_HPP