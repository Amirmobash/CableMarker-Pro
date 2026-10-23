#pragma once

#include <stddef.h>
#include <stdint.h>

namespace cablemarker {

// Decodes the 13-byte measurement frame used by the rangefinder protocol in
// the original CableMarker Pro firmware. Returns false for malformed headers,
// invalid BCD digits, or checksum errors.
bool decodeMeasurementFrame(const uint8_t* frame, size_t length, uint32_t& millimeters);

}  // namespace cablemarker
