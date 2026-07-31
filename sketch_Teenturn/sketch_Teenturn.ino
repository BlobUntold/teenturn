// ============================================================================
// TEENTURN AIR-QUALITY WEB PROJECT
// Board: Arduino Nano 33 IoT
//
// This file is organized in two levels:
//   1. STUDENT SETTINGS and WEBSITE sections are safe places to edit.
//   2. ADVANCED CODE runs the sensor, OLED, LEDs, and web server.
//
// Hardware used:
//   SCD41 air-quality sensor (I2C)
//   SSD1306 128x64 OLED display (I2C address 0x3C)
//   Green LED on D4, red LED on D5, orange LED on D6
//   Button 1 on D2 and Button 2 on D3 (wired to GND when pressed)
// ============================================================================

#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2cScd4x.h>
#include <RTCZero.h>

// ============================================================================
// STUDENT SETTINGS: These values are designed to be easy to change.
// ============================================================================

// Wi-Fi network used by the Arduino and student computer.
char ssid[] = "Shibby";
char pass[] = "12345678";

// Air-quality levels. The current project keeps the requested 800/1000 limits.
const uint16_t CO2_GOOD_MAX = 799;
const uint16_t CO2_MODERATE_MAX = 999;

// How often the sensor is checked. The SCD41 normally provides data every 5 sec.
const unsigned long MEASUREMENT_INTERVAL_MS = 5000;

// Choose the OLED screen shown when the Arduino starts: 0, 1, 2, or 3.
// Screen 3 is the easy-to-replace student screen.
const byte STARTING_SCREEN = 0;

// Editable words shown on the OLED.
const char PROJECT_NAME[] = "AIR MONITOR";
const char CO2_LABEL[] = "CO2";
const char TEMPERATURE_LABEL[] = "Temp";
const char HUMIDITY_LABEL[] = "Humidity";

// Keep true to show the clock/date header on the overview screen.
const bool SHOW_CLOCK_HEADER = true;

// The existing sensor project intentionally displayed half the raw CO2 reading.
// Leave this true to preserve that behavior. Change to false for raw SCD41 ppm.
const bool DIVIDE_CO2_BY_TWO = true;

// ============================================================================
// HARDWARE SETTINGS: Already matched to the working sensor project.
// ============================================================================

const int BUTTON_1_PIN = 2;
const int BUTTON_2_PIN = 3;
const int GREEN_LED_PIN = 4;
const int RED_LED_PIN = 5;
const int ORANGE_LED_PIN = 6;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// ============================================================================
// WEBSITE TEMPLATE
//
// Students paste card blocks between "PASTE CARDS HERE" markers.
// Card snippets in the cards-paste folder already use IDs understood by the
// JavaScript below. The page makes no sensor requests for cards that are absent.
// ============================================================================

