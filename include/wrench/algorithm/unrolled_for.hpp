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
/// \file  unrolled_for.hpp
/// \brief This file implements functionality for an unrolled for loop with a
///        compile-time size.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_ALGORITHM_UNROLLED_FOR_HPP
#define WRENCH_ALGORITHM_UNROLLED_FOR_HPP

#include "detail/unrolled_for_impl_.hpp"

namespace wrench {

/// Applies the functor `Amount` times and passes the index of the unrolling
/// as the first argument to the functor. The unrolling is performed at compile
/// time, so the number of iterations should not be too large. The index
/// parameter has a compile-time value, and can therefore be used in constexpr
/// contexts, and can be converted to `size_t` as a constexpr expression.
/// For example:
///
/// ~~~cpp
/// auto tuple = std::make_tuple("string", 4, AType*());
/// unrolled_for<2>([&tuple] (auto i) {
///   do_something(get<i>(tuple), get<i + 1>(tuple));
/// });
/// ~~~
///
/// Which effectively will become:
///
/// ~~~cpp
/// do_something(get<0>(tuple), get<1>(tuple));
/// do_something(get<1>(tuple), get<2>(tuple));
/// ~~~
///
/// \note This will cause code bloat if used often. Use this only in cases where
///       compile time unrolling is required with a compile time index variable.
///
/// \note The functor must not return any value.
///
/// \param  functor   The functor to unroll.
/// \param  args      The arguments to the functor.
/// \tparam Amount    The amount of unrolling to do.
/// \tparam Functor   The type of the functor.
/// \tparam Args      The type of the functor arguments.
template <size_t Amount, typename Functor, typename... Args>
constexpr inline auto
unrolled_for(Functor&& functor, Args&&... args) noexcept -> void {
  detail::Unroll<Amount> unrolled(
    std::forward<Functor>(functor), std::forward<Args>(args)...);
}

} // namespace wrench

#endif // WRENCH_ALGORITHM_UNROLLED_FOR_HPP