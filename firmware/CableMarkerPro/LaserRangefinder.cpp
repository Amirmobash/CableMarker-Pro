#include "LaserRangefinder.h"

#include "AppConfig.h"
#include "LaserProtocol.h"

namespace {

const uint8_t kLaserOnCommand[] = {0xAA, 0x00, 0x01, 0xBE, 0x00, 0x01, 0x00, 0x01, 0xC1};
const uint8_t kQuickMeasurementCommand[] = {0xAA, 0x00, 0x00, 0x22, 0x00, 0x01, 0x00, 0x00, 0x23};

bool deadlineReached(uint32_t nowMs, uint32_t deadlineMs) {
  return static_cast<int32_t>(nowMs - deadlineMs) >= 0;
}

}  // namespace

LaserRangefinder::LaserRangefinder(HardwareSerial& serial) : serial_(serial) {}

void LaserRangefinder::begin() {
  pinMode(config::kLaserEnablePin, OUTPUT);
  digitalWrite(config::kLaserEnablePin, HIGH);

  serial_.begin(config::kLaserBaud, SERIAL_8N1, config::kLaserRxPin, config::kLaserTxPin);
  // The legacy hardware allowed the rangefinder a short power-up period before
  // the first command. This happens before Wi-Fi/server startup, so a one-time
  // boot delay is preferable to sending a command while the sensor is not ready.
  delay(400);
  sendLaserOn();

  const uint32_t nowMs = millis();
  lastMeasurementMs_ = nowMs - config::kMeasurementPeriodMs;
  lastKeepAliveMs_ = nowMs;
}

void LaserRangefinder::tick(uint32_t nowMs) {
  if (restartState_ != RestartState::Idle) {
    serviceRestart(nowMs);
    return;
  }

  if (nowMs - lastKeepAliveMs_ >= config::kLaserKeepAliveMs) {
    sendLaserOn();
    lastKeepAliveMs_ = nowMs;
  }

  if (nowMs - lastMeasurementMs_ >= config::kMeasurementPeriodMs) {
    lastMeasurementMs_ = nowMs;
    measureOnce(nowMs);
  }
}

void LaserRangefinder::requestRestart(uint32_t nowMs) {
  if (restartState_ != RestartState::Idle) {
    return;
  }

  digitalWrite(config::kLaserEnablePin, LOW);
  restartState_ = RestartState::PowerOffWait;
  restartDeadlineMs_ = nowMs + 100;
  lastSuccessMs_ = 0;
  latestMillimeters_ = -1;
}

bool LaserRangefinder::healthy(uint32_t nowMs) const {
  if (restartState_ != RestartState::Idle || lastSuccessMs_ == 0) {
    return false;
  }
  return nowMs - lastSuccessMs_ <= config::kLaserStaleAfterMs;
}

bool LaserRangefinder::restarting() const {
  return restartState_ != RestartState::Idle;
}

bool LaserRangefinder::hasReading() const {
  return latestMillimeters_ >= 0;
}

int32_t LaserRangefinder::rawMillimeters() const {
  return latestMillimeters_;
}

uint32_t LaserRangefinder::lastSuccessMs() const {
  return lastSuccessMs_;
}

void LaserRangefinder::serviceRestart(uint32_t nowMs) {
  if (!deadlineReached(nowMs, restartDeadlineMs_)) {
    return;
  }

  switch (restartState_) {
    case RestartState::PowerOffWait:
      digitalWrite(config::kLaserEnablePin, HIGH);
      restartState_ = RestartState::PowerOnWait;
      restartDeadlineMs_ = nowMs + 500;
      break;

    case RestartState::PowerOnWait:
      serial_.end();
      serial_.begin(config::kLaserBaud, SERIAL_8N1, config::kLaserRxPin, config::kLaserTxPin);
      restartState_ = RestartState::UartRestartWait;
      restartDeadlineMs_ = nowMs + 100;
      break;

    case RestartState::UartRestartWait:
      drainInput();
      sendLaserOn();
      restartState_ = RestartState::LaserOnWait;
      restartDeadlineMs_ = nowMs + 300;
      break;

    case RestartState::LaserOnWait:
      restartState_ = RestartState::Idle;
      lastMeasurementMs_ = nowMs - config::kMeasurementPeriodMs;
      lastKeepAliveMs_ = nowMs;
      break;

    case RestartState::Idle:
      break;
  }
}

void LaserRangefinder::measureOnce(uint32_t nowMs) {
  drainInput();
  sendQuickMeasurement();

  uint32_t millimeters = 0;
  if (readMeasurementFrame(millimeters)) {
    latestMillimeters_ = static_cast<int32_t>(millimeters);
    lastSuccessMs_ = nowMs;
  }
}

bool LaserRangefinder::readMeasurementFrame(uint32_t& millimeters) {
  uint8_t frame[13] = {};
  size_t count = 0;
  const uint32_t startedAt = millis();

  while (millis() - startedAt < config::kFrameReadTimeoutMs) {
    while (serial_.available() > 0) {
      const int next = serial_.read();
      if (next < 0) {
        break;
      }

      const uint8_t value = static_cast<uint8_t>(next);
      if (count == 0) {
        if (value != 0xAA) {
          continue;
        }
        frame[count++] = value;
      } else {
        frame[count++] = value;
      }

      if (count == sizeof(frame)) {
        return cablemarker::decodeMeasurementFrame(frame, sizeof(frame), millimeters);
      }
    }
    delay(1);  // Yield to the ESP32 Wi-Fi stack while waiting for UART bytes.
  }

  return false;
}

void LaserRangefinder::sendLaserOn() {
  serial_.write(kLaserOnCommand, sizeof(kLaserOnCommand));
}

void LaserRangefinder::sendQuickMeasurement() {
  serial_.write(kQuickMeasurementCommand, sizeof(kQuickMeasurementCommand));
}

void LaserRangefinder::drainInput() {
  // Bound the drain so a noisy UART cannot trap the main loop forever.
  for (size_t i = 0; i < 64 && serial_.available() > 0; ++i) {
    serial_.read();
  }
}
