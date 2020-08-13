
.. _program_listing_file_include_wrench_perf_profiler.hpp:

Program Listing for File profiler.hpp
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_perf_profiler.hpp>` (``include/wrench/perf/profiler.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   //==--- wrench/perf/profiler.hpp --------------------------- -*- C++ -*- ---==//
   //
   //                                Wrench
   //
   //                      Copyright (c) 2020 Rob Clucas
   //
   //  This file is distributed under the MIT License. See LICENSE for details.
   //
   //==------------------------------------------------------------------------==//
   //
   //
   //==------------------------------------------------------------------------==//
   
   #ifndef WRENCH_PERF_PROFILER_HPP
   #define WRENCH_PERF_PROFILER_HPP
   
   #include <wrench/utils/portability.hpp>
   #include <array>
   #include <chrono>
   #include <cstdint>
   
   #ifdef wrench_linux
     #include <unistd.h>
     #include <sys/ioctl.h>
     #include <linux/perf_event.h>
   #else
   struct perf_event_attr;
   #endif
   
   namespace wrench {
   
   class Profiler {
     enum PerfEvent : uint8_t {
       instructions  = 0, 
       cpu_cycles    = 1, 
       dcache_refs   = 2, 
       dcache_misses = 3, 
       branches      = 4, 
       branch_misses = 5, 
       icache_refs   = 6, 
       icache_misses = 7, 
       event_count        
     };
   
     // clang-format off
     static constexpr uint32_t all_enabled  = 0x00000007;
     static constexpr uint32_t none_enabled = 0x00000000;
     // clang-format on
   
     //==--- [construction] ---------------------------------------------------==//
   
     Profiler() noexcept;
   
     Profiler(uint32_t event_mask) noexcept;
   
     ~Profiler() noexcept;
   
     // clang-format off
     Profiler(const Profiler&) = delete;
     Profiler(Profiler&&)      = delete;
   
     //==--- [operator overloads] ---------------------------------------------==//
   
     auto operator=(const Profiler&) = delete;
     auto operator=(Profiler&&)      = delete;
     // clang-format on
   
     //==---- [interface] -----------------------------------------------------==//
   
     auto reset_events(uint32_t event_mask = all_enabled) noexcept -> uint32_t;
   
     auto enabled_events() const noexcept -> uint32_t {
       return enabled_events_;
     }
   
     constexpr auto perf_event_flag(PerfEvent event) -> uint32_t {
       return 1u << event;
     }
   
     auto is_valid() const noexcept -> bool {
       return first_fd() >= 0;
     }
   
     class Counters {
       friend class Profiler;
       static constexpr size_t num_counters = PerfEvent::event_count;
   
       struct Counter {
         uint64_t value = 0; 
         uint64_t id    = 0; 
       };
   
       uint64_t                          time_enabled_ = 0; 
       uint64_t                          time_running_ = 0; 
       std::array<Counter, num_counters> counters_;         
   
      public:
       using Duration = std::chrono::duration<uint64_t, std::nano>;
   
       auto perf_event_value(PerfEvent event) const noexcept -> uint64_t {
         return counters_[event].value;
       }
   
       auto perf_event_ratio(PerfEvent a, PerfEvent b) const noexcept -> double {
         return double(perf_event_value(a)) / double(perf_event_value(b));
       }
   
       auto wall_time() const noexcept -> Duration {
         return Duration{time_enabled_};
       }
   
       auto running_time() const noexcept -> Duration {
         return Duration{time_running_};
       }
   
       auto instructions_per_cycle() const noexcept -> double {
         return perf_event_ratio(PerfEvent::instructions, PerfEvent::cpu_cycles);
       }
   
       auto cycles_per_instruction() const noexcept -> double {
         return perf_event_ratio(PerfEvent::cpu_cycles, PerfEvent::instructions);
       }
   
       auto l1d_miss_rate() const noexcept -> double {
         return perf_event_ratio(PerfEvent::dcache_misses, PerfEvent::dcache_refs);
       }
   
       auto l1d_hit_rate() const noexcept -> double {
         return 1.0 - l1d_miss_rate();
       }
   
       auto l1i_miss_rate() const noexcept -> double {
         return perf_event_ratio(PerfEvent::icache_misses, PerfEvent::icache_refs);
       }
   
       auto l1i_hit_rate() const noexcept -> double {
         return 1.0 - l1i_miss_rate();
       }
   
       auto branch_miss_rate() const noexcept -> double {
         return perf_event_ratio(PerfEvent::branch_misses, PerfEvent::branches);
       }
   
       auto branch_hit_rate() const noexcept -> double {
         return 1.0 - branch_miss_rate();
       }
     };
   
     wrench_no_discard auto has_branch_rates() const noexcept -> bool {
       return counter_fds_[PerfEvent::branches] >= 0 &&
              counter_fds_[PerfEvent::branch_misses] >= 0;
     }
   
     wrench_no_discard auto has_instruction_cache_rates() const noexcept -> bool {
       return counter_fds_[PerfEvent::icache_refs] >= 0 &&
              counter_fds_[PerfEvent::icache_misses] >= 0;
     }
   
   #ifdef wrench_linux
     auto reset() noexcept -> void {
       ioctl(first_fd(), PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
     }
   
     auto start() noexcept -> void {
       ioctl(first_fd(), PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
     }
   
     auto stop() noexcept -> void {
       ioctl(first_fd(), PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
     }
   
     auto read_counters() noexcept -> Counters;
   #else  // !wrench_linux
     auto reset() noexcept -> void {}
     auto start() noexcept -> void {}
     auto stop() noexcept -> void {}
     auto read_counters() noexcept -> Counters {
       return {};
     }
   #endif // wrench_linux
   
    private:
     using CounterIds = std::array<uint8_t, PerfEvent::event_count>;
     using CounterFds = std::array<int, PerfEvent::event_count>;
   
     CounterIds counter_ids_    = {};           
     CounterFds counter_fds_    = {};           
     uint32_t   enabled_events_ = none_enabled; 
   
     wrench_no_discard auto first_fd() const noexcept -> int {
       return counter_ids_.front();
     }
   
     auto update_events(
       perf_event_attr* event,
       PerfEvent        perf_event,
       uint32_t         event_mask,
       uint32_t         event_type,
       uint64_t         event_config,
       int              group_fd,
       uint8_t&         count) -> void;
   };
   
   } // namespace wrench
   
   #endif // WRENCH_PERF_PROFILER_HPP
