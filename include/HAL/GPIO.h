#pragma once

#include <cstdint>
#include "Utility/Status.h"

namespace robocon {
namespace HAL {

/**
 * @class GPIO
 * @brief Hardware Abstraction Layer for ESP32 GPIO operations using ESP-IDF driver
 *
 * Single Responsibility: Encapsulate ESP32 GPIO register access and pin configuration.
 * Provides a clean interface using native ESP-IDF GPIO functions without exposing hardware details.
 *
 * Thread Safety: 
 *   - gpio_set_level() is ISR-safe (can be called from interrupt context)
 *   - gpio_get_level() is ISR-safe
 *   - gpio_config() is NOT ISR-safe (must be called from task context only)
 *   - init() must NOT be called from ISR or concurrent tasks without external synchronization
 *   - External mutex required for concurrent init() calls
 *
 * Task Safety:
 *   - All operations are non-blocking
 *   - read() and write() are FreeRTOS task-safe
 *   - init() must serialize across tasks (use external mutex if called from multiple tasks)
 *
 * Pin Restrictions:
 *   - GPIO 6-11 are RESERVED for flash memory (invalid, rejected)
 *   - GPIO 0, 2 have boot-mode constraints (initialization possible but affects boot)
 *   - GPIO 34-39 are INPUT-ONLY (no output capability)
 *   - GPIO 1, 3 are UART0 TX/RX (reserved for Jetson link)
 *
 * Blocking: All methods are non-blocking.
 *
 * Production Quality:
 *   - No dynamic allocation
 *   - No blocking operations
 *   - Static allocation only
 *   - Input validation on all parameters
 *   - Error propagation via Status enum
 *   - Minimal overhead (direct register access via ESP-IDF)
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

  // ESP32 pin restrictions
  static constexpr uint8_t FLASH_RESERVED_START = 6;   ///< GPIO 6 reserved for flash
  static constexpr uint8_t FLASH_RESERVED_END = 11;    ///< GPIO 11 reserved for flash
  static constexpr uint8_t INPUT_ONLY_START = 34;      ///< GPIO 34-39 are input-only
  static constexpr uint8_t INPUT_ONLY_END = 39;
  static constexpr uint8_t MAX_GPIO = 40;              ///< Total GPIO pins on ESP32

  /**
   * @brief Validate GPIO pin number
   * @param pin GPIO number (0-39 for ESP32)
   * @return Status::OK if valid, Status::ERROR if invalid or reserved
   *
   * Rejects:
   *   - pin > 39 (out of range)
   *   - pin 6-11 (reserved for flash memory)
   */
  static Status validatePin(uint8_t pin);

 public:
  /**
   * @brief Initialize a GPIO pin with specified mode using ESP-IDF gpio_config()
   * @param pin GPIO pin number (0-39, excluding 6-11)
   * @param mode Pin configuration (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN)
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin, reserved pin (6-11), invalid mode, or configuration failed
   *
   * Thread Safety: NOT ISR-safe. Must be called from task context only.
   * Blocking: No (register write only).
   *
   * Implementation uses ESP-IDF gpio_config():
   *   - Configures pin mode, direction, pull-up/pull-down
   *   - Disables ISR on init (ISR not managed by HAL GPIO)
   *   - Configures pin as GPIO (not peripheral function)
   *
   * Notes:
   *   - Pins 34-39 support INPUT modes only (input-only pins on ESP32)
   *   - Pins 6-11 are REJECTED (reserved for flash memory)
   *   - Calling init() on already-initialized pin reconfigures it (safe)
   *   - No internal state tracking (idempotent at ESP-IDF level, but should serialize)
   *   - For thread-safe concurrent initialization, caller must provide mutex
   */
  static Status init(uint8_t pin, Mode mode);

  /**
   * @brief Set GPIO pin to HIGH or LOW using ESP-IDF gpio_set_level()
   * @param pin GPIO pin number
   * @param level Desired logic level (HIGH or LOW)
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin
   *
   * Thread Safety: ISR-safe. Can be called from interrupt context.
   * Blocking: No (direct register write).
   * Time: O(1), microseconds.
   *
   * Implementation uses ESP-IDF gpio_set_level():
   *   - Directly modifies GPIO output register
   *   - No validation at register level (validation done at init time)
   *
   * Notes:
   *   - Non-blocking (immediate register write)
   *   - Can be called on any pin (safe, but only meaningful for OUTPUT mode)
   *   - Safe to call from ISR, FreeRTOS task, or main loop
   *   - No resource contention (atomic write)
   */
  static Status write(uint8_t pin, Level level);

  /**
   * @brief Read GPIO pin state using ESP-IDF gpio_get_level()
   * @param pin GPIO pin number
   * @param level Output reference to receive pin state
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin
   *
   * Thread Safety: ISR-safe. Can be called from interrupt context.
   * Blocking: No (direct register read).
   * Time: O(1), microseconds.
   *
   * Implementation uses ESP-IDF gpio_get_level():
   *   - Directly reads GPIO input register
   *   - Returns instantaneous pin voltage (not latched)
   *   - No validation at register level
   *
   * Notes:
   *   - Non-blocking (immediate register read)
   *   - Valid for both INPUT and OUTPUT modes
   *   - Returns the actual instantaneous pin voltage level
   *   - Safe to call from ISR, FreeRTOS task, or main loop
   *   - No resource contention (atomic read)
   */
  static Status read(uint8_t pin, Level& level);

  /**
   * @brief Toggle GPIO pin state using read + write sequence
   * @param pin GPIO pin number
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin or operation failed
   *
   * Thread Safety: NOT ISR-safe (read-modify-write sequence is not atomic).
   * Blocking: No (register operations only).
   *
   * Implementation:
   *   - Reads current pin state via gpio_get_level()
   *   - Inverts state
   *   - Writes new state via gpio_set_level()
   *   - Not atomic: possible race condition if pin is written concurrently
   *
   * Notes:
   *   - NOT safe to call from ISR (other ISR could write pin between read and write)
   *   - Safe to call from single task context
   *   - Useful for LED blinking or pulse generation from task
   *   - If concurrent access expected, caller must serialize with mutex
   */
  static Status toggle(uint8_t pin);

  /**
   * @brief Change mode of already-initialized pin
   * @param pin GPIO pin number
   * @param mode New pin configuration
   * @return Status::OK if successful
   * @return Status::ERROR if invalid pin, reserved pin, or mode invalid
   *
   * Thread Safety: NOT ISR-safe. Must be called from task context only.
   * Blocking: No (register write only).
   *
   * Notes:
   *   - Reconfigures pin without affecting other pins
   *   - Equivalent to calling init() again with new mode
   *   - For thread-safe concurrent calls, caller must provide mutex
   */
  static Status setMode(uint8_t pin, Mode mode);
};

}  // namespace HAL
}  // namespace robocon
