#include "MarkerActuator.h"

#include "AppConfig.h"

namespace {

bool deadlineReached(uint32_t nowMs, uint32_t deadlineMs) {
  return static_cast<int32_t>(nowMs - deadlineMs) >= 0;
}

}  // namespace

void MarkerActuator::begin() {
  pinMode(config::kBuzzerPin, OUTPUT);
  digitalWrite(config::kBuzzerPin, LOW);

  servo_.attach(config::kServoPin);
  currentAngle_ = config::kServoHomeAngle;
  writeAngle(currentAngle_);
}

void MarkerActuator::tick(uint32_t nowMs) {
  if (phase_ == Phase::Idle || !deadlineReached(nowMs, nextActionMs_)) {
    return;
  }

  switch (phase_) {
    case Phase::MovingForward:
      currentAngle_ += config::kServoStepDegrees;
      if (currentAngle_ >= config::kServoTravelAngle) {
        currentAngle_ = config::kServoTravelAngle;
        writeAngle(currentAngle_);
        phase_ = Phase::Holding;
        nextActionMs_ = nowMs + config::kServoHoldMs;
      } else {
        writeAngle(currentAngle_);
        nextActionMs_ = nowMs + config::kServoStepIntervalMs;
      }
      break;

    case Phase::Holding:
      phase_ = Phase::MovingHome;
      nextActionMs_ = nowMs;
      break;

    case Phase::MovingHome:
      currentAngle_ -= config::kServoStepDegrees;
      if (currentAngle_ <= config::kServoHomeAngle) {
        currentAngle_ = config::kServoHomeAngle;
        writeAngle(currentAngle_);
        digitalWrite(config::kBuzzerPin, HIGH);
        phase_ = Phase::Beeping;
        nextActionMs_ = nowMs + config::kBuzzerPulseMs;
      } else {
        writeAngle(currentAngle_);
        nextActionMs_ = nowMs + config::kServoStepIntervalMs;
      }
      break;

    case Phase::Beeping:
      digitalWrite(config::kBuzzerPin, LOW);
      phase_ = Phase::Idle;
      lastCompletedMs_ = nowMs;
      ++completedMarks_;
      break;

    case Phase::Idle:
      break;
  }
}

bool MarkerActuator::requestMark(uint32_t nowMs) {
  if (phase_ != Phase::Idle) {
    return false;
  }

  if (lastCompletedMs_ != 0 && nowMs - lastCompletedMs_ < config::kMinimumMarkIntervalMs) {
    return false;
  }

  phase_ = Phase::MovingForward;
  currentAngle_ = config::kServoHomeAngle;
  nextActionMs_ = nowMs;
  return true;
}

bool MarkerActuator::busy() const {
  return phase_ != Phase::Idle;
}

uint32_t MarkerActuator::completedMarks() const {
  return completedMarks_;
}

void MarkerActuator::writeAngle(int angle) {
  servo_.write(constrain(angle, config::kServoMinAngle, config::kServoMaxAngle));
}
