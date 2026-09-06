#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

enum FlowState {
  FLOW_HALT,
  SAMPLING,
  LOW_FLOW,
  HIGH_FLOW
};

// -------------------- PIN CONFIG --------------------
constexpr int IR_SENSOR_PIN = 32;
constexpr int SERVO_PIN = 25;
constexpr int OLED_SDA_PIN = 21;
constexpr int OLED_SCL_PIN = 22;

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// -------------------- TIMING --------------------
constexpr unsigned long DEBOUNCE_MS = 80;
constexpr unsigned long SAMPLE_WINDOW_MS = 5000;

// Startup gimmick
constexpr unsigned long STARTUP_MEASURING_MS = 3000;

// Flow threshold
constexpr float LOW_FLOW_MAX_DROPS_PER_SECOND = 1.0F;

// -------------------- SERVO ANGLES --------------------
constexpr int SERVO_NEUTRAL_ANGLE = 90;
constexpr int SERVO_LOW_FLOW_ANGLE = 110;
constexpr int SERVO_HIGH_FLOW_ANGLE = 70;

// -------------------- WIFI --------------------
const char *WIFI_AP_NAME = "IV-Drop-Demo";
const char *WIFI_AP_PASSWORD = "dropdemo";

// -------------------- OBJECTS --------------------
Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

Servo pointerServo;
WebServer server(80);

// -------------------- VARIABLES --------------------
unsigned long totalDrops = 0;
unsigned long windowDrops = 0;

unsigned long sampleStartMs = 0;
unsigned long lastDropMs = 0;
unsigned long lastSensorChangeMs = 0;
unsigned long lastScreenRefreshMs = 0;

float dropsPerSecond = 0.0F;

bool sensorActive = false;
bool oledReady = false;

// Startup mode
bool startupMeasuring = true;
unsigned long startupStartMs = 0;

FlowState flowState = FLOW_HALT;

// ====================================================
// FORWARD DECLARATIONS
// ====================================================

void setPointerForState();
void drawScreen(unsigned long now);
String sampleWindowText(unsigned long now);
void handleStatus();
void startWebDashboard();
void startSampleWindow(unsigned long now);
void finishSampleWindow();
void countDrop(unsigned long now);

// ====================================================
// FLOW LABEL
// ====================================================

const char *flowLabel() {

  if (startupMeasuring) {
    return "MEASURING";
  }

  switch (flowState) {

    case FLOW_HALT:
      return "FLOW HALT";

    case LOW_FLOW:
      return "LOW FLOW";

    case HIGH_FLOW:
      return "HIGH FLOW";

    case SAMPLING:
      return "MEASURING";

    default:
      return "MEASURING";
  }
}

// ====================================================
// SERVO
// ====================================================

void setPointerForState() {

  // During startup keep pointer in neutral position
  if (startupMeasuring) {
    pointerServo.write(SERVO_NEUTRAL_ANGLE);
    return;
  }

  if (flowState == LOW_FLOW) {

    pointerServo.write(SERVO_LOW_FLOW_ANGLE);

  } else if (flowState == HIGH_FLOW) {

    pointerServo.write(SERVO_HIGH_FLOW_ANGLE);

  } else {

    pointerServo.write(SERVO_NEUTRAL_ANGLE);
  }
}

// ====================================================
// OLED DISPLAY
// ====================================================

