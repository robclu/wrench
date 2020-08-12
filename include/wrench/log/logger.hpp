//==--- wrench/log/logger.hpp ------------------------------ -*- C++ -*- ---==//
//
//                                Wrench
//
//                      Copyright (c) 2020 Ripple
//
//  This file is distributed under the MIT License. See LICENSE for details.
//
//==------------------------------------------------------------------------==//
//
/// \file  logger.hpp
/// \brief This file defines a logger class.
//
//==------------------------------------------------------------------------==//

#ifndef WRENCH_LOG_LOGGER_HPP
#define WRENCH_LOG_LOGGER_HPP

#include <wrench/multithreading/spinlock.hpp>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <array>
#include <cstddef>
#include <fstream>

namespace wrench {

/// Default buffer size for the logger.
static constexpr size_t logger_default_buffer_size = 1024;

/// Defines the default file to log to.
static constexpr const char* const logfile_path = ".wrench_log.txt";

/// Defines levels for logging.
enum class LogLevel : uint8_t {
  debug   = 0, //!< Debugging log level.
  info    = 1, //!< Info log level.
  warning = 2, //!< Warning log level.
  error   = 3, //!< Error log.
  none    = 4  //!< No logging.
};

/// Basic logger class, which logs messages when the level of the message is
/// more tha the MinLevel. The messages are buffered in a buffer of BufferSize,
/// and if a message wont fit in the buffer, the buffer is flushed.
///
/// This logger is not designed to be high-performance, since in release the
/// only messages that will be logged in the fast path are error messages, and
/// info messages will be in the slow path, so safety and flexibility are more
/// important.
///
/// The logger uses an instance of the locking policy to lock when logging.
///
/// \tparam MinLevel   The minimum level for message logging.
/// \tparam BufferSize The size of the message buffer.
/// \tparam LockPolicy The locking polcity for the lock.
template <
  LogLevel MinLevel,
  size_t   BufferSize = logger_default_buffer_size,
  typename LockPolicy = Spinlock>
class Logger;

//==--- [alias for the logger] ---------------------------------------------==//

/// Defines the type of the logger.
using Log =
#if WRENCH_LOG_LEVEL_NONE
  Logger<LogLevel::none>;
#elif WRENCH_LOG_LEVEL_ERROR
  Logger<LogLevel::error>;
#elif WRENCH_LOG_LEVEL_WARN
  Logger<LogLevel::warn>;
#elif WRENCH_LOG_LEVEL_INFO
  Logger<LogLevel::info>;
#else
  Logger<LogLevel::debug>;
#endif

//==--- [log functions] ----------------------------------------------------==//

/// Logs the \p format format specifier, filling in the format string with the
/// \p fmt_args, if the LogLevel of the logger is less than or equal to
/// LogLevel::error. The \p format and \p fmt_args should be specified as they
/// would to `std::format()`.
/// \param  format   The format specifier for the message.
/// \param  fmt_args The arguments for the format specifier.
/// \tparam Fmt      The type of the format specifier.
/// \tparam FmtArgs  The types of the format arguments.
template <typename Fmt, typename... FmtArgs>
auto log_error(Fmt&& format, FmtArgs&&... fmt_args) -> void;

/// Logs the \p format format specifier, filling in the format string with the
/// \p fmt_args, if the LogLevel of the logger is less than or equal to
/// LogLevel::warn. The \p format and \p fmt_args should be specified as they
/// would to `std::format()`.
/// \param  format   The format specifier for the message.
/// \param  fmt_args The arguments for the format specifier.
/// \tparam Fmt      The type of the format specifier.
/// \tparam FmtArgs  The types of the format arguments.
template <typename Fmt, typename... FmtArgs>
auto log_warn(Fmt&& format, FmtArgs&&... fmt_args) -> void;

/// Logs the \p format format specifier, filling in the format string with the
/// \p fmt_args, if the LogLevel of the logger is less than or equal to
/// LogLevel::info. The \p format and \p fmt_args should be specified as they
/// would to `std::format()`.
/// \param  format   The format specifier for the message.
/// \param  fmt_args The arguments for the format specifier.
/// \tparam Fmt      The type of the format specifier.
/// \tparam FmtArgs  The types of the format arguments.
template <typename Fmt, typename... FmtArgs>
auto log_info(Fmt&& format, FmtArgs&&... fmt_args) -> void;

/// Logs the \p format format specifier, filling in the format string with the
/// \p fmt_args, if the LogLevel of the logger is less than or equal to
/// LogLevel::debug. The \p format and \p fmt_args should be specified as they
/// would to `std::format()`.
/// \param  format   The format specifier for the message.
/// \param  fmt_args The arguments for the format specifier.
/// \tparam Fmt      The type of the format specifier.
/// \tparam FmtArgs  The types of the format arguments.
template <typename Fmt, typename... FmtArgs>
auto log_debug(Fmt&& format, FmtArgs&&... fmt_args) -> void;

//==--- [implementation] ---------------------------------------------------==//

// Implementation of the logger class.
// \tparam MinLevel   The minimum level for message logging.
// \tparam BufferSize The size of the message buffer.
// \tparam LockPolicy The locking policy for the logger.
template <LogLevel MinLevel, size_t BufferSize, typename LockPolicy>
class Logger {
  /// Defines the end of the buffer.
  static constexpr size_t buffer_end = BufferSize;
  /// Defines the type of the buffer.
  using Buffer = std::array<char, buffer_end>;
  /// Defines the type of the guard for the logger.
  using Guard = std::lock_guard<LockPolicy>;

  /// Defines a valid type if Level < MinLevel.
  /// \tparam Level The level to base the enable on.
  template <LogLevel Level>
  using void_log_enable_t = std::enable_if_t<(Level < MinLevel), int>;

