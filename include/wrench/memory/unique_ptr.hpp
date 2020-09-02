//==--- wrench/memory/unique_ptr.hpp ----------------------- -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  unique_ptr.hpp
/// \brief An implementation of a unique pointer which is similar to the
///        std::unique_ptr implementation, but with a much smaller compile-time
///        footprint.
//
//==------------------------------------------------------------------------==/

#ifndef WRENCH_MEMORY_UNIQUE_PTR_HPP
#define WRENCH_MEMORY_UNIQUE_PTR_HPP

#include <wrench/utils/portability.hpp>
#include <wrench/utils/type_traits.hpp>

namespace wrench {

/**
 * Default deleter implemenation. This is provided to remove the need for
 * std::default_delete which requires <memory>.
 *
 * \tparam T The type to delete.
 */
template <typename T>
struct DefaultDelete {
  /**
   * Default constructor to initialize the deleter.
   */
  constexpr DefaultDelete() noexcept = default;

  /**
   * Copy constructor, enabled if U is convertible to T.
   * \tparam U The type of the other object to delete.
   */
  template <typename U, typename = convertible_to_enable_t<U, T>>
  DefaultDelete(const DefaultDelete<U>&) noexcept {}

  /**
   * Move constructor, enabled if U is convertible to T.
   * \tparam U The type of the other object to delete.
   */
  template <typename U, typename D = convertible_to_enable_t<U, T>>
  DefaultDelete(DefaultDelete<U>&&) noexcept {}

  /**
   * Overload of opeator() to invoke the deleter, which deletes the \p ptr.
   * \param ptr The pointer to delete.
   */
  auto operator()(T* ptr) const -> void {
    delete ptr;
  }