void drawScreen(unsigned long now) {

  if (!oledReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BENCH IV DROP DEMO");

  display.setCursor(0, 10);
  display.print("DEMONSTRATOR ONLY");

  display.drawLine(
    0,
    20,
    127,
    20,
    SSD1306_WHITE
  );

  // ---------------- STARTUP SCREEN ----------------

  if (startupMeasuring) {

    display.setTextSize(2);
    display.setCursor(5, 27);
    display.print("MEASURING");

    // Animated dots
    int dots = (now / 400) % 4;

    display.setCursor(105, 27);

    for (int i = 0; i < dots; i++) {
      display.print(".");
    }

    display.setTextSize(1);
    display.setCursor(15, 47);

    unsigned long elapsed = now - startupStartMs;

    int progress =
      map(
        min(elapsed, STARTUP_MEASURING_MS),
        0,
        STARTUP_MEASURING_MS,
        0,
        100
      );

    display.print("System check ");
    display.print(progress);
    display.print("%");

    display.drawRect(
      15,
      56,
      98,
      6,
      SSD1306_WHITE
    );

    int barWidth =
      map(
        min(elapsed, STARTUP_MEASURING_MS),
        0,
        STARTUP_MEASURING_MS,
        0,
        94
      );

    display.fillRect(
      17,
      58,
      barWidth,
      2,
      SSD1306_WHITE
    );

    display.display();

    return;
  }

  // ---------------- NORMAL SCREEN ----------------

  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print(flowLabel());

  display.setTextSize(1);
  display.setCursor(0, 45);

  if (flowState == SAMPLING) {

    unsigned long elapsedMs =
      now - sampleStartMs;

    unsigned long remainingSeconds =
      (elapsedMs >= SAMPLE_WINDOW_MS)
        ? 0
        : (SAMPLE_WINDOW_MS - elapsedMs + 999) / 1000;

    display.print("Check: ");
    display.print(remainingSeconds);
    display.print("s  Drops: ");
    display.print(windowDrops);

  } else if (flowState == FLOW_HALT) {

    display.print("Waiting for a drop");

  } else {

    display.print("Rate: ");
    display.print(dropsPerSecond, 2);
    display.print(" drop/s");
  }

  display.setCursor(0, 55);

  display.print("Total drops: ");
  display.print(totalDrops);

  display.display();
}

// ====================================================
// WEB WINDOW TEXT
// ====================================================

String sampleWindowText(unsigned long now) {

  if (startupMeasuring) {

    unsigned long elapsed =
      now - startupStartMs;

    unsigned long remaining =
      (elapsed >= STARTUP_MEASURING_MS)
        ? 0
        : (STARTUP_MEASURING_MS - elapsed + 999) / 1000;

    return String("System measuring... ") +
           remaining +
           " s";
  }

  if (flowState != SAMPLING) {

    if (flowState == FLOW_HALT) {
      return "Waiting for a drop";
    }

    return "Last five-second result";
  }

  unsigned long elapsedMs =
    now - sampleStartMs;

  unsigned long remainingSeconds =
    (elapsedMs >= SAMPLE_WINDOW_MS)
      ? 0
      : (SAMPLE_WINDOW_MS - elapsedMs + 999) / 1000;

  return String("Checking: ") +
         remainingSeconds +
         " s left, " +
         windowDrops +
         " drops";
}

// ====================================================
// WEB STATUS
// ====================================================

void handleStatus() {

  const unsigned long now = millis();

  String json =
    "{\"state\":\"" +
    String(flowLabel()) +
    "\",\"total\":" +
    String(totalDrops) +
    ",\"rate\":" +
    String(dropsPerSecond, 2) +
    ",\"sensor\":" +
    String(sensorActive ? "true" : "false") +
    ",\"window\":\"" +
    sampleWindowText(now) +
    "\"}";

  server.send(
    200,
    "application/json",
    json
  );
}

// ====================================================
// WEB DASHBOARD
// ====================================================

const char DASHBOARD_HTML[] PROGMEM = R"HTML(
<!doctype html>

<html>

<head>

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>Bench Drop Demo</title>

<style>

body{
margin:0;
background:#071426;
color:#e9f4ff;
font-family:Arial,sans-serif;
text-align:center;
}

main{
max-width:420px;
margin:auto;
padding:22px 16px;
}

.notice{
font-size:.8rem;
color:#ffbf69;
}

.card{
background:#0d2742;
border:1px solid #1d5c8f;
border-radius:14px;
margin:14px 0;
padding:20px;
}

.state{
font-size:2rem;
font-weight:bold;
color:#2fb7ff;
}

.value{
font-size:2.2rem;
font-weight:bold;
margin:8px;
}

.label{
color:#9eb8d0;
font-size:.85rem;
text-transform:uppercase;
letter-spacing:.08em;
}

.row{
display:flex;
gap:12px;
}

.row .card{
flex:1;
margin:0;
}

small{
color:#9eb8d0;
}

.bar{
height:8px;
background:#163b5b;
border-radius:10px;
overflow:hidden;
margin-top:15px;
}

.progress{
height:100%;
width:0%;
background:#2fb7ff;
transition:width .3s;
}

</style>

</head>

<body>

<main>

<h2>Bench Drop Monitor</h2>

<p class="notice">
DEMONSTRATOR ONLY — NOT FOR PATIENT USE
</p>

<div class="card">

<div class="label">
Current flow state
</div>

<div id="state"
class="state">
MEASURING
</div>

<div id="detail">
Initializing system...
</div>

<div class="bar">
<div id="progress"
class="progress">
</div>
</div>

</div>

<div class="row">

<div class="card">

<div class="label">
Total drops
</div>

<div id="total"
class="value">
0
</div>

</div>

<div class="card">

<div class="label">
Rate
</div>

<div id="rate"
class="value">
0.00
</div>

<small>
drops / second
</small>

</div>

</div>

<div class="card">

<div class="label">
Five-second measurement
</div>

<div id="window">
System measuring...
</div>

</div>

<p>
<small>
Updates once per second · Local Wi-Fi only
</small>
</p>

</main>

<script>

function refresh(){

fetch('/api/status')

.then(r => r.json())

.then(d => {

document.getElementById('state')
.textContent = d.state;

document.getElementById('total')
.textContent = d.total;

document.getElementById('rate')
.textContent = d.rate.toFixed(2);

document.getElementById('window')
.textContent = d.window;

if(d.state === "MEASURING"){

document.getElementById('detail')
.textContent =
"Checking sensor and flow...";

let now =
new Date().getTime();

let progress =
(now % 3000) / 3000 * 100;

document.getElementById('progress')
.style.width =
progress + "%";

}else{

document.getElementById('detail')
.textContent =
d.sensor
? "Sensor sees a drop"
: "Sensor ready";

document.getElementById('progress')
.style.width = "100%";
}

})

.catch(()=>{

document.getElementById('state')
.textContent =
"CONNECTION LOST";

});

}

refresh();

setInterval(refresh,1000);

</script>

</body>

</html>
)HTML";

