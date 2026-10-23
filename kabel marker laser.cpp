#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ===== Pins
#define PIN_RX        16
#define PIN_TX        17
#define PIN_ENA       5
#define PIN_SERVO     18
#define PIN_BUZZER    27
#define PIN_BLINK     25
#define LED_BUILTIN   2

// Trigger-Pins
#define PIN_TRIG_OUT  23
#define PIN_TRIG_IN   19

#define UART_BAUD     9600

// WiFi AP
const char* AP_SSID = "KabelMarker_Pro";
const char* AP_PASS = "";
WebServer server(80);

// UART2 für Laser
HardwareSerial LZR(2);
Servo markerServo;

// ===== Laser-Kommandos
const uint8_t CMD_LASER_ON[]  = {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x01,0xC1};
const uint8_t CMD_QUICK[]     = {0xAA,0x00,0x00,0x22,0x00,0x01,0x00,0x00,0x23};

// ===== Zustände
long last_mm = -1;
bool communicationHealthy = false;
bool markBusy = false;
unsigned long lastMarkTime = 0;

unsigned long lastSuccessTime = 0;
unsigned long keepAliveT = 0, blinkT = 0;

// ===== Servo
int HOME_ANGLE = 30;
int TRAVEL_DEG = 120;

// ===== Kalibrierung
const long CALIB_OFFSET = -33;

// ===== Timing
const unsigned long MIN_MARK_INTERVAL_MS = 800;

// ===== Helper Functions - OPTIMIZED
void laserOn() { 
  digitalWrite(PIN_ENA, HIGH); 
}

void clearSerial() { 
  while(LZR.available()) LZR.read();
}

void sendCmd(const uint8_t *p, size_t n) { 
  LZR.write(p, n); 
  delay(2); // REDUCED
}

bool readFrame(long &mm) {
  if (!LZR.available()) return false;
  
  uint8_t f[13];
  int bytes = 0;
  unsigned long start = millis();
  
  // Schnellere Header-Suche
  while(millis() - start < 200 && bytes == 0) { 
    if(LZR.available()) { 
      if(LZR.read() == 0xAA) { 
        f[0] = 0xAA; 
        bytes = 1; 
        break; 
      } 
    } 
  }
  
  if(bytes == 0) return false;
  
  // Schnelleres Lesen
  start = millis();
  while(bytes < 13 && millis() - start < 100) {
    if(LZR.available()) {
      f[bytes++] = LZR.read();
    }
  }
  
  if(bytes < 13) return false;
  
  // Schnellere Validierung
  if(!(f[0]==0xAA && f[4]==0x00 && f[5]==0x04)) return false;
  
  // Checksum beschleunigt
  uint8_t sum = 0;
  for(int i=1; i<12; i++) sum += f[i];
  if(sum != f[12]) return false;
  
  // BCD Konvertierung beschleunigt
  mm = 0;
  for(int i=6; i<10; i++) {
    mm = mm * 100 + ((f[i]>>4)&0x0F)*10 + (f[i]&0x0F);
  }
  
  return true;
}

void restartLaser() {
  digitalWrite(PIN_ENA, LOW);
  delay(100);
  digitalWrite(PIN_ENA, HIGH);
  delay(500);
  
  LZR.end();
  delay(50);
  LZR.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(100);
  
  sendCmd(CMD_LASER_ON, sizeof(CMD_LASER_ON));
  delay(300);

  // Schnellere Initialisierung
  long mm;
  for(int i=0; i<2; i++) {
    clearSerial();
    sendCmd(CMD_QUICK, sizeof(CMD_QUICK));
    delay(25);
    if(readFrame(mm)) {
      communicationHealthy = true;
      last_mm = mm;
      lastSuccessTime = millis();
      digitalWrite(LED_BUILTIN, HIGH);
      return;
    }
  }
  
  communicationHealthy = false;
  digitalWrite(LED_BUILTIN, LOW);
}

