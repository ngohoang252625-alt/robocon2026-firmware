/**
 * @file src/main.cpp
 * @brief Arduino application entry point for Robocon O2 2026 Firmware
 *
 * This is the permanent entry point that integrates platform components progressively
 * as implementation proceeds. No temporary stubs; every addition is production code.
 *
 * Thread Safety: setup() runs once at boot (single-threaded).
 *                After FreeRTOS scheduler starts, loop() is NOT called.
 *
 * Blocking: setup() performs blocking operations (platform initialization).
 *           loop() is non-blocking (yields control to scheduler).
 */

#include <Arduino.h>

/**
 * @brief Arduino setup() entry point
 *
 * Called once at ESP32 boot, before loop() starts.
 * Initializes only components that have been implemented so far.
 *
 * Current stage:
 *   - Initialize USB Serial for debug output
 *
 * Future stages will add:
 *   - HAL initialization (PWM, UART, I2C, Timer, PCNT)
 *   - Driver initialization (BTS7960, DRV8825, Encoder, MPU6050, HMC5883L, LimitSwitch)
 *   - Middleware initialization (PID, Filters, Odometry, Kinematics, CRC16, Packet)
 *   - Application initialization (Chassis, Lift, Navigator, Mission, Robot)
 *   - FreeRTOS task creation and scheduler startup
 */
void setup() {
  // Initialize USB Serial for debug/logging
  // Baud rate: 115200 (per Config::UART_BAUDRATE, used for USB serial only)
  // Note: GPIO1/GPIO3 (UART0) are reserved for Jetson link, not used for debug
  Serial.begin(115200);
}

/**
 * @brief Arduino loop() entry point
 *
 * Called repeatedly by Arduino framework (after setup() completes).
 *
 * Current stage:
 *   - Minimal: yield control
 *
 * Future stages will add:
 *   - Platform component updates
 *   - FreeRTOS task scheduling (scheduler takes over main loop)
 */
void loop() {
  yield();
}
