
.. _program_listing_file_include_wrench_algorithm_detail_unrolled_for_impl_.hpp:

Program Listing for File unrolled_for_impl_.hpp
===============================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_algorithm_detail_unrolled_for_impl_.hpp>` (``include/wrench/algorithm/detail/unrolled_for_impl_.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/algorithm/detail/unrolled_for_impl_.hpp ----- -*- C++ -*- ---==//
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
   
   #ifndef WRENCH_ALGORITHM_DETAIL_UNROLLED_FOR_IMPL__HPP
   #define WRENCH_ALGORITHM_DETAIL_UNROLLED_FOR_IMPL__HPP
   
   #include <wrench/utils/number.hpp>
   #include <utility>
   
   namespace wrench::detail {
   
   template <size_t Amount>
   struct Unroll : Unroll<(Amount <= 1 ? 0 : Amount - 1)> {
     static constexpr size_t previous_level = (Amount <= 1 ? 0 : Amount - 1);
   
     using PreviousLevel = Unroll<previous_level>;
   
     template <typename Functor, typename... Args>
     constexpr Unroll(Functor&& functor, Args&&... args) noexcept
     : PreviousLevel(std::forward<Functor>(functor), std::forward<Args>(args)...) {
       functor(Num<previous_level>(), std::forward<Args>(args)...);
     }
   };
   
   template <>
   struct Unroll<1> {
     template <typename Functor, typename... Args>
     constexpr Unroll(Functor&& functor, Args&&... args) noexcept {
       functor(Num<0>(), std::forward<Args>(args)...);
     }
   };
   
   template <>
   struct Unroll<0> {
     template <typename Functor, typename... Args>
     constexpr Unroll(Functor&& functor, Args&&... args) noexcept {}
   };
   
   } // namespace wrench::detail
   
   #endif // WRENCH_ALGORITHM_DETAIL_UNROLLED_FOR_IMPL__HPP
