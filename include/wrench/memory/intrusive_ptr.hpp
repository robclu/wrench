//==--- wrench/memory/intrusive_ptr.hpp -------------------- -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  intrusive_ptr.hpp
/// \brief This file defines an intrusive shared pointer class.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MEMORY_INTRUSIVE_POINTER_HPP
#define WRENCH_MEMORY_INTRUSIVE_POINTER_HPP

#include "ref_tracker.hpp"
#include <memory>

namespace wrench {

//==--- [forward declarations & aliases] -----------------------------------==//

/// Forwrad declaration of an intrusive pointer.
/// \tparam T The type to wrap in an intrusive pointer.
template <typename T>
class IntrusivePtr;

/// Provides reference tracking and deleting functionality which can be
/// inherited to enable IntrusivePtr functionality.
/// \tparam T                The type of the pointer.
/// \tparam Deleter          The type of the deleter for the object.
/// \tparam ReferenceTracker The type of the refrence tracker.
template <
  typename T,
  typename Deleter          = std::default_delete<T>,
  typename ReferenceTracker = DefaultRefTracker>
class IntrusivePtrEnabled;

/// Alias for explicit single threaded intrusive pointer enable.
/// \tparam T       The type to enable intrusive pointer functionality for.
/// \tparam Deleter The type of the deleter.
template <typename T, typename Deleter = std::default_delete<T>>
using SingleThreadedIntrusivePtrEnabled =
  IntrusivePtrEnabled<T, Deleter, SingleThreadedRefTracker>;

/// Alias for explicit multi threaded intrusive pointer enable.
/// \tparam T       The type to enable intrusive pointer functionality for.
/// \tparam Deleter The type of the deleter.
template <typename T, typename Deleter = std::default_delete<T>>
using MultiThreadedIntrusivePtrEnabled =
  IntrusivePtrEnabled<T, Deleter, MultiThreadedRefTracker>;

/// Creates an intrusive pointer of type `IntrusivePtr<T>`, using the \p args to
/// construct the type T.
/// \param  args The args for construction of the type T.
/// \tparam T    The type to create an intrusive pointer for.
/// \tparam Args The types of the construction arguments.
template <typename T, typename... Args>
auto make_intrusive_ptr(Args&&... args) -> IntrusivePtr<T>;

//==--- [intrusive ptr enable] ---------------------------------------------==//

/// Provides reference tracking and deleting functionality which can be
/// inherited to enable IntrusivePtr functionality.
/// \tparam T                The type of the pointer.
/// \tparam Deleter          The type of the deleter for the object.
/// \tparam ReferenceTracker The type of the refrence tracker.
template <typename T, typename Deleter, typename ReferenceTracker>
class IntrusivePtrEnabled {
  static_assert(
    is_ref_tracker_v<ReferenceTracker>,
    "Reference tracker for intrusive ptr enabled type must implement the "
    "RefTracker interface.");

  /// Defines the type of this class.
  using Self = IntrusivePtrEnabled;

 public:
  //==--- [aliases] --------------------------------------------------------==//

  // clang-format off
  /// Defines the type of the base which requires the functionality.
  using Enabled          = T;
  /// Defines the type of the intrusive pointer.
  using IntrusivePointer = IntrusivePtr<Enabled>;
  /// Defines the type of the deleter.
  using DeleterType      = Deleter;
  /// Defines the type of the reference tracker.
  using RefTracker       = ReferenceTracker;
  // clang-format on

  //==--- [construction] ---------------------------------------------------==//

  /// Constructor, which initializes the count and checks that the reference
  /// tracker implements the RefTracker interface.
  IntrusivePtrEnabled() = default;

  // clang-format off
  /// Here we enable move semantics as a performance optimization, which results
  /// in less atomics than just copying.
  /// \param other The other type to move.
  IntrusivePtrEnabled(IntrusivePtrEnabled&& other) noexcept = default;

