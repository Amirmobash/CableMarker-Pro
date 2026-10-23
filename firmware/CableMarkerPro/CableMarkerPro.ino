#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "AppConfig.h"
#include "DebouncedInput.h"
#include "LaserRangefinder.h"
#include "MarkerActuator.h"
#include "WebUi.h"

HardwareSerial laserSerial(2);
LaserRangefinder laser(laserSerial);
MarkerActuator marker;
DebouncedInput triggerInput(config::kTriggerInPin, config::kTriggerDebounceMs);
WebServer server(80);

namespace {

bool statusLedState = false;
uint32_t lastStatusLedToggleMs = 0;

int32_t calibratedDistanceMm(uint32_t nowMs) {
  if (!laser.hasReading() || !laser.healthy(nowMs)) {
    return -1;
  }

  const int64_t corrected = static_cast<int64_t>(laser.rawMillimeters()) + config::kCalibrationOffsetMm;
  return corrected < 0 ? 0 : static_cast<int32_t>(corrected);
}

String jsonState(uint32_t nowMs, const char* message = nullptr) {
  String json;
  json.reserve(320);
  json += F("{\"version\":\"");
  json += config::kFirmwareVersion;
  json += F("\",\"distance_mm\":");
  json += calibratedDistanceMm(nowMs);
  json += F(",\"raw_distance_mm\":");
  json += laser.hasReading() ? String(laser.rawMillimeters()) : String(-1);
  json += F(",\"calibration_offset_mm\":");
  json += config::kCalibrationOffsetMm;
  json += F(",\"laser_healthy\":");
  json += laser.healthy(nowMs) ? F("true") : F("false");
  json += F(",\"laser_restarting\":");
  json += laser.restarting() ? F("true") : F("false");
  json += F(",\"marker_busy\":");
  json += marker.busy() ? F("true") : F("false");
  json += F(",\"completed_marks\":");
  json += marker.completedMarks();
  json += F(",\"uptime_ms\":");
  json += nowMs;

  if (message != nullptr) {
    json += F(",\"message\":\"");
    json += message;
    json += '"';
  }

  json += '}';
  return json;
}

void sendJson(int statusCode, const String& body) {
  server.sendHeader(F("Cache-Control"), F("no-store"));
  server.send(statusCode, F("application/json; charset=utf-8"), body);
}

void handleStatus() {
  sendJson(200, jsonState(millis()));
}

void handleMark() {
  const uint32_t nowMs = millis();
  if (!marker.requestMark(nowMs)) {
    sendJson(409, jsonState(nowMs, "Marker is busy or in cooldown."));
    return;
  }
  sendJson(202, jsonState(nowMs, "Mark cycle started."));
}

void handleLaserRestart() {
  const uint32_t nowMs = millis();
  laser.requestRestart(nowMs);
  sendJson(202, jsonState(nowMs, "Laser restart requested."));
}

void handleLegacyData() {
  const uint32_t nowMs = millis();
  String json = F("{\"d\":");
  json += calibratedDistanceMm(nowMs);
  json += F(",\"h\":");
  json += laser.healthy(nowMs) ? F("true") : F("false");
  json += F(",\"b\":");
  json += marker.busy() ? F("true") : F("false");
  json += '}';
  sendJson(200, json);
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, PSTR("text/html; charset=utf-8"), kWebUi);
  });

  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/mark", HTTP_POST, handleMark);
  server.on("/api/laser/restart", HTTP_POST, handleLaserRestart);

  // Compatibility with the original one-file firmware. New integrations should
  // use the /api/* endpoints above.
  server.on("/d", HTTP_GET, handleLegacyData);
  server.on("/m", HTTP_GET, handleMark);
  server.on("/m", HTTP_POST, handleMark);
  server.on("/r", HTTP_GET, handleLaserRestart);
  server.on("/r", HTTP_POST, handleLaserRestart);

  server.on("/healthz", HTTP_GET, []() {
    server.send(200, F("text/plain; charset=utf-8"), F("ok"));
  });

  server.onNotFound([]() {
    server.send(404, F("application/json; charset=utf-8"), F("{\"error\":\"not_found\"}"));
  });
}

void updateStatusIndicators(uint32_t nowMs) {
  const bool healthy = laser.healthy(nowMs);
  digitalWrite(config::kBuiltinLedPin, healthy ? HIGH : LOW);

  if (marker.busy()) {
    digitalWrite(config::kStatusLedPin, HIGH);
    statusLedState = true;
    return;
  }

  const uint32_t intervalMs = healthy ? 800 : 200;
  if (nowMs - lastStatusLedToggleMs >= intervalMs) {
    lastStatusLedToggleMs = nowMs;
    statusLedState = !statusLedState;
    digitalWrite(config::kStatusLedPin, statusLedState ? HIGH : LOW);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);

  pinMode(config::kStatusLedPin, OUTPUT);
  pinMode(config::kBuiltinLedPin, OUTPUT);
  pinMode(config::kTriggerOutPin, OUTPUT);
  digitalWrite(config::kTriggerOutPin, HIGH);  // Preserves the legacy ready/output behavior.

  triggerInput.begin(INPUT_PULLDOWN);
  marker.begin();
  laser.begin();

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(config::kAccessPointSsid, config::kAccessPointPassword);

  setupRoutes();
  server.begin();

  Serial.println();
  Serial.printf("%s firmware %s\n", config::kProjectName, config::kFirmwareVersion);
  Serial.printf("Access point: %s\n", config::kAccessPointSsid);
  Serial.print("Web UI: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  const uint32_t nowMs = millis();

  laser.tick(nowMs);
  marker.tick(nowMs);

  if (triggerInput.risingEdge(nowMs)) {
    marker.requestMark(nowMs);
  }

  updateStatusIndicators(nowMs);
  server.handleClient();
  delay(1);
}
