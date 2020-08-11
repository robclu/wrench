
.. _program_listing_file_include_wrench_multithreading_spinlock.hpp:

Program Listing for File spinlock.hpp
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_multithreading_spinlock.hpp>` (``include/wrench/multithreading/spinlock.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
     static constexpr uint8_t unlocked = 0;
     static constexpr uint8_t locked   = 1;
     // clang-format
   
     struct Sleeper {
       static constexpr uint32_t max_spins = 2000;
   
       static inline auto sleep() noexcept -> void {
         using namespace std::chrono_literals;
         std::this_thread::sleep_for(200us);
       }
   
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
       uint32_t spincount_ = 0; 
     };
   
    public:
     auto try_lock() noexcept -> bool {
       return __sync_bool_compare_and_swap(&lock_, unlocked, locked);
     }
   
     auto lock() noexcept -> void {
       Sleeper sleeper;
       while (!__sync_bool_compare_and_swap(&lock_, unlocked, locked)) {
         do {
           // Wait until a cas might succeed:
           sleeper.wait();
         } while (lock_);
       }
     }
   
     auto unlock() noexcept -> void {
       // Memory barries so that we can write the lock to unlocked.
       asm volatile("" ::: "memory");
       lock_ = unlocked;
     }
   
    private:
     uint8_t lock_ = unlocked; 
   };
   
   } // namespace wrench
   
   #endif // WRENCH_MULTITHREADING_SPINLOCK_HPP
