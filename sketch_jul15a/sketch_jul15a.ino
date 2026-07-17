#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Shibby";
char pass[] = "12345678";

WiFiServer server(80);

// --------------------------------------------------
// Pin setup
// Assumed meaning from your message:
// LED = D9
// Button 1 = D2
// Button 2 = D3
// --------------------------------------------------
const int ledPin = 12;
const int button1Pin = 5;
const int button2Pin = 6;

bool ledState = false;

// --------------------------------------------------
// Small hosted webpage
// --------------------------------------------------
const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
 <meta charset="UTF-8">
 <meta name="viewport" content="width=device-width, initial-scale=1.0">
 <title>Arduino Dashboard</title>
 <style>
 body {
 margin: 0;
 font-family: Arial, sans-serif;
 background: #0b1020;
 color: white;
 padding: 20px;
 }

.container {
 max-width: 1100px;
 margin: 0 auto;
 }

.dashboard {
 display: grid;
 grid-template-columns: 1fr 1fr;
 gap: 20px;
 }

.span-2 {
 grid-column: 1 / 3;
 }

.card {
 padding: 20px;
 border-radius: 16px;
 min-height: 160px;
 }

.dark-card {
 background: #1e3a5f;
 color: white;
 }

.light-card {
 background: white;
 color: black;
 }

.blue-card {
 background: #2563eb;
 color: white;
 }

.warning-card {
 background: #f59e0b;
 color: black;
 }

.status-box {
 margin-top: 12px;
 padding: 12px;
 border-radius: 10px;
 background: rgba(255,255,255,0.92);
 color: #111;
 font-weight: bold;
 }

 button {
 background: #7b2cff;
 color: white;
 border: none;
 border-radius: 10px;
 padding: 12px 18px;
 cursor: pointer;
 margin-top: 10px;
 }

 input[type="text"] {
 width: 100%;
 padding: 12px;
 border-radius: 10px;
 border: 1px solid #ccc;
 margin-top: 10px;
 box-sizing: border-box;
 }

 @media (max-width: 800px) {
.dashboard {
 grid-template-columns: 1fr;
 }

.span-2 {
 grid-column: auto;
 }
 }
 </style>
</head>
<body>
 <div class="container">
 <div class="dashboard">

 <div class="card blue-card span-2">
 <h2>Arduino Dashboard</h2>
 <p>This page controls the LED and shows button states.</p>
 </div>

 <div class="card dark-card">
 <h2>Control 1</h2>
 <p>Toggle the LED on D9.</p>
 <button onclick="runControl1()">Toggle LED</button>
 <div class="status-box" id="control1Status">Control 1: waiting...</div>
 </div>

 <div class="card light-card">
 <h2>Sensor 1</h2>
 <p>Button 1 on D2.</p>
 <div class="status-box" id="sensor1">Sensor 1: waiting...</div>
 </div>

 <div class="card light-card">
 <h2>Sensor 2</h2>
 <p>Button 2 on D3.</p>
 <div class="status-box" id="sensor2">Sensor 2: waiting...</div>
 </div>

 <div class="card blue-card">
 <h2>Status 1</h2>
 <p>Shows LED state.</p>
 <div class="status-box" id="status1">Status 1: waiting...</div>
 </div>

 <div class="card warning-card">
 <h2>Warning</h2>
 <p>Shows a warning if both buttons are pressed.</p>
 <div class="status-box" id="warning1">Warning: none</div>
 </div>

 <div class="card light-card span-2">
 <h2>Send Text</h2>
 <p>Send a message to the Arduino serial monitor.</p>
 <input type="text" id="textToArduino" placeholder="Type text here">
 <button onclick="sendTextToArduino()">Send Text</button>
 <div class="status-box" id="textSendStatus">Message: waiting...</div>
 </div>

 </div>
 </div>

 <script>
 function runControl1() {
 fetch('/control1')
.then(response => response.text())
.then(data => {
 document.getElementById('control1Status').textContent = 'Control 1: ' + data;
 })
.catch(error => {
 document.getElementById('control1Status').textContent = 'Control 1: error';
 });
 }

 function updateSensor1() {
 fetch('/sensor1')
.then(response => response.text())
.then(data => {
 document.getElementById('sensor1').textContent = 'Sensor 1: ' + data;
 })
.catch(error => {
 document.getElementById('sensor1').textContent = 'Sensor 1: unavailable';
 });
 }

 function updateSensor2() {
 fetch('/sensor2')
.then(response => response.text())
.then(data => {
 document.getElementById('sensor2').textContent = 'Sensor 2: ' + data;
 })
.catch(error => {
 document.getElementById('sensor2').textContent = 'Sensor 2: unavailable';
 });
 }

 function updateStatus1() {
 fetch('/status1')
.then(response => response.text())
.then(data => {
 document.getElementById('status1').textContent = 'Status 1: ' + data;
 })
.catch(error => {
 document.getElementById('status1').textContent = 'Status 1: unavailable';
 });
 }

 function updateWarning1() {
 fetch('/warning1')
.then(response => response.text())
.then(data => {
 document.getElementById('warning1').textContent = 'Warning: ' + data;
 })
.catch(error => {
 document.getElementById('warning1').textContent = 'Warning: unavailable';
 });
 }

 function sendTextToArduino() {
 const text = document.getElementById('textToArduino').value;

 fetch('/sendText?message=' + encodeURIComponent(text))
.then(response => response.text())
.then(data => {
 document.getElementById('textSendStatus').textContent = 'Message: ' + data;
 })
.catch(error => {
 document.getElementById('textSendStatus').textContent = 'Message: error';
 });
 }

 setInterval(updateSensor1, 1000);
 setInterval(updateSensor2, 1000);
 setInterval(updateStatus1, 1000);
 setInterval(updateWarning1, 1000);

 updateSensor1();
 updateSensor2();
 updateStatus1();
 updateWarning1();
 </script>