  /**
   * Deleted call operator for a pointer of different type.
   * \param  ptr The pointer to delete.
   * \tparam U   The type of the pointer.
   */
  template <typename U>
  auto operator()(U* ptr) const -> void = delete;
};

/**
 * A unique pointer implementation with the same interface as std::unique_ptr
 * but with a much smaller compile-time footprint, since the <memory> header
 * is huge and takes a long time to compile.
 *
 * \note This implementation does not support arrays.
 *
 * \tparam T The type to wrap in a pointer.
 */
template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr : private Deleter {
  static_assert(!std::is_array_v<T>, "UniquePtr doesn't support arrays!");

  /**
   * Friend class for unique pointers with convertible types and deleters.
   * \tparam U The type of the other pointer.
   * \tparam D The type of the other deleter.
   */
  template <typename U, typename D>
  friend class UniquePtr;

  /**
   * Defines a valid type for a reference type.
   * \tparam U The type to get the copy type for.
   */
  template <typename U, typename V = std::remove_reference_t<U>>
  using CopyDeleter = std::conditional_t<std::is_reference_v<U>, V&, const V&>;

  /**
   * Defines a valid type for a reference type.
   * \tparam U The type to get the move type for.
   */
  template <typename U, typename V = std::remove_reference_t<U>>
  using MoveDeleter = std::conditional_t<std::is_reference_v<U>, V&&, V&&>;

  /**
   * Defines a valid type for a different deleter type.
   * \tparam U The type of the other deleter.
   */
  template <typename U, typename V = std::remove_reference_t<U>>
  using OtherDeleter = std::conditional_t<std::is_reference_v<U>, V&, V&&>;

 public:
  /**
   * Default constructor to set the pointer to a nullptr.
   * \param ptr The nullptr to initialze with.
   */
  UniquePtr(std::nullptr_t ptr = nullptr) noexcept : ptr_{ptr} {}

  /**
   * Constuctor to set the unique pointer to the \p ptr.
   * \param ptr The pointer to intialize the unique ptr with.
   */
  explicit UniquePtr(T* ptr) noexcept : ptr_{ptr} {}

  /**
   * Constuctor to set the unique pointer to the \p ptr.
   * \param  ptr     The pointer to intialize the unique ptr with.
   * \param  deleter The deleter for the unique pointer.
   * \tparam D       The type of the deleter.
   */
  template <typename D>
  explicit UniquePtr(T* ptr, const D& deleter) noexcept
  : Deleter{std::forward<CopyDeleter<D>>(deleter)}, ptr_{ptr} {}

  /**
   * Constuctor to set the unique pointer to the \p ptr.
   * \param ptr The pointer to intialize the unique ptr with.
   */
  template <typename D>
  explicit UniquePtr(T* ptr, D&& deleter) noexcept
  : Deleter{std::forward<MoveDeleter<D>>(deleter)}, ptr_{ptr} {}

  /**
   * Move constructor to move the \p other pointer into this one, along with the
   * other pointer's deleter.
   *
   * \note This will fail if the type U is not a base of T.
   *
   * \post This pointer holds the pointer, \p other holds a nullptr.
   *
   * \param  other The other pointer to move into this one.
   * \tparam U     The type of the other pointer.
   * \tparam D     The type of the other deleter.
   * \return A unique pointer which points to \p other's pointer.
   */
  template <typename U, typename D, typename E = base_of_enable_t<T, U>>
  UniquePtr(UniquePtr<U, D>&& other) noexcept
  : Deleter{std::forward<OtherDeleter<D>>(other.get_deleter())},
    ptr_{other.ptr_} {
    other.ptr_ = nullptr;
  }

  /**
   * Constructor to initialize the pointer from another unique pointer.
   *
   * \post This pointer holds the pointer, \p other holds a nullptr.
   *
   * \param  other The other pointer to move into this one.
   * \return A unique pointer which points to \p other's pointer.
   */
  UniquePtr(UniquePtr&& other) noexcept
  : Deleter{std::move(other.get_deleter())}, ptr_{other.ptr_} {
    other.ptr_ = nullptr;
  }

  /**
   * Destructor to delete the owned pointer.
   */
  ~UniquePtr() {
    get_deleter()(ptr_);
  }

  /**
   * Move assignment operator to create the pointer from the \p other unique
   * pointer.
   *
   * \post This pointer holds the pointer, \p other holds a nullptr.
   *
   * \param other The other pointer to move into this one.
   * \return A unique pointer which points to \p other's pointer.
   */
  auto operator=(UniquePtr&& other) noexcept -> UniquePtr& {
    ptr_       = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }

  /**
   * Move assignment operator to create the pointer from the \p other unique
   * pointer.
   *
   * \post This pointer holds the pointer, \p other holds a nullptr.
   *
   * \param other The other pointer to move into this one.
   * \return A unique pointer which points to \p other's pointer.
   */
  template <
    typename U,
    typename D,
    typename Del = OtherDeleter<D>,
    base_of_enable_t<T, U>>
  auto operator=(UniquePtr<U, D>&& other) noexcept -> UniquePtr& {
    ptr_          = other.ptr_;
    get_deleter() = std::forward<Del>(other.get_deleter());
    other.ptr_    = nullptr;
    return *this;
  }

  /*==--- [deleted] --------------------------------------------------------==*/

  /** Copy constructor -- deleted. */
  UniquePtr(const UniquePtr&) = delete;

  /** Copy assignment -- deleted.  */
  auto operator=(const UniquePtr&) = delete;

  /*==--- [operator overloads] ---------------------------------------------==*/

  /**
   * Overload of equality operator to check if the pointer is a nullptr.
   * \return __true__ if the pointer is a nullptr.
   */
  auto operator==(std::nullptr_t) const -> bool {
    return ptr_ == nullptr;
  }

  /**
   * Overload of equality operator to check if the pointer is not a nullptr.
   * \return __true__ if the pointer is not a nullptr.
   */
  auto operator!=(std::nullptr_t) const {
    return ptr_ != nullptr;
  }

  /**
   * Overload of conversion to bool operator.
   * \return __true__ if the pointer is not a nullptr.
   */
  explicit operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  // clang-format off
  /**
   * Gets the owned pointer.
   * \return The owned pointer.
   */
  auto operator->() noexcept -> T* {
    assert(ptr_ && "Accessed nullptr in UniquePtr!");
    return ptr_;
  }

  /**
   * Gets the owned pointer.
   * \return A const pointer to the owned pointer.
   */
  auto operator->() const noexcept -> const T* {
    assert(ptr_ && "Accessed nullptr in UniquePtr!");
    return ptr_;
  }
  // clang-format on

  /**
   * Gets a reference to the owned type.
   * \return A reference to the owned object.
   */
  auto operator*() noexcept -> T& {
    assert(ptr_ && "Accessed nullptr in UniquePtr!");
    return *ptr_;
  }

  /**
   * Gets a reference to the owned type.
   * \return A const reference to the owned pointer.
   */
  auto operator*() const noexcept -> const T& {
    assert(ptr_ && "Accessed nullptr in UniquePtr!");
    return *ptr_;
  }

  /*==--- [inteface] -------------------------------------------------------==*/

  /**
   * Gets a pointer to the owned pointer.
   * \return The owned pointer.
   */
  auto get() noexcept -> T* {
    return ptr_;
  }

  /**
   * Gets a const pointer to the owned pointer.
   * \return The owned pointer.
   */
  auto get() const noexcept -> const T* {
    return ptr_;
  }

  /**
   * Gets a reference to the deleter.
   * \return A reference to the deleter.
   */
  auto get_deleter() noexcept -> Deleter& {
    return *this;
  }

  /**
   * Gets a const reference to the deleter.
   * \return A const reference to the deleter.
   */
  auto get_deleter() const noexcept -> const Deleter& {
    return *this;
  }

  /**
   * Resets the owned pointer, freeing it and setting it to \p ptr.
   * \param ptr The new pointer to set this pointer to.
   */
  auto reset(T* ptr = nullptr) noexcept -> void {
    get_deleter()(ptr_);
    ptr_ = ptr;
  }

  /**
   * Releases the owned pointer.
   * \return The owned pointer.
   */
  auto release() noexcept -> T* {
    T* const out = ptr_;
    ptr_         = nullptr;
    return out;
  }

 private:
  T* ptr_ = nullptr; //!< The owned pointer.
};

/**
 * Returns a UniquePointer to a newly allocated type T, constructing T with the
 * given \p args.
 *
 * \note This method does not allow creation of a UniquePtr with a custom
 *       deleter. For such a case, the created object is usually allocated
 *       before creation of the UniquePtr, so this method isn't required.
 *       For that case, create the unique pointer directly.
 *
 * \note This may throw bad_alloc if the allocation of T fails.
 *
 * \param  args The args for construcion of T.
 * \tparam T    The type of the object to create.
 * \tparam Args The type of the arguments for the constructor.
 */
template <typename T, typename... Args>
auto make_unique(Args&&... args) -> UniquePtr<T> {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

} // namespace wrench

#endif //  WRENCH_MEMORY_UNIQUE_PTR_HPP