bool measureDistance() {
  clearSerial();
  sendCmd(CMD_QUICK, sizeof(CMD_QUICK));
  delay(25); // REDUCED
  
  long mm;
  if(readFrame(mm)) {
    last_mm = mm;
    communicationHealthy = true;
    lastSuccessTime = millis();
    digitalWrite(LED_BUILTIN, HIGH);
    return true;
  }
  
  communicationHealthy = false;
  digitalWrite(LED_BUILTIN, LOW);
  return false;
}

void setServoAngle(int angle) { 
  markerServo.write(constrain(angle, 20, 125)); 
  delay(10); // REDUCED
}

void beep(int n) { 
  for(int i=0; i<n; i++) {
    digitalWrite(PIN_BUZZER, HIGH); 
    delay(80); 
    digitalWrite(PIN_BUZZER, LOW); 
    if(i<n-1) delay(40); // REDUCED
  }
}

void doMarkCycle() {
  if(markBusy) return;
  
  if(millis() - lastMarkTime < MIN_MARK_INTERVAL_MS) return;
  
  markBusy = true;
  
  // Beschleunigte Servo-Bewegung
  for(int angle = HOME_ANGLE; angle <= TRAVEL_DEG; angle += 4) { // FASTER
    setServoAngle(angle);
    delay(8); // REDUCED
  }
  delay(80); // REDUCED
  
  for(int angle = TRAVEL_DEG; angle >= HOME_ANGLE; angle -= 4) { // FASTER
    setServoAngle(angle);
    delay(8); // REDUCED
  }
  
  digitalWrite(PIN_BUZZER, HIGH); 
  delay(100); // REDUCED
  digitalWrite(PIN_BUZZER, LOW);
  
  lastMarkTime = millis();
  markBusy = false;
}

long calibrated_mm() { 
  return (last_mm < 0) ? -1 : last_mm + CALIB_OFFSET;
}

