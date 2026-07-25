#pragma once

#include <cstdint>
#include "Utility/Status.h"

namespace robocon {
namespace HAL {

/**
 * @class GPIO
 * @brief Hardware Abstraction Layer for ESP32 GPIO operations
 *
 * Single Responsibility: Encapsulate ESP32 GPIO register access and pin configuration.
 * Provides a clean interface for input/output operations without exposing hardware details.
 *
 * Thread Safety: Not thread-safe. Must be called from single context or with external mutex.
 * Blocking: All methods are non-blocking.
 *
 * Production Quality:
 * - No dynamic allocation
 * - No blocking operations
 * - Static allocation only
 * - Minimal overhead (inline where appropriate)
 */
class GPIO {
 public:
  /**
   * @enum Mode
   * @brief GPIO pin configuration mode
   */
  enum class Mode : uint8_t {
    INPUT = 0,              ///< Input mode (high impedance)
    OUTPUT = 1,             ///< Output mode (push-pull)
    INPUT_PULLUP = 2,       ///< Input with internal pull-up resistor
    INPUT_PULLDOWN = 3      ///< Input with internal pull-down resistor
  };

  /**
   * @enum Level
   * @brief GPIO pin logic level
   */
  enum class Level : uint8_t {
    LOW = 0,   ///< Logic 0 / GND
    HIGH = 1   ///< Logic 1 / VCC
  };

 private:
  // Private constructor (static-only class)
  GPIO() = delete;
  ~GPIO() = delete;

  /**
   * @brief Validate GPIO pin number
   * @param pin GPIO number (0-39 for ESP32)
   * @return Status::OK if valid, Status::ERROR if invalid
   */
  static Status validatePin(uint8_t pin);

 public:
  /**
   * @brief Initialize a GPIO pin with specified mode
   * @param pin GPIO pin number (0-39)
   * @param mode Pin configuration (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN)
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin or mode
   *
   * Notes:
   * - Pins 34-39 support INPUT modes only (input-only pins on ESP32)
   * - Calling init() on already-initialized pin is safe (reconfigures)
   * - No internal state tracking (idempotent)
   */
  static Status init(uint8_t pin, Mode mode);

  /**
   * @brief Set GPIO pin to HIGH or LOW
   * @param pin GPIO pin number
   * @param level Desired logic level (HIGH or LOW)
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin or pin not initialized for output
   *
   * Notes:
   * - Non-blocking (immediate register write)
   * - Can be called on any pin (safe, but only meaningful for OUTPUT mode)
   */
  static Status write(uint8_t pin, Level level);

  /**
   * @brief Read GPIO pin state
   * @param pin GPIO pin number
   * @param level Output reference to receive pin state
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin
   *
   * Notes:
   * - Non-blocking (immediate register read)
   * - Valid for both INPUT and OUTPUT modes
   * - Returns the actual pin voltage level
   */
  static Status read(uint8_t pin, Level& level);

  /**
   * @brief Toggle GPIO pin state (HIGH → LOW or LOW → HIGH)
   * @param pin GPIO pin number
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin
   *
   * Notes:
   * - Non-blocking (immediate register write)
   * - Useful for LED blinking or pulse generation
   */
  static Status toggle(uint8_t pin);

  /**
   * @brief Change mode of already-initialized pin
   * @param pin GPIO pin number
   * @param mode New pin configuration
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin or mode
   *
   * Notes:
   * - Reconfigures pin without affecting other pins
   * - Equivalent to calling init() again with new mode
   */
  static Status setMode(uint8_t pin, Mode mode);
};

}  // namespace HAL
}  // namespace robocon