// ====================================================
// START WEB SERVER
// ====================================================

void startWebDashboard() {

  WiFi.mode(WIFI_AP);

  const bool wifiStarted =
    WiFi.softAP(
      WIFI_AP_NAME,
      WIFI_AP_PASSWORD
    );

  Serial.println(
    wifiStarted
      ? "Wi-Fi dashboard started"
      : "Wi-Fi dashboard failed to start"
  );

  Serial.print("Connect to Wi-Fi: ");
  Serial.println(WIFI_AP_NAME);

  Serial.print("Dashboard address: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {

    server.send_P(
      200,
      "text/html",
      DASHBOARD_HTML
    );

  });

  server.on(
    "/api/status",
    HTTP_GET,
    handleStatus
  );

  server.onNotFound([]() {

    server.send(
      404,
      "text/plain",
      "Not found"
    );

  });

  server.begin();
}

// ====================================================
// START MEASUREMENT
// ====================================================

void startSampleWindow(unsigned long now) {

  flowState = SAMPLING;

  windowDrops = 1;

  sampleStartMs = now;

  lastDropMs = now;

  setPointerForState();

  Serial.println(
    "Drop after halt: starting 5 s measurement"
  );
}

// ====================================================
// FINISH MEASUREMENT
// ====================================================

void finishSampleWindow() {

  dropsPerSecond =
    static_cast<float>(windowDrops) / 5.0F;

  flowState =
    (dropsPerSecond <=
     LOW_FLOW_MAX_DROPS_PER_SECOND)
      ? LOW_FLOW
      : HIGH_FLOW;

  setPointerForState();

  Serial.print("5 s rate: ");

  Serial.print(
    dropsPerSecond,
    2
  );

  Serial.print(" drop/s - ");

  Serial.println(
    flowLabel()
  );
}

// ====================================================
// COUNT DROP
// ====================================================