// ===== Web Interface - OPTIMIZED
String html() {
  String distHuman = (last_mm >= 0) ? String(calibrated_mm() / 1000.0, 3) + " m" : "---";
  String distRaw = (last_mm >= 0) ? String(calibrated_mm()) + " mm" : "Warte...";
  String statusColor = communicationHealthy ? "#22c55e" : "#ef4444";
  String statusText = communicationHealthy ? "Verbunden" : "Getrennt";
  String busyText = markBusy ? "Aktiv" : "Bereit";
  
  return R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>KabelMarker Pro</title>
<style>
body{font-family:Arial,sans-serif;background:#0f0f0f;color:#fff;margin:0;padding:16px;text-align:center}
.container{max-width:400px;margin:0 auto}
.header{background:#1a1a1a;padding:20px;border-radius:12px;margin-bottom:12px}
.distance{font-size:42px;font-weight:bold;color:#f97316;margin:8px 0}
.status{display:inline-block;padding:6px 12px;border-radius:16px;margin:4px;font-size:13px}
.button{background:#f97316;color:white;border:none;padding:14px;border-radius:8px;font-size:16px;width:100%;margin:4px 0;cursor:pointer}
.button:active{background:#ea580c}
.info{background:#1a1a1a;padding:12px;border-radius:8px;margin:8px 0;font-size:13px;color:#86efac}
</style></head><body>
<div class="container">
<div class="header">
<h1>🎯 KabelMarker Pro</h1><p>Amir Messsystem</p>
</div>
<div class="header">
<div class="distance" id="distance">)rawliteral" + distHuman + R"rawliteral(</div>
<div style="color:#aaa;font-size:13px" id="rawDistance">)rawliteral" + distRaw + R"rawliteral(</div>
<div class="status" id="commStatus" style="background:)rawliteral" + statusColor + R"rawliteral(20;color:)rawliteral" + statusColor + R"rawliteral(;">Laser: )rawliteral" + statusText + R"rawliteral(</div>
<div class="status" id="busyStatus" style="background:#22c55e20;color:#22c55e;">Markierung: )rawliteral" + busyText + R"rawliteral(</div>
</div>
<button class="button" onclick="mark()">🎯 MARKIEREN</button>
<button class="button" onclick="restartLaser()" style="background:#374151;">🔄 LASER NEUSTART</button>
<div class="info">✅ -33mm Kalibrierung aktiv</div>
</div>
<script>
function update(){fetch('/d').then(r=>r.json()).then(d=>{
if(d.d>=0){document.getElementById('distance').textContent=(d.d/1000).toFixed(3)+' m';document.getElementById('rawDistance').textContent=d.d+' mm'}
document.getElementById('commStatus').textContent='Laser: '+(d.h?'Verbunden':'Getrennt');document.getElementById('commStatus').style.background=d.h?'#22c55e20':'#ef444420';document.getElementById('commStatus').style.color=d.h?'#22c55e':'#ef4444';
document.getElementById('busyStatus').textContent='Markierung: '+(d.b?'Aktiv':'Bereit');document.getElementById('busyStatus').style.background=d.b?'#f59e0b20':'#22c55e20';document.getElementById('busyStatus').style.color=d.b?'#f59e0b':'#22c55e';
})}
function mark(){fetch('/m')}
function restartLaser(){fetch('/r')}
setInterval(update,800)
update()
</script>
</body></html>
)rawliteral";
}

void handleData() {
  // Kompaktes JSON
  String json = "{\"d\":" + String(calibrated_mm());
  json += ",\"h\":" + String(communicationHealthy);
  json += ",\"b\":" + String(markBusy) + "}";
  server.send(200, "application/json", json);
}

// ===== Setup
void setup() {
  Serial.begin(115200);
  
  // Pins
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_BLINK, OUTPUT);
  pinMode(PIN_TRIG_OUT, OUTPUT);
  pinMode(PIN_TRIG_IN, INPUT_PULLDOWN);
  
  digitalWrite(PIN_TRIG_OUT, HIGH);
  laserOn();
  
  // Laser
  LZR.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(400);
  sendCmd(CMD_LASER_ON, sizeof(CMD_LASER_ON));
  
  // Servo
  markerServo.attach(PIN_SERVO);
  setServoAngle(HOME_ANGLE);
  
  // WiFi
  WiFi.softAP(AP_SSID, AP_PASS);
  server.begin();
  
  // Server - Kürzere URLs
  server.on("/", []() { server.send(200, "text/html", html()); });
  server.on("/d", handleData);
  server.on("/m", []() { doMarkCycle(); server.send(200, "text/plain", "OK"); });
  server.on("/r", []() { restartLaser(); server.send(200, "text/plain", "OK"); });
  
  beep(1);
}

// ===== Loop - OPTIMIZED
void loop() {
  static unsigned long lastMeasure = 0;
  static bool lastTrigger = false;
  
  // Messung mit reduzierter Frequenz
  if(millis() - lastMeasure > 1200) { // REDUCED FREQUENCY
    measureDistance();
    lastMeasure = millis();
  }
  
  // Keep-Alive seltener
  if(millis() - keepAliveT > 5000) { // REDUCED FREQUENCY
    sendCmd(CMD_LASER_ON, sizeof(CMD_LASER_ON));
    keepAliveT = millis();
  }
  
  // LED
  if(millis() - blinkT > (communicationHealthy ? 800 : 200)) {
    digitalWrite(PIN_BLINK, !digitalRead(PIN_BLINK));
    blinkT = millis();
  }
  
  // Trigger
  bool trigger = digitalRead(PIN_TRIG_IN);
  if(trigger && !lastTrigger) {
    delay(15); // REDUCED
    if(digitalRead(PIN_TRIG_IN)) {
      doMarkCycle();
    }
  }
  lastTrigger = trigger;
  
  server.handleClient();
  delay(2); // REDUCED
}
