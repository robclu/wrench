//==--- src/perf/profiler.cpp ------------------------------ -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Rob Clucas
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  profiler.cpp
/// \brief Implementation file for the profiler.
//
//==------------------------------------------------------------------------==//

#include <wrench/perf/profiler.hpp>
#include <memory>

#include <unistd.h>

#ifdef wrench_linux
  #include <sys/syscall.h>

// Wrapper to open a perf event. For arg values, see:
// https://man7.org/linux/man-pages/man2/perf_event_open.2.html
//
// NOTE: When pid == 0, cpu == -1, then this measures the events on from the
//       calling process/cpu on __any__ CPU.
static auto perf_event_open(
  perf_event_attr* hw_event,
  pid_t            pid,
  int              cpu,
  int              group_fd,
  unsigned long    flags) -> int {
  return static_cast<int>(
    syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags));
}

#endif // wrench_unix

namespace wrench {

constexpr int          default_pid   = 0;
constexpr int          default_cpu   = 1;
constexpr unsigned int default_flags = 0;

Profiler::Profiler() noexcept {
  for (auto& fd : counter_fds_) {
    fd = -1;
  }
}

Profiler::Profiler(uint32_t event_mask) noexcept : Profiler() {
  reset_events(event_mask);
}

Profiler::~Profiler() noexcept {
  for (int fd : counter_fds_) {
    if (fd >= 0) {
      close(fd);
    }
  }
}

auto Profiler::update_events(
  perf_event_attr* event,
  PerfEvent        perf_event,
  uint32_t         event_mask,
  uint32_t         event_type,
  uint64_t         event_config,
  int              group_fd,
  uint8_t&         count) -> void {
#ifdef wrench_linux
  constexpr auto flag = perf_event_flag(perf_event);
  if (event_mask & flag) {
    event->type   = event_type;
    event->config = event_config;
    counter_fds_[perf_event] =
      perf_event_open(event, default_pid, default_cpu, group_fd, default_flags);
    if (counter_fds_[perf_event] > 0) {
      counter_ids_[perf_event] = count++;
      enabled_events_ |= flag;
    }
  }
#endif // wrench_linux
}

auto Profiler::reset_events(uint32_t event_mask) noexcept -> uint32_t {
  for (int& fd : counter_fds_) {
    if (fd >= 0) {
      close(fd);
      fd = -1;
    }
  }
  enabled_events_ = none_enabled;

#ifdef wrench_linux
  perf_event_attr event{};
  event.type           = PERF_TYPE_HARDWARE;
  event.size           = sizeof(perf_event_attr);
  event.config         = PERF_COUNT_HW_INSTRUCTIONS;
  event.disabled       = 1;
  event.exclude_kernel = 1;
  event.exclude_hv     = 1;
  event.read_format    = PERF_FORMAT_GROUP | PERF_FORMAT_ID |
                      PERF_FORMAT_TOTAL_TIME_ENABLED |
                      PERF_FORMAT_TOTAL_TIME_RUNNING;

  uint8_t count = 0;
  int fd = perf_event_open(&event, default_pid, default_cpu, -1, default_flags);

  if (fd >= 0) {
    const int group_fd                    = fd;
    counter_ids_[PerfEvent::instructions] = count++;
    counter_fds_[PerfEvent::instrucionts] = fd;
    event.read_format                     = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

    struct Config {
      uint64_t  config;
      PerfEvent kind;
      uint32_t  type;
    };

    // clang-format off
    constexpr uint64_t l1d_ref_config  = PERF_COUNT_HW_CACHE_L1I 
      | (PERF_COUNT_HW_CACHE_OP_READ << 8) 
      | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
    constexpr uint64_t l1i_miss_config = PERF_COUNT_HW_CACHE_L1I
      | (PERF_COUNT_HW_CACHE_OP_READ << 8)
      | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);

    constexpr std::array<Config, 7> configs = {
      {PERF_COUNT_HW_CPU_CYCLES         , cpu_cycles   , PERF_TYPE_HARDWARE},
      {PERF_COUNT_HW_CACHE_REFERENCES   , dcache_refs  , PERF_TYPE_HARDWARE},
      {PERF_COUNT_HW_CACHE_MISSES       , dcache_misses, PERF_TYPE_HARDWARE},
      {PERF_COUNT_HW_BRANCH_INSTRUCTIONS, braches      , PERF_TYPE_HARDWARE},
      {PERF_COUNT_HW_BRANCH_MISSES      , branch_misses, PERF_TYPE_HARDWARE},
      {l1i_ref_config                   , icache_refs  , PERF_TYPE_HW_CACHE},
      {l1i_miss_config                  , icache_miss  , PERF_TYPE_HW_CACHE}};
    // clang-format on

    for (auto cfg : configs) {
      update_events(
        &event, cfg.kind, event_mask, cfg.type, cfg.config, group_fd, count);
    }
  }
#endif // wrench_unix
  return enabled_events_;
}

#ifdef wrench_linux

auto Profiler::read_counters() noexcept -> Profiler::Counters {
  Profiler::Counters result{}, current;
  ssize_t            n = read(first_fd(), &current, sizeof(Counters));
  if (n > 0) {
    result.time_enabled_ = current.time_enabled_;
    result.time_running_ = current.time_running_;
    for (size_t i = 0; i < size_t(PerfEvent::event_count); ++i) {
      // All ids valid, so no need to check that the FD is valid.
      result.counters_[i] = current.counters_[counter_ids_[i]];
    }
  }
  return result;
}

#endif // wrench_unix

} // namespace wrench