  /// Here we enable move semantics as a performance optimization. It would be
  /// quite fine to just copy the intrusive pointer and then deleted the old
  /// one. However, in a multi-threaded context, this will result in two atomic
  /// operations, when we can just move the reference tracker and have none.
  /// \param other The other type to move.
  auto operator=(IntrusivePtrEnabled&& other) noexcept -> IntrusivePtrEnabled& 
    = default;

  //==--- [deleted] --------------------------------------------------------==//

  /// Copy constructor -- deleted.
  IntrusivePtrEnabled(const IntrusivePtrEnabled&) = delete;
  /// Copy assignment operator -- deleted.
  auto operator=(const IntrusivePtrEnabled&)      = delete;
  // clang-format on

  //==--- [implementation] -------------------------------------------------==//

  /// Releases the reference to the pointed to object, deleting the object if
  /// the reference count gets to zero.
  auto release_reference() noexcept -> void {
    if (ref_tracker_.release()) {
      ref_tracker_.destroy_resource(static_cast<Enabled*>(this), DeleterType());
    }
  }

  /// Adds a reference to the tracked reference count.
  void add_reference() noexcept {
    ref_tracker_.add_reference();
  }

 protected:
  /// Creates a new intrusive pointer from the pointed to object, incrementing
  /// the reference count.
  auto reference_from_this() noexcept -> IntrusivePointer;

 private:
  RefTracker ref_tracker_; //!< The reference tracker.
};

/// Returns true if the type T is intrusive ptr enabled.
/// \tparam T The type to determine if is intrusive pointer enabled.
template <typename T>
static constexpr bool is_intrusive_ptr_enabled_v =
  std::is_base_of_v<IntrusivePtrEnabled<std::decay_t<T>>, std::decay_t<T>>;

//==--- [intrusive pointer] ------------------------------------------------==//

/// The IntrusivePtr type is a shared pointer implementation which is
/// intrusive. The reference count is stored in the intrusive pointer. It has
/// a smaller memroy footprint than `shared_ptr` and usually gives better
/// performance.
///
/// It additionally requires classes to inherit from `IntrusivePtrEnable<T,
/// Deleter, ReferenceType>`, which allows for custom specialization of the
/// deleter and referenc types, for single/multi-threaded use cases.
///
/// For a non-thread-safe, single-threaded optimized referencing, use
/// `SingleThreadedRefTracker`, while for a thread-safe, multi-threaded
/// optimized referencing, using `MultiThreadedRefTracker`.
///
/// Instances of instrusive pointer types should be create with
/// `make_intrusive_ptr(...)`, rather than through direct construction.
///
/// \tparam T The type to wrap in an intrusive pointer.
template <typename T>
class IntrusivePtr {
  static_assert(
    is_intrusive_ptr_enabled_v<T>,
    "Type for IntrusivePtr must be a subclass of IntrusivePtrEnabled");

  /// Enable access for intrusive pointer type with different templates.
  template <typename U>
  friend class IntrusivePtr;

 public:
  //==--- [aliases] --------------------------------------------------------==//

  using Ptr      = T*;       //!< Pointer type.
  using Ref      = T&;       //!< Reference type.
  using ConstPtr = const T*; //!< Const pointer type.
  using ConstRef = const T&; //!< Const reference type.

  /// Defines the type of intrusive enabled base for the type T.
  using IntrusiveEnabledBase = IntrusivePtrEnabled<
    typename T::Enabled,
    typename T::DeleterType,
    typename T::RefTracker>;

  //==--- [construction] ---------------------------------------------------==//

  /// Default constructor.
  IntrusivePtr() noexcept = default;

  /// Constructor which takes a pointer \p ptr.
  /// \param data A pointer to the data.
  explicit IntrusivePtr(Ptr data) noexcept : data_(data) {}

  /// Copy constructor to create the intrusive pointer from \p other.
  IntrusivePtr(const IntrusivePtr& other) noexcept = default;

