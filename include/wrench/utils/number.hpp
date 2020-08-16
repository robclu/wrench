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
/// \file  number.hpp
/// \brief This file defines a class which represents a compile time constant
///        number type, where a number can be represented as at compile time
///        as both a type and a value.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_UTILS_NUMBER_HPP
#define WRENCH_UTILS_NUMBER_HPP

#include <cstddef>
#include <cstdint>

namespace wrench {

/// The Num struct stores a number as a type, but also provides functionality to
/// convert the value of the number into a numeric type at runtime or compile
/// time. It is useful in a metaprogramming context to use as a type, but also
/// when the numeric value of the type is required. It works better with
/// variadic templates than using a raw numeric literal.
/// \tparam Value The value of the number.
template <size_t Value>
struct Num {
  /// Returns the value of the index.
  static constexpr auto value = size_t{Value};

  /// Conversion to size_t so that the number can be used as a size type.
  constexpr operator size_t() const noexcept {
    return Value;
  }
};

/// The Int64 struct stores an integer as a type, but also provides
/// functionality to convert the value of the Int64 into a 64 bit integer type
/// at runtime or compile time. It is useful in a metaprogramming context to use
/// as a type, but also when the numeric value of the type is required. It works
/// better with variadic templates than using a raw numeric literal.
/// \tparam Value The value of the number.
template <int64_t Value>
struct Int64 {
  /// Returns the value of the index.
  static constexpr auto value = int64_t{Value};

  /// Conversion to size_t so that the number can be used as a size type.
  constexpr operator int64_t() const noexcept {
    return Value;
  }
};

} // namespace wrench

#endif // WRENCH_UTILS_NUMBER_HPP