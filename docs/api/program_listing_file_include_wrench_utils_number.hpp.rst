
.. _program_listing_file_include_wrench_utils_number.hpp:

Program Listing for File number.hpp
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_utils_number.hpp>` (``include/wrench/utils/number.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/utils/number.hpp ---------------------------- -*- C++ -*- ---==//
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
   
   #ifndef WRENCH_UTILS_NUMBER_HPP
   #define WRENCH_UTILS_NUMBER_HPP
   
   #include <cstddef>
   #include <cstdint>
   
   namespace wrench {
   
   template <size_t Value>
   struct Num {
     static constexpr auto value = size_t{Value};
   
     constexpr operator size_t() const noexcept {
       return Value;
     }
   };
   
   template <int64_t Value>
   struct Int64 {
     static constexpr auto value = int64_t{Value};
   
     constexpr operator int64_t() const noexcept {
       return Value;
     }
   };
   
   } // namespace wrench
   
   #endif // WRENCH_UTILS_NUMBER_HPP
