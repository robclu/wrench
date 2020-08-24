//==--- wrench/multithreading/void_lock.hpp ---------------- -*- C++ -*- ---==//
//
//                                  Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  void_lock.hpp
/// \brief This file defines a type which implements the lock interface but does
///        not do any work.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MULTITHREADING_VOID_LOCK_HPP
#define WRENCH_MULTITHREADING_VOID_LOCK_HPP

namespace wrench {

/**
 * Default locking implementation which does no locking, it can be used with
 * types which have a locking policy where for the single-threaded case the
 * required behaviour is not to lock.
 */
struct VoidLock {
  /**
   *  Does nothing when `lock()` is called.
   */
  auto lock() noexcept -> void {}

  /**
   * Does nothing when `unlock()` is called.
   */
  auto unlock() noexcept -> void {}
};

} // namespace wrench

#endif // WRENCH_MULTITHREADING_VOID_LOCK_HPP