const char webpage[] = R"TEENTURN_HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>My Air Quality Project</title>
  <style>
    /* STUDENT COLORS: Change color values, but keep the --names. */
    :root {
      --bg: #0b1020;
      --dark-card: #1e3a5f;
      --light-card: #ffffff;
      --blue-card: #2563eb;
      --warning-card: #f59e0b;
      --green-card: #10b981;
      --pink-card: #ec4899;
      --purple-card: #7c3aed;
      --button: #7b2cff;
      --button-hover: #9955ff;
      --light-text: #ffffff;
      --dark-text: #111111;
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      padding: 20px;
      font-family: Arial, sans-serif;
      color: var(--light-text);
      background: var(--bg);
    }

    .container { width: 100%; max-width: 1200px; margin: 0 auto; }
    h1, .subtitle { text-align: center; }
    h1 { margin-bottom: 8px; }
    .subtitle { margin-top: 0; margin-bottom: 28px; color: #cbd5e1; }

    .dashboard {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 20px;
    }

    .span-2 { grid-column: 1 / -1; }

    .card {
      min-height: 170px;
      padding: 20px;
      border-radius: 16px;
      box-shadow: 0 8px 22px rgba(0, 0, 0, 0.2);
    }

    .card h2 { margin-top: 0; }
    .dark-card { color: var(--light-text); background: var(--dark-card); }
    .light-card { color: var(--dark-text); background: var(--light-card); }
    .blue-card { color: var(--light-text); background: var(--blue-card); }
    .warning-card { color: var(--dark-text); background: var(--warning-card); }
    .green-card { color: var(--light-text); background: var(--green-card); }
    .pink-card { color: var(--light-text); background: var(--pink-card); }
    .purple-card { color: var(--light-text); background: var(--purple-card); }
    .outline-card { color: var(--light-text); background: transparent; border: 2px solid white; }

    .status-box {
      margin-top: 12px;
      padding: 12px;
      color: var(--dark-text);
      background: rgba(255, 255, 255, 0.94);
      border-radius: 10px;
      font-weight: bold;
    }

    button {
      margin-top: 10px;
      padding: 12px 18px;
      color: white;
      background: var(--button);
      border: 0;
      border-radius: 10px;
      font-size: 1rem;
      cursor: pointer;
    }

    button:hover { background: var(--button-hover); }
    button:disabled { opacity: 0.65; cursor: wait; }

    input[type="text"] {
      width: 100%;
      margin-top: 10px;
      padding: 12px;
      border: 1px solid #cbd5e1;
      border-radius: 10px;
      font-size: 1rem;
    }

    img { display: block; width: 100%; margin-top: 10px; border-radius: 12px; }

    @media (max-width: 800px) {
      .dashboard { grid-template-columns: 1fr; }
      .span-2 { grid-column: auto; }
    }
  </style>
</head>
<body>
  <main class="container">
    <!-- STUDENTS: Change the title and description. -->
    <h1>My Air Quality Project</h1>
    <p class="subtitle">Live information from our Arduino Nano 33 IoT.</p>

    <div class="dashboard">
      <!-- ==================== PASTE CARDS HERE ==================== -->

      <!-- ================= END OF CARD AREA ======================= -->
    </div>
  </main>

  <script>
    // This support code recognizes IDs used by cards in the cards-paste folder.
    const POLL_INTERVAL_MS = 2000;
    const REQUEST_TIMEOUT_MS = 1800;
    let pollInProgress = false;

    function exists(id) {
      return document.getElementById(id) !== null;
    }

    function setText(id, text) {
      const element = document.getElementById(id);
      if (element) element.textContent = text;
    }

    async function getArduinoText(path) {
      const separator = path.includes('?') ? '&' : '?';
      const controller = new AbortController();
      const timer = setTimeout(function () { controller.abort(); }, REQUEST_TIMEOUT_MS);

      try {
        const response = await fetch(path + separator + 'time=' + Date.now(), {
          method: 'GET',
          cache: 'no-store',
          signal: controller.signal
        });
        if (!response.ok) throw new Error('HTTP ' + response.status);
        return (await response.text()).trim();
      } finally {
        clearTimeout(timer);
      }
    }

    async function updateCard(id, route, label) {
      if (!exists(id)) return;
      try {
        const value = await getArduinoText(route);
        setText(id, label + value);
      } catch (error) {
        setText(id, label + 'unavailable');
      }
    }

    // Example control: change to the next OLED screen.
    async function runControl1() {
      const button = document.getElementById('control1Button');
      if (button) button.disabled = true;
      setText('control1Status', 'Screen: changing...');

      try {
        const result = await getArduinoText('/control1');
        setText('control1Status', 'Screen: ' + result);
        setText('screenStatus', 'Screen: ' + result);
      } catch (error) {
        setText('control1Status', 'Screen: unavailable');
      } finally {
        if (button) button.disabled = false;
      }
    }

    async function sendTextToArduino() {
      const input = document.getElementById('textToArduino');
      const message = input ? input.value.trim() : '';
      if (!message) {
        setText('textSendStatus', 'Message: type something first');
        return;
      }

      try {
        const result = await getArduinoText('/sendText?message=' + encodeURIComponent(message));
        setText('textSendStatus', 'Message: ' + result);
        input.value = '';
      } catch (error) {
        setText('textSendStatus', 'Message: unavailable');
      }
    }

    // Requests are sequential so the small Arduino server is not overwhelmed.
    async function updateLiveCards() {
      if (pollInProgress || document.hidden) return;
      pollInProgress = true;

      try {
        await updateCard('co2Value', '/co2', 'CO2: ');
        await updateCard('temperatureValue', '/temperature', 'Temperature: ');
        await updateCard('humidityValue', '/humidity', 'Humidity: ');
        await updateCard('airQualityValue', '/airQuality', 'Air quality: ');
        await updateCard('button1Value', '/sensor1', 'Button 1: ');
        await updateCard('button2Value', '/sensor2', 'Button 2: ');
        await updateCard('screenStatus', '/status1', 'Screen: ');
        await updateCard('warning1', '/warning1', 'Warning: ');
      } finally {
        pollInProgress = false;
      }
    }

    const textInput = document.getElementById('textToArduino');
    if (textInput) {
      textInput.addEventListener('keydown', function (event) {
        if (event.key === 'Enter') sendTextToArduino();
      });
    }

    updateLiveCards();
    setInterval(updateLiveCards, POLL_INTERVAL_MS);
  </script>
</body>
</html>
)TEENTURN_HTML";

