#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Shibby";
char pass[] = "12345678";
WiFiServer server(80);

void setup() {
 Serial.begin(9600);
 while (!Serial);

 Serial.println("Connecting to WiFi...");
 while (WiFi.begin(ssid, pass)!= WL_CONNECTED) {
 delay(1000);
 Serial.println("Trying again...");
 }

 server.begin();

 Serial.print("Connected! Open: http://");
 Serial.println(WiFi.localIP());
}

void loop() {
 WiFiClient client = server.available();
 if (!client) return;

 Serial.println("Client connected");

 unsigned long start = millis();
 while (client.connected() && millis() - start < 2000) {
 if (client.available()) {
 String line = client.readStringUntil('\\n');
 Serial.print("Request line: ");
 Serial.println(line);

 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/plain");
 client.println("Connection: close");
 client.println();
 client.println("Hello from Arduino");

 delay(10);
 client.stop();
 Serial.println("Response sent");
 return;
 }
 }

 client.stop();
 Serial.println("Client timeout/disconnected");
}