  /// Move the \p other intrusive pointer into this one.
  /// \param other The other intrusive pointer to move into this one.
  IntrusivePtr(IntrusivePtr&& other) noexcept = default;

  /// Copy constructor to create the intrusive pointer from \p other. This will
  /// fail at compile time if U is not derived from T, or convertible to T.
  /// \param  other The other intrusive pointer to copy from.
  /// \tparam U     The type of the other's pointed to data.
  template <typename U>
  IntrusivePtr(const IntrusivePtr<U>& other) noexcept {
    static_assert(
      std::is_base_of_v<T, U> || std::is_convertible_v<U, T>,
      "Types of pointed to data for the intrusive pointer are not compatible.");
    *this = other;
  }

  /// Moves the \p other intrusive pointer into this one. This will fail at
  /// compile time if U is not derived from T or is convertible to T.
  /// \param other The other type to move into this one.
  template <typename U>
  IntrusivePtr(IntrusivePtr<U>&& other) noexcept {
    static_assert(
      std::is_base_of_v<T, U> || std::is_convertible_v<U, T>,
      "Types of pointed to data for the intrusive pointer are not compatible.");
    *this = std::move(other);
  }

  /// Destructor to clean up the pointer.
  ~IntrusivePtr() noexcept {
    reset();
  }

  //==--- [operator overloads] ---------------------------------------------==//

  /// Copy assignment operator to set the intrusive pointer from the \p other
  /// intrusive pointer.
  /// \param other The other pointer to set this one from.
  auto operator=(const IntrusivePtr& other) noexcept -> IntrusivePtr& {
    if (this != &other) {
      reset(); // Reset incase this points to something valid.

      data_ = other.data_;
      if (data_) {
        as_intrusive_enabled()->add_reference();
      }
    }
    return *this;
  }

  /// Move assignment operator to move the \p other intrusive pointer into
  /// this one.
  /// \param other The other pointer to move into this one.
  auto operator=(IntrusivePtr&& other) noexcept -> IntrusivePtr& {
    if (this != &other) {
      reset();
      data_       = other.data_;
      other.data_ = nullptr;
    }
    return *this;
  }

  /// Copy assignment operator to set the intrusive pointer from the \p other
  /// intrusive pointer, which wraps a __different__ type to this one. If the
  /// type of the \p other is not a base of T, or convertible to T, then this
  /// will cause a compile time error.
  /// \param  other The other pointer to set this one from.
  /// \tparam U     The type of the other pointer.
  template <typename U>
  auto operator=(const IntrusivePtr<U>& other) -> IntrusivePtr& {
    static_assert(
      std::is_base_of_v<T, U> || std::is_convertible_v<U, T>,
      "Types of pointed to data for the intrusive pointer are not compatible.");

    // Reset incase this class points to valid data:
    reset();
    data_ = static_cast<Ptr>(other.data_);

    if (data_) {
      as_intrusive_enabled()->add_reference();
    }
    return *this;
  }

  /// Move assignment operator to move the \p other intrusive pointer into this
  /// one. This will fail if U is not derived from T or convertible to T.
  /// \param  other The other pointer to set this one from.
  /// \tparam U     The type of the other pointer.
  template <typename U>
  auto operator=(IntrusivePtr<U>&& other) noexcept -> IntrusivePtr& {
    static_assert(
      std::is_base_of_v<T, U> || std::is_convertible_v<U, T>,
      "Types of pointed to data for the intrusive pointer are not compatible.");

    reset();
    data_       = static_cast<Ptr>(other.data_);
    other.data_ = nullptr;
    return *this;
  }

  //==--- [access] ---------------------------------------------------------==//

  /// Returns a reference to the data.
  auto operator*() noexcept -> Ref {
    return *data_;
  }
  /// Returns a const reference to the data.
  auto operator*() const noexcept -> ConstRef {
    return *data_;
  }