// ============================================================================
// OBJECTS AND LIVE DATA: Advanced code uses these shared values.
// ============================================================================

WiFiServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SensirionI2cScd4x scd4x;
RTCZero rtc;

float temperature = 0.0;
float humidity = 0.0;
uint16_t co2ppm = 0;
bool dataValid = false;
bool displayReady = false;
bool sensorStarted = false;

unsigned long lastMeasurement = 0;
unsigned long lastBlink = 0;
unsigned long lastDisplayDraw = 0;
bool blinkState = false;

const byte SCREEN_COUNT = 4;
byte currentScreen = STARTING_SCREEN % SCREEN_COUNT;

const char *monthNames[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void initializeHardware();
void connectToWiFi();
void readAirQualitySensor();
void updateAirQualityLeds();
const char *getAirQualityLabel();
void setRtcFromCompileTime();
void showSplashScreen();
void drawCurrentScreen();
void drawOverviewScreen();
void drawLargeCo2Screen();
void drawClimateScreen();
void drawStudentScreen();
void drawClockHeader();
const char *getScreenName();
void changeToNextScreen();
void serviceWebServer();
void handleRequest(WiFiClient &client, const String &request);
bool parseRequestLine(const String &request, String &method, String &target);
String urlDecode(const String &input);
void sendResponse(WiFiClient &client, int statusCode, const char *statusText, const char *contentType, const String &body);
void sendHTMLPage(WiFiClient &client);
void sendNoContent(WiFiClient &client);
void sendCORSPreflight(WiFiClient &client);

// ============================================================================
// SETUP AND MAIN LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);

  // Do not wait forever for Serial; the project must run without a computer.
  unsigned long serialWaitStarted = millis();
  while (!Serial && millis() - serialWaitStarted < 3000) {
  }

  // Connect silently so the website address is the first printed line.
  connectToWiFi();
  server.begin();

  Serial.print("Website IP: http://");
  Serial.println(WiFi.localIP());

  // Hardware startup messages, if any, are printed after the website address.
  initializeHardware();
}

void loop() {
  unsigned long now = millis();

  // Read the sensor without using a long delay, so the website stays responsive.
  if (sensorStarted && (lastMeasurement == 0 || now - lastMeasurement >= MEASUREMENT_INTERVAL_MS)) {
    lastMeasurement = now;
    readAirQualitySensor();
  }

  // Blink the clock separator and high-CO2 red LED every half second.
  if (now - lastBlink >= 500) {
    lastBlink = now;
    blinkState = !blinkState;
  }

  updateAirQualityLeds();

  // Refreshing five times per second looks smooth without slowing the web server.
  if (displayReady && now - lastDisplayDraw >= 200) {
    lastDisplayDraw = now;
    drawCurrentScreen();
  }

  serviceWebServer();
}

