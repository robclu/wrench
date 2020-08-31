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

/**
 * Defines a valid type if Derived is a base of Base.
 * \tparam Base    The base type for the enable.
 * \tparam Derived The derived type.
 */
template <typename Base, typename Derived>
using base_of_enable_t = std::enable_if_t<std::base_of_v<Base, Derived>, int>;

#endif // WRENCH_UTILS_TYPE_TRAITS_HPP