#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

WiFiServer server(80);

const int ledPin = 9;
bool ledState = false;

const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
 <meta charset="UTF-8">
 <meta name="viewport" content="width=device-width, initial-scale=1.0">
 <title>LED Control</title>
 <style>
 body {
 font-family: Arial, sans-serif;
 text-align: center;
 background: `#0d1b2a`;
 color: white;
 padding-top: 60px;
 margin: 0;
 }
 h1 {
 font-size: 2em;
 }
 button {
 background: `#7b2cff`;
 color: white;
 border: none;
 padding: 20px 40px;
 font-size: 24px;
 border-radius: 12px;
 cursor: pointer;
 }
 button:hover {
 background: `#9955ff`;
 }
 `#status` {
 margin-top: 20px;
 font-size: 22px;
 }
 </style>
</head>
<body>
 <h1>LED Control</h1>
 <button onclick="toggleLED()">Toggle LED</button>
 <div id="status">Checking...</div>

 <script>
 function toggleLED() {
 fetch('/toggle')
.then(response => response.text())
.then(data => {
 document.getElementById('status').innerText = data;
 });
 }

 function getStatus() {
 fetch('/status')
.then(response => response.text())
.then(data => {
 document.getElementById('status').innerText = data;
 });
 }

 getStatus();
 </script>
</body>
</html>
)rawliteral";

void setup() {
 pinMode(ledPin, OUTPUT);
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

 Serial.print("Connected! Go to: http://");
 Serial.println(WiFi.localIP());
}

void loop() {
 WiFiClient client = server.available();
 if (!client) {
 return;
 }

 String request = "";
 unsigned long timeout = millis();

 while (client.connected() && millis() - timeout < 1000) {
 while (client.available()) {
 char c = client.read();
 request += c;
 if (request.endsWith("\\r\\n\\r\\n")) {
 handleRequest(client, request);
 client.stop();
 return;
 }
 }
 }

 client.stop();
}

void handleRequest(WiFiClient &client, String request) {
 if (request.indexOf("GET /toggle") >= 0) {
 ledState =!ledState;
 digitalWrite(ledPin, ledState? HIGH: LOW);
 sendText(client, ledState? "LED is ON": "LED is OFF");
 }
 else if (request.indexOf("GET /status") >= 0) {
 sendText(client, ledState? "LED is ON": "LED is OFF");
 }
 else {
 sendHTML(client, webpage);
 }
}

void sendHTML(WiFiClient &client, const char* html) {
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/html");
 client.println("Connection: close");
 client.println();
 client.println(html);
}

void sendText(WiFiClient &client, const char* text) {
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/plain");
 client.println("Connection: close");
 client.println();
 client.println(text);
}