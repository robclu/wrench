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
/// \file  profiler.hpp
/// \brief This file defines a profiler utility which can be used to get
///        performance statistics for an application.
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

/// Class which can be used to profile applications. Profiling functionality
/// currently only works on osx and linux.
class Profiler {
  /// Enumeration for events.
  enum PerfEvent : uint8_t {
    instructions  = 0, //!< Instruction count.
    cpu_cycles    = 1, //!< Cpu cycles.
    dcache_refs   = 2, //!< Data cache references.
    dcache_misses = 3, //!< Data cache misses.
    branches      = 4, //!< Number of branches.
    branch_misses = 5, //!< Number of branch misses.
    icache_refs   = 6, //!< Instruction cache references.
    icache_misses = 7, //!< Instruciton cache misses.
    event_count        //!< Number of events.
  };

  // clang-format off
  /// Mask for all events enabled.
  static constexpr uint32_t all_enabled  = 0x00000007;
  /// Mask for no events enables.
  static constexpr uint32_t none_enabled = 0x00000000;
  // clang-format on

  //==--- [construction] ---------------------------------------------------==//

  /// Constructor to initialize the profiler.
  Profiler() noexcept;

  /// Convstructor to create the profiler with the \p event_mask.
  /// \param event_mask The mask to create the profiler with.
  Profiler(uint32_t event_mask) noexcept;

  /// Destructor to clean up the profiler resources.
  ~Profiler() noexcept;

  // clang-format off
  /// Copy constructor -- deleted.
  Profiler(const Profiler&) = delete;
  /// Move constructor -- deleted.
  Profiler(Profiler&&)      = delete;

  //==--- [operator overloads] ---------------------------------------------==//

  /// Copy assignment operator -- deleted.
  auto operator=(const Profiler&) = delete;
  /// Move assignment operator -- deleted.
  auto operator=(Profiler&&)      = delete;
  // clang-format on

  //==---- [interface] -----------------------------------------------------==//

  /// Resets all events for the profiler.
  /// \param event_mask The mask for the events to reset.
  auto reset_events(uint32_t event_mask = all_enabled) noexcept -> uint32_t;

  /// Returns a mask of all enabled events;
  auto enabled_events() const noexcept -> uint32_t {
    return enabled_events_;
  }

  /// Returns the flag for the \p event.
  constexpr auto perf_event_flag(PerfEvent event) -> uint32_t {
    return 1u << event;
  }

  /// Returns true if the profiler is valid.
  auto is_valid() const noexcept -> bool {
    return first_fd() >= 0;
  }

  /// Classs to represent the counters.
  class Counters {
    friend class Profiler;
    /// Defines the number of counters.
    static constexpr size_t num_counters = PerfEvent::event_count;

    /// Struct to represent a counter.
    struct Counter {
      uint64_t value = 0; //!< Value of the counter.
      uint64_t id    = 0; //!< Id for the counter.
    };

    uint64_t                          time_enabled_ = 0; //!< Wall clock time.
    uint64_t                          time_running_ = 0; //!< Running time.
    std::array<Counter, num_counters> counters_;         //!< Counters.

   public:
    /// Defines the type of the time duration.
    using Duration = std::chrono::duration<uint64_t, std::nano>;

    /// Returns the value of the perf event \p event.
    /// \param event The event to get the value of.
    auto perf_event_value(PerfEvent event) const noexcept -> uint64_t {
      return counters_[event].value;
    }

    /// Returns the ratio of the event \p a to the event \p b, returning
    /// `a / b`.
    /// \param a  The first event for the ratio.
    /// \param b The second event for the ratio.
    auto perf_event_ratio(PerfEvent a, PerfEvent b) const noexcept -> double {
      return double(perf_event_value(a)) / double(perf_event_value(b));
    }

    /// Returns the wall clock time.
    auto wall_time() const noexcept -> Duration {
      return Duration{time_enabled_};
    }

