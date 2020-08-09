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

/// Defines a macro for linux
#ifndef wrench_linux
  #define wrench_linux __linux__
#endif

/// Defines a macro for osx
#ifndef wrench_osx
  #define wrench_osx (__APPLE__ && __MACH__)
#endif

/// Defines a macro for unix
#ifndef wrench_unix
  #define wrench_unix (wrench_linux || wrench_osx)
#endif
// clang-format on

#endif // WRENCH_UTILS_PORTABILITY_HPP