</body>
</html>
)rawliteral";

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup() {
 pinMode(ledPin, OUTPUT);

 // Buttons use INPUT_PULLUP
 // Pressed = LOW
 // Not pressed = HIGH
 pinMode(button1Pin, INPUT_PULLUP);
 pinMode(button2Pin, INPUT_PULLUP);

 digitalWrite(ledPin, LOW);

 Serial.begin(9600);
 while (!Serial) {
;
 }

 Serial.println("Connecting to WiFi...");
 while (WiFi.begin(ssid, pass)!= WL_CONNECTED) {
 delay(1000);
 Serial.println("Trying again...");
 }

 server.begin();

 Serial.print("Connected! Open: http://");
 Serial.println(WiFi.localIP());
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------
void loop() {
 WiFiClient client = server.available();
 if (!client) {
 return;
 }

 String request = "";
 unsigned long timeout = millis();

 while (client.connected() && millis() - timeout < 5000) {
 while (client.available()) {
 char c = client.read();
 request += c;

 if (request.endsWith("\r\n\r\n")) {
 handleRequest(client, request);
 client.stop();
 return;
 }
 }
 }

 client.stop();
}

// --------------------------------------------------
// Request handler
// --------------------------------------------------
void handleRequest(WiFiClient &client, String request) {
 if (request.indexOf("GET /control1") >= 0) {
 ledState =!ledState;
 digitalWrite(ledPin, ledState? HIGH: LOW);
 sendText(client, ledState? "LED ON": "LED OFF");
 }
 else if (request.indexOf("GET /sensor1") >= 0) {
 bool pressed = (digitalRead(button1Pin) == LOW);
 sendText(client, pressed? "PRESSED": "NOT PRESSED");
 }
 else if (request.indexOf("GET /sensor2") >= 0) {
 bool pressed = (digitalRead(button2Pin) == LOW);
 sendText(client, pressed? "PRESSED": "NOT PRESSED");
 }
 else if (request.indexOf("GET /status1") >= 0) {
 sendText(client, ledState? "LED IS ON": "LED IS OFF");
 }
 else if (request.indexOf("GET /warning1") >= 0) {
 bool b1 = (digitalRead(button1Pin) == LOW);
 bool b2 = (digitalRead(button2Pin) == LOW);

 if (b1 && b2) {
 sendText(client, "BOTH BUTTONS PRESSED");
 } else {
 sendText(client, "none");
 }
 }
 else if (request.indexOf("GET /sendText?message=") >= 0) {
 int start = request.indexOf("message=") + 8;
 int end = request.indexOf(" ", start);
 String message = request.substring(start, end);

 // Basic decoding
 message.replace("%20", " ");
 message.replace("+", " ");

 Serial.println("Message received from webpage:");
 Serial.println(message);

 sendText(client, "received");
 }
 else {
 sendHTML(client, webpage);
 }
}

// --------------------------------------------------
// Response helpers
// --------------------------------------------------
void sendHTML(WiFiClient &client, const char* html) {
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/html");
 client.println("Connection: close");
 client.println();
 client.println(html);
}

void sendText(WiFiClient &client, String text) {
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/plain");
 client.println("Connection: close");
 client.println();
 client.println(text);
}
