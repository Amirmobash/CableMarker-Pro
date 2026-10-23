#pragma once

#include <Arduino.h>

class DebouncedInput {
 public:
  DebouncedInput(uint8_t pin, uint32_t debounceMs)
      : pin_(pin), debounceMs_(debounceMs) {}

  void begin(uint8_t mode) {
    pinMode(pin_, mode);
    rawState_ = digitalRead(pin_);
    stableState_ = rawState_;
    lastRawChangeMs_ = millis();
  }

  bool risingEdge(uint32_t nowMs) {
    const bool raw = digitalRead(pin_);
    if (raw != rawState_) {
      rawState_ = raw;
      lastRawChangeMs_ = nowMs;
    }

    if (rawState_ != stableState_ && nowMs - lastRawChangeMs_ >= debounceMs_) {
      stableState_ = rawState_;
      return stableState_;
    }

    return false;
  }

 private:
  uint8_t pin_;
  uint32_t debounceMs_;
  bool rawState_ = false;
  bool stableState_ = false;
  uint32_t lastRawChangeMs_ = 0;
};