// ============================================================================
// HARDWARE STARTUP
// ============================================================================

void initializeHardware() {
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  Wire.begin();
  setRtcFromCompileTime();

  displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (displayReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    showSplashScreen();
  } else {
    Serial.println("OLED not found. Website and sensor will still run.");
  }

  scd4x.begin(Wire, 0x62);
  scd4x.stopPeriodicMeasurement();
  delay(500);

  uint16_t error = scd4x.startPeriodicMeasurement();
  sensorStarted = (error == 0);

  if (sensorStarted) {
    Serial.println("SCD41 sensor started.");
  } else {
    Serial.print("SCD41 start error: ");
    Serial.println(error);
  }
}

void connectToWiFi() {
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
  }
}

// ============================================================================
// SENSOR AND AUTOMATIC LED FUNCTIONS
// ============================================================================

void readAirQualitySensor() {
  bool dataReady = false;
  uint16_t error = scd4x.getDataReadyStatus(dataReady);

  if (error || !dataReady) {
    return;
  }

  uint16_t rawCo2 = 0;
  float newTemperature = 0.0;
  float newHumidity = 0.0;

  error = scd4x.readMeasurement(rawCo2, newTemperature, newHumidity);

  if (!error && rawCo2 != 0) {
    co2ppm = DIVIDE_CO2_BY_TWO ? rawCo2 / 2 : rawCo2;
    temperature = newTemperature;
    humidity = newHumidity;
    dataValid = true;

    Serial.print("CO2:");
    Serial.print(co2ppm);
    Serial.print(",Temp:");
    Serial.print(temperature, 1);
    Serial.print(",Humidity:");
    Serial.println(humidity, 1);
  } else {
    Serial.print("SCD41 read error: ");
    Serial.println(error);
  }
}

const char *getAirQualityLabel() {
  if (!dataValid) return "Waiting for data";
  if (co2ppm <= CO2_GOOD_MAX) return "Good";
  if (co2ppm <= CO2_MODERATE_MAX) return "Moderate";
  return "High CO2";
}

void updateAirQualityLeds() {
  if (!dataValid) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(ORANGE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    return;
  }

  if (co2ppm <= CO2_GOOD_MAX) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(ORANGE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
  } else if (co2ppm <= CO2_MODERATE_MAX) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(ORANGE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(ORANGE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, blinkState ? HIGH : LOW);
  }
}

// ============================================================================
// BEGINNER-FRIENDLY OLED SCREENS
//
// Screen 0: overview, Screen 1: large CO2, Screen 2: temperature/humidity,
// Screen 3: the easy-to-replace student screen.
// Students can edit words in STUDENT SETTINGS without touching drawing code.
// Optional screen snippets can be copied from the oled-screen-paste folder.
// ============================================================================

void showSplashScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(4, 16);
  display.print(PROJECT_NAME);
  display.setTextSize(1);
  display.setCursor(12, 43);
  display.print("Starting sensor...");
  display.display();
  delay(1200);
}

void drawCurrentScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (currentScreen == 0) {
    drawOverviewScreen();
  } else if (currentScreen == 1) {
    drawLargeCo2Screen();
  } else if (currentScreen == 2) {
    drawClimateScreen();
  } else {
    drawStudentScreen();
  }

  display.display();
}

void drawClockHeader() {
  int month = rtc.getMonth();
  if (month < 1 || month > 12) month = 1;

  char dateBuffer[12];
  char timeBuffer[8];
  char separator = blinkState ? ':' : ' ';

  snprintf(dateBuffer, sizeof(dateBuffer), "%s %02d", monthNames[month - 1], rtc.getDay());
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d%c%02d", rtc.getHours(), separator, rtc.getMinutes());

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(dateBuffer);
  display.setCursor(92, 0);
  display.print(timeBuffer);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
}