  // clang-format off
  /// Returns a pointer to the data.
  auto operator->() noexcept -> Ptr {
    return data_;
  }
  /// Returns a const pointer to the data.
  auto operator->() const noexcept -> ConstPtr {
    return data_;
  }
  // clang-format on

  /// Returns a pointer to the data.
  auto get() noexcept -> Ptr {
    return data_;
  }

  /// Returns a const pointer to the data.
  auto get() const noexcept -> ConstPtr {
    return data_;
  }

  //==--- [conparison ops] -------------------------------------------------==//

  /// Returns true if the data is not a nullptr.
  explicit operator bool() const noexcept {
    return data_ != nullptr;
  };

  /// Returns true if the pointer to \p other's data is the same as the pointer
  /// to this data.
  /// \param other The other pointer to compare with.
  auto operator==(const IntrusivePtr& other) const noexcept -> bool {
    return data_ == other.data_;
  }
  /// Returns true if the pointer to \p other's data is not the same as the
  /// pointer to this data.
  /// \param other The other pointer to compare with.
  auto operator!=(const IntrusivePtr& other) const noexcept -> bool {
    return data_ != other.data_;
  }

  //==--- [reset] ----------------------------------------------------------==//

  /// Resets the intrusive pointer by releasing the reference, and resetting the
  /// pointer to the data.
  auto reset() noexcept -> void {
    if (data_) {
      as_intrusive_enabled()->release_reference();
      data_ = nullptr;
    }
  }

 private:
  Ptr data_ = nullptr; //!< Pointer to the data.

  /// Returns a pointer to the upcasted intrusive pointer enabled base class.
  auto as_intrusive_enabled() noexcept -> IntrusiveEnabledBase* {
    static_assert(
      std::is_convertible_v<T*, IntrusiveEnabledBase*>,
      "IntrusivePtr requires type T to implement the IntrusivePtrEnabled "
      "interface.");
    return static_cast<IntrusiveEnabledBase*>(data_);
  }
};

//==--- [intrusive ptr enabled implemenatations] ---------------------------==//

/// Implementation for creating a reference from an intrusive pointer enbled
/// type.
/// \tparam T       The to create an intrusive pointer for.
/// \tparam Deleter The deleter for the object.
/// \tparam Tracker The type of the reference tracker.
template <typename T, typename Deleter, typename Tracker>
auto IntrusivePtrEnabled<T, Deleter, Tracker>::reference_from_this() noexcept
  -> IntrusivePtr<T> {
  add_reference();
  return IntrusivePtr<T>(static_cast<T*>(this));
}

//==--- [helper implementations] -------------------------------------------==//

/// Implementation of the intrusive pointer creation function.
/// \param  args The arguments for the construction of the intrusive pointer.
/// \tparam T    The type of the intrusive pointed to type.
/// \tparam Args The type of the args.
template <typename T, typename... Args>
auto make_intrusive_ptr(Args&&... args) -> IntrusivePtr<T> {
  return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}

/// Implementation of the intrusive pointer allocation function. This uses the
/// \p allocator to allocate the data for the object. The type T must be a base
/// of IntrsivePtrEnable<T, Deleter> with a Deleter type which uses the given
/// \p allocator to destroy the data.
///
/// The allocator must have an `alloc(size, alignment)` method.
///
/// \param  allocator The allocator to allocate the data with.
/// \param  args      The arguments for the construction of the pointer.
/// \tparam T         The type of the intrusive pointed to type.
/// \tparam Args      The type of the args.
template <typename T, typename Allocator, typename... Args>
auto allocate_intrusive_ptr(Allocator& allocator, Args&&... args)
  -> IntrusivePtr<T> {
  void* const p = allocator.alloc(sizeof(T), alignof(T));
  return IntrusivePtr<T>(new (p) T(std::forward<Args>(args)...));
}

} // namespace wrench

#endif // WRENCH_MEMORY_INTRUSIVE_PTR_HPP