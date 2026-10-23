#pragma once

#include <Arduino.h>

class LaserRangefinder {
 public:
  explicit LaserRangefinder(HardwareSerial& serial);

  void begin();
  void tick(uint32_t nowMs);
  void requestRestart(uint32_t nowMs);

  bool healthy(uint32_t nowMs) const;
  bool restarting() const;
  bool hasReading() const;
  int32_t rawMillimeters() const;
  uint32_t lastSuccessMs() const;

 private:
  enum class RestartState : uint8_t {
    Idle,
    PowerOffWait,
    PowerOnWait,
    UartRestartWait,
    LaserOnWait,
  };

  HardwareSerial& serial_;
  RestartState restartState_ = RestartState::Idle;
  uint32_t restartDeadlineMs_ = 0;
  uint32_t lastMeasurementMs_ = 0;
  uint32_t lastKeepAliveMs_ = 0;
  uint32_t lastSuccessMs_ = 0;
  int32_t latestMillimeters_ = -1;

  void serviceRestart(uint32_t nowMs);
  void measureOnce(uint32_t nowMs);
  bool readMeasurementFrame(uint32_t& millimeters);
  void sendLaserOn();
  void sendQuickMeasurement();
  void drainInput();
};
