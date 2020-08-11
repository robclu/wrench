
.. _program_listing_file_include_wrench_utils_portability.hpp:

Program Listing for File portability.hpp
========================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_utils_portability.hpp>` (``include/wrench/utils/portability.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/utils/portability.hpp ----------------------- -*- C++ -*- ---==//
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
   
   #ifndef WRENCH_UTILS_PORTABILITY_HPP
   #define WRENCH_UTILS_PORTABILITY_HPP
   
   #if defined(__linux__)
     #define wrench_linux 1
   #endif
   
   #if defined(__APPLE__) && (__MACH__)
     #define wrench_osx 1
   #endif
   
   #if defined(wrench_osx) || defined(wrench_linux)
     #define wrench_unix 1
   #endif
   
   #endif // WRENCH_UTILS_PORTABILITY_HPP
