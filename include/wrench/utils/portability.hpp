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
/// \file  portability.hpp
/// \brief This file defines proitability functionality.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_UTILS_PORTABILITY_HPP
#define WRENCH_UTILS_PORTABILITY_HPP

#if defined(__linux__)
  /// Defines a macro for linux
  #define wrench_linux 1
#endif

#if defined(__APPLE__) && (__MACH__)
  /// Defines a macro for osx
  #define wrench_osx 1
#endif

#if defined(wrench_osx) || defined(wrench_linux)
  /// Defines a macro for unix
  #define wrench_unix 1
#endif

#if __cplusplus == 201703L
  #define wrench_no_discard [[nodiscard]]
#else
  #define wrench_no_discard
#endif

#endif // WRENCH_UTILS_PORTABILITY_HPP