void drawOverviewScreen() {
  int firstLine = 0;

  if (SHOW_CLOCK_HEADER) {
    drawClockHeader();
    firstLine = 16;
  }

  if (!dataValid) {
    display.setTextSize(1);
    display.setCursor(8, 28);
    display.print("Waiting for data...");
    return;
  }

  display.setTextSize(1);
  display.setCursor(0, firstLine);
  display.print(TEMPERATURE_LABEL);
  display.print(": ");
  display.print(temperature, 1);
  display.print(" C");

  display.setCursor(0, firstLine + 12);
  display.print(HUMIDITY_LABEL);
  display.print(": ");
  display.print(humidity, 1);
  display.print(" %");

  display.setCursor(0, firstLine + 24);
  display.print(CO2_LABEL);
  display.print(": ");
  display.print(co2ppm);
  display.print(" ppm");

  display.setCursor(0, firstLine + 36);
  display.print(getAirQualityLabel());
}

void drawLargeCo2Screen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(CO2_LABEL);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (!dataValid) {
    display.setCursor(8, 30);
    display.print("Waiting for data...");
    return;
  }

  display.setTextSize(3);
  display.setCursor(8, 18);
  display.print(co2ppm);

  display.setTextSize(1);
  display.setCursor(94, 34);
  display.print("ppm");
  display.setCursor(0, 54);
  display.print(getAirQualityLabel());
}

void drawClimateScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("CLIMATE");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (!dataValid) {
    display.setCursor(8, 30);
    display.print("Waiting for data...");
    return;
  }

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print(temperature, 1);
  display.print(" C");

  display.setCursor(0, 42);
  display.print(humidity, 1);
  display.print(" %");
}

// ============================================================================
// STUDENT OLED PASTE AREA
//
// To use a design from oled-screen-paste:
//   1. Select this entire drawStudentScreen() function.
//   2. Replace it with the function from the chosen snippet file.
//   3. Upload the sketch and switch to screen 3 from the website.
// ============================================================================
void drawStudentScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("MY CUSTOM SCREEN");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 22);
  display.print("Paste a screen from");
  display.setCursor(0, 34);
  display.print("oled-screen-paste");
  display.setCursor(0, 52);
  display.print("Screen number: 3");
}

const char *getScreenName() {
  if (currentScreen == 0) return "Overview";
  if (currentScreen == 1) return "Large CO2";
  if (currentScreen == 2) return "Climate";
  return "Student Screen";
}

void changeToNextScreen() {
  currentScreen = (currentScreen + 1) % SCREEN_COUNT;
  if (displayReady) drawCurrentScreen();
}

void setRtcFromCompileTime() {
  char monthText[4];
  int day, year, hour, minute, second;

  sscanf(__DATE__, "%3s %d %d", monthText, &day, &year);
  sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

  int month = 1;
  for (int i = 0; i < 12; i++) {
    if (strncmp(monthText, monthNames[i], 3) == 0) {
      month = i + 1;
      break;
    }
  }

  rtc.begin();
  rtc.setTime((uint8_t)hour, (uint8_t)minute, (uint8_t)second);
  rtc.setDate((uint8_t)day, (uint8_t)month, (uint8_t)(year % 100));
}

// ============================================================================
// WEB ROUTES
//
// /co2          -> CO2 value in ppm
// /temperature  -> temperature in C
// /humidity     -> relative humidity percent
// /airQuality   -> Good, Moderate, High CO2, or Waiting for data
// /sensor1      -> physical Button 1 state
// /sensor2      -> physical Button 2 state
// /control1     -> changes to the next OLED screen
// /status1      -> current OLED screen name
// /warning1     -> warning text for a warning card
// /sendText     -> prints a webpage message in Serial Monitor
// ============================================================================

