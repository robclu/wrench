
.. _program_listing_file_include_wrench_log_logger.hpp:

Program Listing for File logger.hpp
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_wrench_log_logger.hpp>` (``include/wrench/log/logger.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   static constexpr size_t logger_default_buffer_size = 1024;
   
   static constexpr const char* const logfile_path = ".wrench_log.txt";
   
   enum class LogLevel : uint8_t {
     debug   = 0, 
     info    = 1, 
     warning = 2, 
     error   = 3, 
     none    = 4  
   };
   
   template <
     LogLevel MinLevel,
     size_t   BufferSize = logger_default_buffer_size,
     typename LockPolicy = Spinlock>
   class Logger;
   
   //==--- [alias for the logger] ---------------------------------------------==//
   
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
   
   template <typename Fmt, typename... FmtArgs>
   auto log_error(Fmt&& format, FmtArgs&&... fmt_args) -> void;
   
   template <typename Fmt, typename... FmtArgs>
   auto log_warn(Fmt&& format, FmtArgs&&... fmt_args) -> void;
   
   template <typename Fmt, typename... FmtArgs>
   auto log_info(Fmt&& format, FmtArgs&&... fmt_args) -> void;
   
   template <typename Fmt, typename... FmtArgs>
   auto log_debug(Fmt&& format, FmtArgs&&... fmt_args) -> void;
   
   //==--- [implementation] ---------------------------------------------------==//
   
   // Implementation of the logger class.
   // \tparam MinLevel   The minimum level for message logging.
   // \tparam BufferSize The size of the message buffer.
   // \tparam LockPolicy The locking policy for the logger.
   template <LogLevel MinLevel, size_t BufferSize, typename LockPolicy>
   class Logger {
     static constexpr size_t buffer_end = BufferSize;
     using Buffer = std::array<char, buffer_end>;
     using Guard = std::lock_guard<LockPolicy>;
   
     template <LogLevel Level>
     using VoidLogEnable = std::enable_if_t<(Level < MinLevel), int>;
   
     template <LogLevel Level>
     using ValidLogEnable = std::enable_if_t<(Level >= MinLevel), int>;
   
    public:
     //==--- [constants] ------------------------------------------------------==//
   
     static constexpr LogLevel level = MinLevel;
   
     //==--- [construction] ---------------------------------------------------==//
   
     ~Logger() {
       flush();
       stream_.close();
     }
   
     //==--- [defaulted] ------------------------------------------------------==//
   
     // clang-format off
     Logger(Logger&&) noexcept                    = default;
     auto operator=(Logger&&) noexcept -> Logger& = default;
   
     //==--- [deleted] --------------------------------------------------------==//
   
     Logger()                      = delete;
     Logger(const Logger&)         = delete;
     auto operator=(const Logger&) = delete;
     // clang-format on
   
     //==--- [interface] ------------------------------------------------------==//
   
     static auto logger() noexcept -> Logger& {
       static Logger l(logfile_path);
       return l;
     }
   
     template <LogLevel L>
     static constexpr auto would_log() noexcept -> bool {
       return L >= MinLevel;
     }
   
     auto flush() -> void {
       stream_.write(&buffer_[0], end_);
       end_ = 0;
     }
   
     template <LogLevel L, VoidLogEnable<L> = 0>
     auto log(const std::string& message) noexcept -> void {}
   
     template <LogLevel L, ValidLogEnable<L> = 0>
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
     buffer_t      buffer_ = {}; 
     size_t        end_    = 0; 
     std::ofstream stream_;     
     std::mutex    lock_;       
   
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
