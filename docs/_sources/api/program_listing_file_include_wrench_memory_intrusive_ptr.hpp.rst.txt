
.. _program_listing_file_include_wrench_memory_intrusive_ptr.hpp:

Program Listing for File intrusive_ptr.hpp
==========================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_memory_intrusive_ptr.hpp>` (``include/wrench/memory/intrusive_ptr.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_MEMORY_INTRUSIVE_PTR_HPP
   #define WRENCH_MEMORY_INTRUSIVE_PTR_HPP
   
   #include "ref_tracker.hpp"
   #include <memory>
   
   namespace wrench {
   
   //==--- [forward declarations & aliases] -----------------------------------==//
   
   template <typename T>
   class IntrusivePtr;
   
   template <
     typename T,
     typename Deleter          = std::default_delete<T>,
     typename ReferenceTracker = DefaultRefTracker>
   class IntrusivePtrEnabled;
   
   template <typename T, typename Deleter = std::default_delete<T>>
   using SingleThreadedIntrusivePtrEnabled =
     IntrusivePtrEnabled<T, Deleter, SingleThreadedRefTracker>;
   
   template <typename T, typename Deleter = std::default_delete<T>>
   using MultiThreadedIntrusivePtrEnabled =
     IntrusivePtrEnabled<T, Deleter, MultiThreadedRefTracker>;
   
   template <typename T, typename... Args>
   auto make_intrusive_ptr(Args&&... args) -> IntrusivePtr<T>;
   
   template <typename T, typename Allocator, typename... Args>
   auto allocate_intrusive_ptr(Allocator& allocator, Args&&... args)
     -> IntrusivePtr<T>;
   
   //==--- [intrusive ptr enable] ---------------------------------------------==//
   
   // Implementation of IntrusivePtrEnabled.
   // \tparam T                The type of the pointer.
   // \tparam Deleter          The type of the deleter for the object.
   // \tparam ReferenceTracker The type of the refrence tracker.
   // Implementation of IntrusivePtrEnabled.
   template <typename T, typename Deleter, typename ReferenceTracker>
   class IntrusivePtrEnabled {
     static_assert(
       is_ref_tracker_v<ReferenceTracker>,
       "Reference tracker for intrusive ptr enabled type must implement the "
       "RefTracker interface.");
   
     using Self = IntrusivePtrEnabled;
   
    public:
     //==--- [aliases] --------------------------------------------------------==//
   
     // clang-format off
     using Enabled          = T;
     using IntrusivePointer = IntrusivePtr<Enabled>;
     using DeleterType      = Deleter;
     using RefTracker       = ReferenceTracker;
     // clang-format on
   
     //==--- [construction] ---------------------------------------------------==//
   
     IntrusivePtrEnabled() = default;
   
     // clang-format off
     IntrusivePtrEnabled(IntrusivePtrEnabled&& other) noexcept = default;
   
     auto operator=(IntrusivePtrEnabled&& other) noexcept -> IntrusivePtrEnabled& 
       = default;
   
     //==--- [deleted] --------------------------------------------------------==//
   
     IntrusivePtrEnabled(const IntrusivePtrEnabled&) = delete;
     auto operator=(const IntrusivePtrEnabled&)      = delete;
     // clang-format on
   
     //==--- [implementation] -------------------------------------------------==//
   
     auto release_reference() noexcept -> void {
       if (ref_tracker_.release()) {
         ref_tracker_.destroy_resource(static_cast<Enabled*>(this), DeleterType());
       }
     }
   
     void add_reference() noexcept {
       ref_tracker_.add_reference();
     }
   
    protected:
     auto reference_from_this() noexcept -> IntrusivePointer;
   
    private:
     RefTracker ref_tracker_; 
   };
   
   template <typename T>
   static constexpr bool is_intrusive_ptr_enabled_v =
     std::is_base_of_v<IntrusivePtrEnabled<std::decay_t<T>>, std::decay_t<T>>;
   
   //==--- [intrusive pointer] ------------------------------------------------==//
   
   // IntrusivePtr imlpementation.
   // \tparam T The type to wrap in an intrusive pointer.
   template <typename T>
   class IntrusivePtr {
     static_assert(
       is_intrusive_ptr_enabled_v<T>,
       "Type for IntrusivePtr must be a subclass of IntrusivePtrEnabled");
   
     template <typename U>
     friend class IntrusivePtr;
   
    public:
     //==--- [aliases] --------------------------------------------------------==//
   
     using Ptr      = T*;       
     using Ref      = T&;       
     using ConstPtr = const T*; 
     using ConstRef = const T&; 
   
     using IntrusiveEnabledBase = IntrusivePtrEnabled<
       typename T::Enabled,
       typename T::DeleterType,
       typename T::RefTracker>;
   
     //==--- [construction] ---------------------------------------------------==//
   
     IntrusivePtr() noexcept = default;
   
     explicit IntrusivePtr(Ptr data) noexcept : data_(data) {}
   
     IntrusivePtr(const IntrusivePtr& other) noexcept = default;
   
     IntrusivePtr(IntrusivePtr&& other) noexcept = default;
   
     template <typename U>
     IntrusivePtr(const IntrusivePtr<U>& other) noexcept {
       static_assert(
         std::is_base_of_v<T, U> || std::is_convertible_v<U, T>,
         "Types of pointed to data for the intrusive pointer are not compatible.");
       *this = other;
     }
   
     template <typename U>
     IntrusivePtr(IntrusivePtr<U>&& other) noexcept {
       static_assert(
         std::is_base_of_v<T, U> || std::is_convertible_v<U, T>,
         "Types of pointed to data for the intrusive pointer are not compatible.");
       *this = std::move(other);
     }
   
     ~IntrusivePtr() noexcept {
       reset();
     }
   
     //==--- [operator overloads] ---------------------------------------------==//
   
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
   
     auto operator=(IntrusivePtr&& other) noexcept -> IntrusivePtr& {
       if (this != &other) {
         reset();
         data_       = other.data_;
         other.data_ = nullptr;
       }
       return *this;
     }
   
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
   
     auto operator*() noexcept -> Ref {
       return *data_;
     }
     auto operator*() const noexcept -> ConstRef {
       return *data_;
     }
   
     // clang-format off
     auto operator->() noexcept -> Ptr {
       return data_;
     }
     auto operator->() const noexcept -> ConstPtr {
       return data_;
     }
     // clang-format on
   
     auto get() noexcept -> Ptr {
       return data_;
     }
   
     auto get() const noexcept -> ConstPtr {
       return data_;
     }
   
     //==--- [conparison ops] -------------------------------------------------==//
   
     explicit operator bool() const noexcept {
       return data_ != nullptr;
     };
   
     auto operator==(const IntrusivePtr& other) const noexcept -> bool {
       return data_ == other.data_;
     }
     auto operator!=(const IntrusivePtr& other) const noexcept -> bool {
       return data_ != other.data_;
     }
   
     //==--- [reset] ----------------------------------------------------------==//
   
     auto reset() noexcept -> void {
       if (data_) {
         as_intrusive_enabled()->release_reference();
         data_ = nullptr;
       }
     }
   
    private:
     Ptr data_ = nullptr; 
   
     auto as_intrusive_enabled() noexcept -> IntrusiveEnabledBase* {
       static_assert(
         std::is_convertible_v<T*, IntrusiveEnabledBase*>,
         "IntrusivePtr requires type T to implement the IntrusivePtrEnabled "
         "interface.");
       return static_cast<IntrusiveEnabledBase*>(data_);
     }
   };
   
   //==--- [intrusive ptr enabled implemenatations] ---------------------------==//
   
   template <typename T, typename Deleter, typename Tracker>
   auto IntrusivePtrEnabled<T, Deleter, Tracker>::reference_from_this() noexcept
     -> IntrusivePtr<T> {
     add_reference();
     return IntrusivePtr<T>(static_cast<T*>(this));
   }
   
   //==--- [helper implementations] -------------------------------------------==//
   
   // Implementation of the intrusive pointer creation function.
   // \param  args The args for construction of the type T.
   // \tparam T    The type to create an intrusive pointer for.
   // \tparam Args The types of the construction arguments.
   template <typename T, typename... Args>
   auto make_intrusive_ptr(Args&&... args) -> IntrusivePtr<T> {
     return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
   }
   
   // Implementation of intrusive pointer allocation creation.
   // \param  allocator The allocator to allocate the data with.
   // \param  args      The arguments for the construction of the pointer.
   // \tparam T         The type of the intrusive pointed to type.
   // \tparam Args      The type of the args.
   template <typename T, typename Allocator, typename... Args>
   auto allocate_intrusive_ptr(Allocator& allocator, Args&&... args)
     -> IntrusivePtr<T> {
     void* const p = allocator.alloc(sizeof(T), alignof(T));
     return IntrusivePtr<T>(new (p) T(std::forward<Args>(args)...));
   }
   
   } // namespace wrench
   
   #endif // WRENCH_MEMORY_INTRUSIVE_PTR_HPP
