#pragma once

#include <Arduino.h>

namespace config {

// Project identity
static constexpr char kProjectName[] = "CableMarker Pro";
static constexpr char kFirmwareVersion[] = "2.0.0";

// ESP32 pin assignment (kept compatible with the original project)
static constexpr uint8_t kLaserRxPin = 16;
static constexpr uint8_t kLaserTxPin = 17;
static constexpr uint8_t kLaserEnablePin = 5;
static constexpr uint8_t kServoPin = 18;
static constexpr uint8_t kBuzzerPin = 27;
static constexpr uint8_t kStatusLedPin = 25;
static constexpr uint8_t kBuiltinLedPin = 2;
static constexpr uint8_t kTriggerOutPin = 23;
static constexpr uint8_t kTriggerInPin = 19;

// Laser UART / protocol
static constexpr uint32_t kLaserBaud = 9600;
static constexpr uint32_t kMeasurementPeriodMs = 750;
static constexpr uint32_t kLaserKeepAliveMs = 5000;
static constexpr uint32_t kLaserStaleAfterMs = 3000;
static constexpr uint32_t kFrameReadTimeoutMs = 140;

// Calibration. Keep the legacy -33 mm offset unless your mechanical setup differs.
static constexpr int32_t kCalibrationOffsetMm = -33;

// Marker actuator
static constexpr int kServoHomeAngle = 30;
static constexpr int kServoTravelAngle = 120;
static constexpr int kServoMinAngle = 20;
static constexpr int kServoMaxAngle = 125;
static constexpr uint8_t kServoStepDegrees = 4;
static constexpr uint32_t kServoStepIntervalMs = 10;
static constexpr uint32_t kServoHoldMs = 80;
static constexpr uint32_t kBuzzerPulseMs = 100;
static constexpr uint32_t kMinimumMarkIntervalMs = 800;

// External trigger
static constexpr uint32_t kTriggerDebounceMs = 25;

// Standalone Wi-Fi access point
static constexpr char kAccessPointSsid[] = "CableMarker-Pro";
static constexpr char kAccessPointPassword[] = "CableMarker100";

}  // namespace config