void serviceWebServer() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = "";
  unsigned long connectedAt = millis();
  unsigned long lastByteAt = connectedAt;

  while (client.connected()) {
    while (client.available()) {
      char c = (char)client.read();
      request += c;
      lastByteAt = millis();

      if (request.length() > 2048) {
        sendResponse(client, 413, "Payload Too Large", "text/plain; charset=UTF-8", "Request too large");
        client.flush();
        delay(5);
        client.stop();
        return;
      }

      if (request.endsWith("\r\n\r\n")) {
        handleRequest(client, request);
        client.flush();
        delay(5);
        client.stop();
        return;
      }
    }

    // Browsers sometimes open an unused connection. Close it quickly so sensor
    // and display updates are not paused for several seconds.
    if (request.length() == 0 && millis() - connectedAt > 300) {
      client.stop();
      return;
    }

    if (request.length() > 0 && millis() - lastByteAt > 700) {
      sendResponse(client, 400, "Bad Request", "text/plain; charset=UTF-8", "Incomplete HTTP request");
      client.stop();
      return;
    }
  }

  client.stop();
}

void handleRequest(WiFiClient &client, const String &request) {
  String method = "";
  String target = "";

  if (!parseRequestLine(request, method, target)) {
    sendResponse(client, 400, "Bad Request", "text/plain; charset=UTF-8", "Malformed HTTP request line");
    return;
  }

  Serial.print("WEB: ");
  Serial.print(method);
  Serial.print(" ");
  Serial.println(target);

  if (method == "OPTIONS") {
    sendCORSPreflight(client);
    return;
  }

  if (method != "GET") {
    sendResponse(client, 405, "Method Not Allowed", "text/plain; charset=UTF-8", "Only GET is supported");
    return;
  }

  String path = target;
  String query = "";
  int questionMark = target.indexOf('?');
  if (questionMark >= 0) {
    path = target.substring(0, questionMark);
    query = target.substring(questionMark + 1);
  }

  if (path == "/" || path == "/index.html") {
    sendHTMLPage(client);
    return;
  }

  if (path == "/favicon.ico") {
    sendNoContent(client);
    return;
  }

  if (path == "/co2") {
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", dataValid ? String(co2ppm) + " ppm" : "waiting");
    return;
  }

  if (path == "/temperature") {
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", dataValid ? String(temperature, 1) + " C" : "waiting");
    return;
  }

  if (path == "/humidity") {
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", dataValid ? String(humidity, 1) + " %" : "waiting");
    return;
  }

  if (path == "/airQuality") {
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", getAirQualityLabel());
    return;
  }

  if (path == "/sensor1") {
    bool pressed = digitalRead(BUTTON_1_PIN) == LOW;
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", pressed ? "PRESSED" : "NOT PRESSED");
    return;
  }

  if (path == "/sensor2") {
    bool pressed = digitalRead(BUTTON_2_PIN) == LOW;
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", pressed ? "PRESSED" : "NOT PRESSED");
    return;
  }

  if (path == "/control1") {
    changeToNextScreen();
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", getScreenName());
    return;
  }

  if (path == "/status1") {
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", getScreenName());
    return;
  }

  if (path == "/warning1") {
    const char *warning = (!dataValid || co2ppm <= CO2_MODERATE_MAX) ? "none" : "HIGH CO2 - improve ventilation";
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", warning);
    return;
  }

  if (path == "/sendText") {
    String message = "";
    int messageStart = query.indexOf("message=");
    if (messageStart >= 0) {
      message = query.substring(messageStart + 8);
      int ampersand = message.indexOf('&');
      if (ampersand >= 0) message = message.substring(0, ampersand);
      message = urlDecode(message);
    }

    Serial.print("WEB MESSAGE: ");
    Serial.println(message);
    sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", "received");
    return;
  }

  sendResponse(client, 404, "Not Found", "text/plain; charset=UTF-8", "Route not found");
}

// ============================================================================
// ADVANCED HTTP HELPERS: Students normally do not edit below this line.
// ============================================================================

