
.. _program_listing_file_include_wrench_algorithm_unrolled_for.hpp:

Program Listing for File unrolled_for.hpp
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_algorithm_unrolled_for.hpp>` (``include/wrench/algorithm/unrolled_for.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/algorithm/unrolled_for.hpp ------------------ -*- C++ -*- ---==//
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
   
   #ifndef WRENCH_ALGORITHM_UNROLLED_FOR_HPP
   #define WRENCH_ALGORITHM_UNROLLED_FOR_HPP
   
   #include "detail/unrolled_for_impl_.hpp"
   
   namespace wrench {
   
   template <size_t Amount, typename Functor, typename... Args>
   constexpr inline auto
   unrolled_for(Functor&& functor, Args&&... args) noexcept -> void {
     detail::Unroll<Amount> unrolled(
       std::forward<Functor>(functor), std::forward<Args>(args)...);
   }
   
   } // namespace wrench
   
   #endif // WRENCH_ALGORITHM_UNROLLED_FOR_HPP