    /// Returns the running time,
    auto running_time() const noexcept -> Duration {
      return Duration{time_running_};
    }

    /// Returns the number of instructions per cycle.
    auto instructions_per_cycle() const noexcept -> double {
      return perf_event_ratio(PerfEvent::instructions, PerfEvent::cpu_cycles);
    }

    /// Returns the number of cycles per insturction.
    auto cycles_per_instruction() const noexcept -> double {
      return perf_event_ratio(PerfEvent::cpu_cycles, PerfEvent::instructions);
    }

    /// Returns the L1D cache miss rate.
    auto l1d_miss_rate() const noexcept -> double {
      return perf_event_ratio(PerfEvent::dcache_misses, PerfEvent::dcache_refs);
    }

    /// Returns the L1D hit rate.
    auto l1d_hit_rate() const noexcept -> double {
      return 1.0 - l1d_miss_rate();
    }

    /// Returns the L1Icache miss rate.
    auto l1i_miss_rate() const noexcept -> double {
      return perf_event_ratio(PerfEvent::icache_misses, PerfEvent::icache_refs);
    }

    /// Returns the L1I hit rate.
    auto l1i_hit_rate() const noexcept -> double {
      return 1.0 - l1i_miss_rate();
    }

    /// Returns the branch miss rate.
    auto branch_miss_rate() const noexcept -> double {
      return perf_event_ratio(PerfEvent::branch_misses, PerfEvent::branches);
    }

    /// returns the branch hit rate.
    auto branch_hit_rate() const noexcept -> double {
      return 1.0 - branch_miss_rate();
    }
  };

  /// Returns true if branch rates are available.
  wrench_no_discard auto has_branch_rates() const noexcept -> bool {
    return counter_fds_[PerfEvent::branches] >= 0 &&
           counter_fds_[PerfEvent::branch_misses] >= 0;
  }

  /// Returns true if instruction cache rates are available.
  wrench_no_discard auto has_instruction_cache_rates() const noexcept -> bool {
    return counter_fds_[PerfEvent::icache_refs] >= 0 &&
           counter_fds_[PerfEvent::icache_misses] >= 0;
  }

#ifdef wrench_linux
  /// Resets the profiler.
  auto reset() noexcept -> void {
    ioctl(first_fd(), PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  }

  /// Starts the profiler.
  auto start() noexcept -> void {
    ioctl(first_fd(), PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
  }

  auto stop() noexcept -> void {
    ioctl(first_fd(), PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
  }

  /// Reads the profiler counters.
  auto read_counters() noexcept -> Counters;
#else  // !wrench_linux
  /// Resets the profiler.
  auto reset() noexcept -> void {}
  /// Starts the profiler.
  auto start() noexcept -> void {}
  /// Stops the profiler.
  auto stop() noexcept -> void {}
  /// Reads the profiler counters.
  auto read_counters() noexcept -> Counters {
    return {};
  }
#endif // wrench_linux

 private:
  /// Type used for counter ids.
  using CounterIds = std::array<uint8_t, PerfEvent::event_count>;
  /// Type used for counter file descriptors.
  using CounterFds = std::array<int, PerfEvent::event_count>;

  CounterIds counter_ids_    = {};           //!< Ids for each counter.
  CounterFds counter_fds_    = {};           //!< Counter file descriptors.
  uint32_t   enabled_events_ = none_enabled; //!< Mask of enabled events.

  /// Returns the first counter file descriptor.
  wrench_no_discard auto first_fd() const noexcept -> int {
    return counter_ids_.front();
  }

  /// Updates the enabled events.
  /// \param event_attr   The attribute describing the event.
  /// \param perf_event   The type of the event to update.
  /// \param event_mask   The mask of events to update.
  /// \param event_type   The type of the perf_event_attr.
  /// \param event_config The config of the perf_event_attr.
  /// \param group_fd     The descriptor for the group.
  /// \param count        The number of events currently updated.
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