void countDrop(unsigned long now) {

  ++totalDrops;

  if (flowState == FLOW_HALT) {

    startSampleWindow(now);

  } else if (flowState == SAMPLING) {

    ++windowDrops;

    lastDropMs = now;

  } else {

    startSampleWindow(now);
  }

  Serial.print(
    "Drop counted: "
  );

  Serial.println(
    totalDrops
  );
}

// ====================================================
// SETUP
// ====================================================

void setup() {

  Serial.begin(115200);

  delay(100);

  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "BENCH IV DROP DEMONSTRATOR"
  );

  Serial.println(
    "================================"
  );

  // Sensor
  pinMode(
    IR_SENSOR_PIN,
    INPUT_PULLUP
  );

  // OLED
  const bool oledPinsAssigned =
    Wire.setPins(
      OLED_SDA_PIN,
      OLED_SCL_PIN
    );

  oledReady =
    oledPinsAssigned &&
    display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS,
      true,
      true
    );

  Serial.println(
    oledReady
      ? "OLED ready"
      : "OLED not found"
  );

  // Servo
  pointerServo.attach(
    SERVO_PIN,
    500,
    2400
  );

  pointerServo.write(
    SERVO_NEUTRAL_ANGLE
  );

  Serial.println(
    "Servo attached"
  );

  // Sensor state
  sensorActive =
    (digitalRead(IR_SENSOR_PIN) == LOW);

  lastSensorChangeMs =
    millis();

  lastDropMs =
    lastSensorChangeMs;

  // -------------------------------
  // STARTUP MEASURING MODE
  // -------------------------------

  startupMeasuring = true;

  startupStartMs =
    millis();

  // Start Wi-Fi
  startWebDashboard();

  // Immediately show MEASURING
  drawScreen(
    startupStartMs
  );
}

// ====================================================
// LOOP
// ====================================================

void loop() {

  const unsigned long now =
    millis();

  server.handleClient();

  bool screenChanged = false;

  // ==================================================
  // STARTUP GIMMICK
  // ==================================================

  if (startupMeasuring) {

    // Refresh startup animation
    if (now - lastScreenRefreshMs >= 200) {

      lastScreenRefreshMs = now;

      drawScreen(now);
    }

    // Finish startup after 3 seconds
    if (now - startupStartMs >=
        STARTUP_MEASURING_MS) {

      startupMeasuring = false;

      flowState = FLOW_HALT;

      dropsPerSecond = 0.0F;

      setPointerForState();

      Serial.println();
      Serial.println(
        "Startup measurement complete"
      );

      Serial.println(
        "Waiting for drop..."
      );

      drawScreen(now);
    }

    // Ignore drop counting during startup
    return;
  }

  // ==================================================
  // SENSOR
  // ==================================================

  const bool activeNow =
    (digitalRead(IR_SENSOR_PIN) == LOW);

  if (
    activeNow != sensorActive &&
    now - lastSensorChangeMs >= DEBOUNCE_MS
  ) {

    sensorActive = activeNow;

    lastSensorChangeMs = now;

    if (sensorActive) {

      countDrop(now);

      screenChanged = true;
    }
  }

  // ==================================================
  // FINISH 5 SECOND SAMPLE
  // ==================================================

  if (
    flowState == SAMPLING &&
    now - sampleStartMs >= SAMPLE_WINDOW_MS
  ) {

    finishSampleWindow();

    screenChanged = true;
  }

  // ==================================================
  // NO DROP FOR 5 SECONDS
  // ==================================================

  if (
    (flowState == LOW_FLOW ||
     flowState == HIGH_FLOW) &&

    now - lastDropMs >= SAMPLE_WINDOW_MS
  ) {

    flowState = FLOW_HALT;

    dropsPerSecond = 0.0F;

    setPointerForState();

    Serial.println(
      "No drop for 5 s - FLOW HALT"
    );

    screenChanged = true;
  }

  // ==================================================
  // DISPLAY COUNTDOWN
  // ==================================================

  if (
    flowState == SAMPLING &&
    now - lastScreenRefreshMs >= 1000
  ) {

    lastScreenRefreshMs = now;

    screenChanged = true;
  }

  // ==================================================
  // DRAW
  // ==================================================

  if (screenChanged) {

    drawScreen(now);
  }
}
