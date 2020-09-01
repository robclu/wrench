//==--- wrench/utils/type_traits.hpp ----------------------- -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  type_traits.hpp
/// \brief This file defines type traits implementation..
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_UTILS_TYPE_TRAITS_HPP
#define WRENCH_UTILS_TYPE_TRAITS_HPP

#include <type_traits>

/**
 * Defines a valid type if Derived is a base of Base.
 * \tparam Base    The base type for the enable.
 * \tparam Derived The derived type.
 */
template <typename Base, typename Derived>
using base_of_enable_t =
  std::enable_if_t<std::is_base_of_v<Base, Derived>, int>;

/**
 * Defines a valid type if From is convertible to To.
 * \tparam From The type to test if converitble from.
 * \tparam To   The type to test if converitble to.
 */
template <typename From, typename To>
using convertible_to_enable_t =
  std::enable_if_t<std::is_convertible_v<From, To>, int>;

#endif // WRENCH_UTILS_TYPE_TRAITS_HPP