  /// Defines a valid type if Level >= MinLevel.
  /// \tparam Level The level to base the enable on.
  template <LogLevel Level>
  using valid_log_enable_t = std::enable_if_t<(Level >= MinLevel), int>;

 public:
  //==--- [constants] ------------------------------------------------------==//

  /// Defines the level for the logger. All messages logged below the level
  /// will become no-ops.
  static constexpr LogLevel level = MinLevel;

  //==--- [construction] ---------------------------------------------------==//

  /// Flushes pending logs in the stream and closes the logging file.
  ~Logger() {
    flush();
    stream_.close();
  }

  //==--- [defaulted] ------------------------------------------------------==//

  // clang-format off
  /// Defaulted move construction.
  Logger(Logger&&) noexcept                    = default;
  /// Delete move assignment, since we can only have a single logger.
  auto operator=(Logger&&) noexcept -> Logger& = default;

  //==--- [deleted] --------------------------------------------------------==//

  /// Deleted , create through create().
  Logger()                      = delete;
  /// Delete copy construction, since we can only have one logger.
  Logger(const Logger&)         = delete;
  /// Delete copy assignment, since we can only have a single logger.
  auto operator=(const Logger&) = delete;
  // clang-format on

  //==--- [interface] ------------------------------------------------------==//

  /// Accesses the global logger.
  static auto logger() noexcept -> Logger& {
    static Logger l(logfile_path);
    return l;
  }

  /// Flushes the pending messages in the logger to the logging file.
  /// Note that this claims the mutex to perform the write, so it should only be
  /// called to flush the logger if a termination happens.
  auto flush() -> void {
    stream_.write(&buffer_[0], end_);
    end_ = 0;
  }

  /// Never logs the \p message, which should already be formatted. This
  /// overload is enabled if the level L is _more than_ the level of the
  /// logger, and will be compiled away.
  ///
  /// \param  message The message to log.
  /// \tparam L       The log level for the message.
  template <LogLevel L, void_log_enable_t<L> = 0>
  auto log(const std::string& message) noexcept -> void {}

  /// Logs the \p message, which should already be formatted. This overload is
  /// only enabled if the level L is _less than or equal_ to the level of the
  /// logger.
  ///
  /// \param  message The message to log.
  /// \tparam L       The log level for the message.
  template <LogLevel L, valid_log_enable_t<L> = 0>
  void log(const std::string& message) {
    const auto rem = buffer_end - end_;

    // NOTE: Here we take the lock until the write is done. If we just take the
    // lock to increment the end of the buffer, then when we do the write into
    // the portion of the buffer there could be false sharing. Since this isn't
    // critical to perforamance, we just lock the whole operation.

    // message fits in remaining buffer:
    if (message.length() < rem) {
      Guard g(lock_);
      end_ += sprintf(&buffer_[end_], "%s", message.c_str());
      return;
    }

    // message fits in whole buffer:
    if (message.length() < buffer_end) {
      Guard g(lock_);
      flush();
      end_ += sprintf(&buffer[end_], "%s", message.c_str());
      return;
    }

    // message doesn't fit, write what we can and flush.
    Guard g(lock_);
    snprintf(&buffer[end_], rem, "%s", message.c_str());
    flush();
  }

 private:
  buffer_t      buffer_ = {}; //!< The buffer for the logger.
  size_t        end_    = 0; //!< The end of the last filled char in the buffer.
  std::ofstream stream_;     //!< The stream to log to.
  std::mutex    lock_;       //!< Mutex for logging messages.

  /// Constructor which initializes the logger with a \p log_file to write to.
  /// \param log_file The file to write the logs to.
  Logger(const std::string& log_file) : stream_(log_file, std::ios::trunc) {}
};

//==--- [log macros] -------------------------------------------------------==//

// Implementation of log_error.
template <typename Fmt, typename... FmtArgs>
auto log_error(Fmt&& format, FmtArgs&&... fmt_args) -> void {
  if constexpr (Log::level <= LogLevel::error) {
    Log::logger().log<LogLevel::error>(fmt::format(
      "[Error] | {0:%H:%M:%S} | {1}\n",
      std::chrono::high_resolution_clock::now().time_since_epoch(),
      fmt::format(format, fmt_args...)));
  }
}

// Implementation of log_warn.
template <typename Fmt, typename... FmtArgs>
auto log_warn(Fmt&& format, FmtArgs&&... fmt_args) -> void {
  if constexpr (Log::level <= LogLevel::warning) {
    Log::logger().log<LogLevel::warning>(fmt::format(
      "[Warn]  | {0:%H:%M:%S} | {1}\n",
      std::chrono::high_resolution_clock::now().time_since_epoch(),
      fmt::format(format, fmt_args...)));
  }
}

// Implementation of log_info.
template <typename Fmt, typename... FmtArgs>
auto log_info(Fmt&& format, FmtArgs&&... fmt_args) -> void {
  if constexpr (Log::level <= LogLevel::info) {
    Log::logger().log<LogLevel::info>(fmt::format(
      "[Info]  | {0:%H:%M:%S} | {1}\n",
      std::chrono::high_resolution_clock::now().time_since_epoch(),
      fmt::format(format, fmt_args...)));
  }
}

// Implementation of log_debug.
template <typename Fmt, typename... FmtArgs>
auto log_debug(Fmt&& format, FmtArgs&&... fmt_args) -> void {
  if constexpr (Log::level <= LogLevel::debug) {
    Log::logger().log<LogLevel::debug>(fmt::format(
      "[Debug] | {0:%H:%M:%S} | {1}\n",
      std::chrono::high_resolution_clock::now().time_since_epoch(),
      fmt::format(format, fmt_args...)));
  }
}

} // namespace wrench

#endif // WRENCH_LOG_LOGGER_HPP