bool parseRequestLine(const String &request, String &method, String &target) {
  int lineEnd = request.indexOf("\r\n");
  if (lineEnd < 0) lineEnd = request.indexOf('\n');
  if (lineEnd <= 0) return false;

  String line = request.substring(0, lineEnd);
  line.trim();

  int firstSpace = line.indexOf(' ');
  int secondSpace = line.indexOf(' ', firstSpace + 1);
  if (firstSpace <= 0 || secondSpace <= firstSpace + 1) return false;

  String httpVersion = line.substring(secondSpace + 1);
  if (!httpVersion.startsWith("HTTP/")) return false;

  method = line.substring(0, firstSpace);
  target = line.substring(firstSpace + 1, secondSpace);

  // Also accept the absolute URL form used by some proxies.
  if (target.startsWith("http://") || target.startsWith("https://")) {
    int slash = target.indexOf('/', target.indexOf("//") + 2);
    target = (slash >= 0) ? target.substring(slash) : "/";
  }

  return target.length() > 0;
}

String urlDecode(const String &input) {
  String output = "";

  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);

    if (c == '+') {
      output += ' ';
    } else if (c == '%' && i + 2 < input.length()) {
      char highCharacter = input.charAt(i + 1);
      char lowCharacter = input.charAt(i + 2);

      int highValue = (highCharacter >= '0' && highCharacter <= '9') ? highCharacter - '0' :
                      (highCharacter >= 'A' && highCharacter <= 'F') ? highCharacter - 'A' + 10 :
                      (highCharacter >= 'a' && highCharacter <= 'f') ? highCharacter - 'a' + 10 : 0;
      int lowValue = (lowCharacter >= '0' && lowCharacter <= '9') ? lowCharacter - '0' :
                     (lowCharacter >= 'A' && lowCharacter <= 'F') ? lowCharacter - 'A' + 10 :
                     (lowCharacter >= 'a' && lowCharacter <= 'f') ? lowCharacter - 'a' + 10 : 0;

      output += (char)((highValue << 4) | lowValue);
      i += 2;
    } else {
      output += c;
    }
  }

  return output;
}

void sendResponse(WiFiClient &client, int statusCode, const char *statusText, const char *contentType, const String &body) {
  client.print("HTTP/1.1 ");
  client.print(statusCode);
  client.print(" ");
  client.print(statusText);
  client.print("\r\nContent-Type: ");
  client.print(contentType);
  client.print("\r\nCache-Control: no-store, no-cache, must-revalidate");
  client.print("\r\nAccess-Control-Allow-Origin: *");
  client.print("\r\nConnection: close");
  client.print("\r\nContent-Length: ");
  client.print(body.length());
  client.print("\r\n\r\n");
  client.print(body);
}

void sendHTMLPage(WiFiClient &client) {
  const size_t totalLength = strlen(webpage);

  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: text/html; charset=UTF-8\r\n");
  client.print("Cache-Control: no-store, no-cache, must-revalidate\r\n");
  client.print("Access-Control-Allow-Origin: *\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: ");
  client.print(totalLength);
  client.print("\r\n\r\n");

  // Chunked writes prevent large embedded pages from being cut off early.
  size_t sent = 0;
  unsigned long timeout = millis();

  while (sent < totalLength && millis() - timeout < 6000) {
    size_t remaining = totalLength - sent;
    size_t chunkSize = remaining > 256 ? 256 : remaining;
    size_t written = client.write((const uint8_t *)webpage + sent, chunkSize);

    if (written > 0) {
      sent += written;
      timeout = millis();
    } else {
      delay(2);
    }
  }

  Serial.print("HTML bytes sent: ");
  Serial.print(sent);
  Serial.print("/");
  Serial.println(totalLength);
}

void sendNoContent(WiFiClient &client) {
  client.print("HTTP/1.1 204 No Content\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: 0\r\n\r\n");
}

void sendCORSPreflight(WiFiClient &client) {
  client.print("HTTP/1.1 204 No Content\r\n");
  client.print("Access-Control-Allow-Origin: *\r\n");
  client.print("Access-Control-Allow-Methods: GET, OPTIONS\r\n");
  client.print("Access-Control-Allow-Headers: Content-Type\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: 0\r\n\r\n");
}
