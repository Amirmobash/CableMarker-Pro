#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

class MarkerActuator {
 public:
  void begin();
  void tick(uint32_t nowMs);
  bool requestMark(uint32_t nowMs);

  bool busy() const;
  uint32_t completedMarks() const;

 private:
  enum class Phase : uint8_t {
    Idle,
    MovingForward,
    Holding,
    MovingHome,
    Beeping,
  };

  Servo servo_;
  Phase phase_ = Phase::Idle;
  int currentAngle_ = 0;
  uint32_t nextActionMs_ = 0;
  uint32_t lastCompletedMs_ = 0;
  uint32_t completedMarks_ = 0;

  void writeAngle(int angle);
};
