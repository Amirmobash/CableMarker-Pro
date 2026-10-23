#include "LaserProtocol.h"

namespace cablemarker {
namespace {

bool isBcdByte(uint8_t value) {
  return ((value >> 4) & 0x0F) <= 9 && (value & 0x0F) <= 9;
}

}  // namespace

bool decodeMeasurementFrame(const uint8_t* frame, size_t length, uint32_t& millimeters) {
  if (frame == nullptr || length != 13) {
    return false;
  }

  if (frame[0] != 0xAA || frame[4] != 0x00 || frame[5] != 0x04) {
    return false;
  }

  uint8_t checksum = 0;
  for (size_t i = 1; i < 12; ++i) {
    checksum = static_cast<uint8_t>(checksum + frame[i]);
  }
  if (checksum != frame[12]) {
    return false;
  }

  uint32_t decoded = 0;
  for (size_t i = 6; i < 10; ++i) {
    if (!isBcdByte(frame[i])) {
      return false;
    }
    decoded = decoded * 100U;
    decoded += static_cast<uint32_t>(((frame[i] >> 4) & 0x0F) * 10U + (frame[i] & 0x0F));
  }

  millimeters = decoded;
  return true;
}

}  // namespace cablemarker
