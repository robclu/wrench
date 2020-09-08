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

namespace wrench {

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

namespace detail {

/**
 * Implementation type for is_base_of functionality for templated bases.
 * \tparam Base    The templated base type.
 * \tparam Derived The derived type.
 */
template <template <typename...> typename Base, typename Derived>
struct IsBaseOfTemplateImpl {
  /**
   * Overload to select base types.
   * \tparam Ts The base template types.
   * \return std::true_type.
   */
  template <typename... Ts>
  static constexpr auto test(const Base<Ts...>*) -> std::true_type;

  /**
   * Overload for any other type.
   * \return std::false_type.
   */
  static constexpr auto test(...) -> std::false_type;

  /**
   * The return type of the evaluation.
   */
  using type = decltype(test(std::declval<Derived*>()));
};

/**
 *
 */

/**
 * Implementation type for is_base_of functionality for non-type templated
 * bases.
 * \tparam Base    The templated base type.
 * \tparam Derived The derived type.
 */
template <template <auto...> typename Base, typename Derived>
struct IsBaseOfAutoTemplateImpl {
  /**
   * Overload to select base types.
   * \tparam Ts The base template types.
   * \return std::true_type.
   */
  template <auto... Ts>
  static constexpr auto test(const Base<Ts...>*) -> std::true_type;

  /**
   * Overload for any other type.
   * \return std::false_type.
   */
  static constexpr auto test(...) -> std::false_type;

  /**
   * The return type of the evaluation.
   */
  using type = decltype(test(std::declval<Derived*>()));
};

} // namespace detail

/**
 * Defines a type as std::true_type if Base is a base class of derived,
 * otherwise defined std::false_type.
 * \tparam Base    The type of the base class.
 * \tparam Derived The type of the derived class.
 */
template <template <typename...> typename Base, typename Derived>
using is_base_of_template_t =
  typename detail::IsBaseOfTemplateImpl<Base, Derived>::type;

/**
 * Defines a type as std::true_type if Base is a base class of derived,
 * otherwise defined std::false_type.
 * \tparam Base    The type of the base class.
 * \tparam Derived The type of the derived class.
 */
template <template <auto...> typename Base, typename Derived>
using is_base_of_nontype_template_t =
  typename detail::IsBaseOfAutoTemplateImpl<Base, Derived>::type;

/**
 * Returns true if Base is a base class of derived otherwise returns false.
 * \tparam Base    The type of the base class.
 * \tparam Derived The type of the derived class.
 */
template <template <typename...> typename Base, typename Derived>
static constexpr bool is_base_of_template_v =
  is_base_of_template_t<Base, Derived>::value;

/**
 * Returns true if Base is a base class of derived otherwise returns false.
 * \tparam Base    The type of the base class.
 * \tparam Derived The type of the derived class.
 */
template <template <auto...> typename Base, typename Derived>
static constexpr bool is_base_of_nontype_template_v =
  is_base_of_nontype_template_t<Base, Derived>::value;

} // namespace wrench

#endif // WRENCH_UTILS_TYPE_TRAITS_HPP