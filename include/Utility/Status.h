#pragma once

namespace robocon {

/**
 * @enum Status
 * @brief Standardized return type for all firmware functions
 *
 * Every function in the firmware returns Status to indicate success or failure.
 * This enables predictable error handling and status propagation up the call stack.
 *
 * NOTE: FATAL from 02_PROJECT_SPEC.md was replaced with TIMEOUT per frozen architecture.
 * Timeout detection is critical for watchdog functionality in this embedded system.
 */
enum class Status {
  OK = 0,           ///< Operation succeeded
  WARNING = 1,      ///< Operation succeeded with concern (used rarely for diagnostics)
  ERROR = 2,        ///< Operation failed — caller must handle and propagate upward
  TIMEOUT = 3       ///< Operation exceeded time limit (critical for watchdog detection)
};

}  // namespace robocon
