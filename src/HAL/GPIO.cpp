#include "HAL/GPIO.h"
#include "driver/gpio.h"

namespace robocon {
namespace HAL {

/**
 * @brief Validate GPIO pin number for ESP32
 * Valid range: 0-5, 12-33 (GPIO 6-11 reserved for flash)
 * Input-only pins: 34-39
 *
 * Rejects GPIO 6-11 which are reserved for flash memory on ESP32.
 * These pins must never be configured as GPIO.
 */
Status GPIO::validatePin(uint8_t pin) {
  // Out of range
  if (pin >= MAX_GPIO) {
    return Status::ERROR;
  }

  // GPIO 6-11 reserved for flash (CRITICAL: must reject)
  if (pin >= FLASH_RESERVED_START && pin <= FLASH_RESERVED_END) {
    return Status::ERROR;
  }

  return Status::OK;
}

/**
 * @brief Initialize GPIO pin using ESP-IDF gpio_config()
 * Validates pin, checks for input-only restriction, configures via gpio_config().
 */
Status GPIO::init(uint8_t pin, Mode mode) {
  // Validate pin number (rejects reserved 6-11)
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Validate mode for input-only pins (34-39)
  if (pin >= INPUT_ONLY_START && pin <= INPUT_ONLY_END) {
    if (mode == Mode::OUTPUT) {
      return Status::ERROR;  // Cannot output on input-only pins
    }
  }

  // Configure pin using ESP-IDF gpio_config()
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << pin);
  io_conf.intr_type = GPIO_INTR_DISABLE;  // No ISR (HAL does not manage ISR)

  // Set mode, pull-up/pull-down
  switch (mode) {
    case Mode::INPUT:
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;

    case Mode::OUTPUT:
      io_conf.mode = GPIO_MODE_OUTPUT;
      io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;

    case Mode::INPUT_PULLUP:
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;

    case Mode::INPUT_PULLDOWN:
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
      break;

    default:
      return Status::ERROR;
  }

  // Apply configuration via ESP-IDF
  esp_err_t ret = gpio_config(&io_conf);
  if (ret != ESP_OK) {
    return Status::ERROR;
  }

  return Status::OK;
}

/**
 * @brief Write GPIO pin using ESP-IDF gpio_set_level()
 * ISR-safe. No validation (assumes init() was called).
 */
Status GPIO::write(uint8_t pin, Level level) {
  // Basic validation (pin range)
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Write using ESP-IDF (ISR-safe)
  gpio_set_level(static_cast<gpio_num_t>(pin), static_cast<uint32_t>(level));
  return Status::OK;
}

/**
 * @brief Read GPIO pin using ESP-IDF gpio_get_level()
 * ISR-safe. Returns instantaneous pin level.
 */
Status GPIO::read(uint8_t pin, Level& level) {
  // Basic validation (pin range)
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Read using ESP-IDF (ISR-safe)
  uint32_t pin_level = gpio_get_level(static_cast<gpio_num_t>(pin));
  level = (pin_level != 0) ? Level::HIGH : Level::LOW;
  return Status::OK;
}

/**
 * @brief Toggle GPIO pin state
 * NOT ISR-safe (read-modify-write is not atomic).
 * Safe for single-task context only.
 */
Status GPIO::toggle(uint8_t pin) {
  // Validate pin number
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Read current state and toggle
  Level currentLevel;
  if (read(pin, currentLevel) != Status::OK) {
    return Status::ERROR;
  }

  Level newLevel = (currentLevel == Level::HIGH) ? Level::LOW : Level::HIGH;
  return write(pin, newLevel);
}

/**
 * @brief Set mode of already-initialized pin
 * Equivalent to init() with new mode.
 */
Status GPIO::setMode(uint8_t pin, Mode mode) {
  // setMode is equivalent to init with new mode
  return init(pin, mode);
}

}  // namespace HAL
}  // namespace robocon
