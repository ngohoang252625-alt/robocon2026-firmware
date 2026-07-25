#include "HAL/GPIO.h"
#include <Arduino.h>

namespace robocon {
namespace HAL {

/**
 * @brief Validate GPIO pin number for ESP32
 * Valid range: 0-39 (40 GPIO pins on ESP32 DevKit V1)
 * Input-only pins: 34-39 (ADC-only, no output capability)
 */
Status GPIO::validatePin(uint8_t pin) {
  if (pin > 39) {
    return Status::ERROR;
  }
  return Status::OK;
}

Status GPIO::init(uint8_t pin, Mode mode) {
  // Validate pin number
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Validate mode for input-only pins (34-39)
  if (pin >= 34 && pin <= 39) {
    if (mode == Mode::OUTPUT) {
      return Status::ERROR;  // Cannot output on input-only pins
    }
  }

  // Configure pin using Arduino API
  switch (mode) {
    case Mode::INPUT:
      pinMode(pin, INPUT);
      break;

    case Mode::OUTPUT:
      pinMode(pin, OUTPUT);
      break;

    case Mode::INPUT_PULLUP:
      pinMode(pin, INPUT_PULLUP);
      break;

    case Mode::INPUT_PULLDOWN:
      pinMode(pin, INPUT_PULLDOWN);
      break;

    default:
      return Status::ERROR;
  }

  return Status::OK;
}

Status GPIO::write(uint8_t pin, Level level) {
  // Validate pin number
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Write pin state using Arduino API
  digitalWrite(pin, static_cast<uint8_t>(level));
  return Status::OK;
}

Status GPIO::read(uint8_t pin, Level& level) {
  // Validate pin number
  if (validatePin(pin) != Status::OK) {
    return Status::ERROR;
  }

  // Read pin state using Arduino API
  int pinState = digitalRead(pin);
  level = (pinState == HIGH) ? Level::HIGH : Level::LOW;
  return Status::OK;
}

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

Status GPIO::setMode(uint8_t pin, Mode mode) {
  // setMode is equivalent to init with new mode
  return init(pin, mode);
}

}  // namespace HAL
}  // namespace robocon
