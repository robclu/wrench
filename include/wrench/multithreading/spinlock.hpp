//==--- wrench/multithreading/spinlock.hpp ------------------ -*- C++ -*----==//
//
//                                  Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  spinlock.hpp
/// \brief This file defines a spinlock implementation.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_MULTITHREADING_SPINLOCK_HPP
#define WRENCH_MULTITHREADING_SPINLOCK_HPP

#include <chrono>
#include <thread>

namespace wrench {

// A small spinlock implementation.
struct Spinlock {
 private:
  // clang-format off
  /// Defines the state of an unlocked lock.
  static constexpr uint8_t unlocked = 0;
  /// Defines the state of a locked lock.
  static constexpr uint8_t locked   = 1;
  // clang-format

  /// A struct which can be used to spin for a certain number of iterations and
  /// then sleep if the iteration count has been reached.
  struct Sleeper {
    /// Max number of spins before sleeping.
    static constexpr uint32_t max_spins = 4000;

    /// Puts the thread to sleep for a duration which is __usually__ less than
    /// the minimum sleep time for the kernel, so the kernel will __usually__
    /// schedule this thread to sleep for the minimum duration, which is usually
    /// somewhere between 1 and 10 ms.
    static auto sleep() noexcept -> void {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(500us);
    }

    /// Causes the CPU to wait if the spin count is less than the maximum number
    /// of spins, otherwise sleeps for the kernel min sleep duration.
    auto wait() noexcept -> void {
      if (spincount_ < max_spins) {
        spincount_++;
        // Essentially _mm_pause() and a memory barrier in one instruction.
        // Just to make sure that there is no memory reordering which might
        // be the case if the compiler decided to move things around.
        // The pause prevents speculative loads from causing pipeline clears
        // due to memory ordering mis-speculation.
        asm volatile("pause" ::: "memory");
        return;
      }
      sleep();
    }

   private:
    uint32_t spincount_ = 0; //!< Number of spins.
  };

 public:
  /// Tries to lock the spinlock, returning true if the lock succeded.
  auto try_lock() noexcept -> bool {
    return __sync_bool_compare_and_swap(&lock_, unlocked, locked);
  }

  /// Locks the spinlock. This will block until the lock is acquired.
  auto lock() noexcept -> void {
    Sleeper sleeper;
    while (!__sync_bool_compare_and_swap(&lock_, unlocked, locked)) {
      do {
        // Wait until a cas might succeed:
        sleeper.wait();
      } while (lock_);
    }
  }

  /// Unlocks the spinlock.
  auto unlock() noexcept -> void {
    // Memory barries so that we can write the lock to unlocked.
    asm volatile("" ::: "memory");
    lock_ = unlocked;
  }

 private:
  uint8_t lock_ = unlocked; //!< Lock for the spinlock.
};

} // namespace wrench

#endif // WRENCH_MULTITHREADING_SPINLOCK_HPP