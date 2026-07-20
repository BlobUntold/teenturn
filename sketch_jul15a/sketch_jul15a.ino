#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Shibby";
char pass[] = "12345678";

WiFiServer server(80);

const int ledPin = 9;
const int button1Pin = 2;
const int button2Pin = 3;

bool ledState = false;

const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Arduino Dashboard</title>
	<style>
		:root {
			--bg: #0b1020;
			--panel: #1e3a5f;
			--panel-light: #ffffff;
			--panel-blue: #2563eb;
			--panel-warn: #f59e0b;
			--text-light: #ffffff;
			--text-dark: #111111;
			--btn: #7b2cff;
			--btn-hover: #9955ff;
		}

		* {
			box-sizing: border-box;
		}

		body {
			margin: 0;
			font-family: Arial, sans-serif;
			background: var(--bg);
			color: var(--text-light);
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

		@media (max-width: 800px) {
			.dashboard {
				grid-template-columns: 1fr;
			}
			.span-2 {
				grid-column: auto;
			}
		}

		.card {
			padding: 20px;
			border-radius: 16px;
			min-height: 160px;
		}

		.dark-card {
			background: var(--panel);
			color: var(--text-light);
		}

		.light-card {
			background: var(--panel-light);
			color: var(--text-dark);
		}

		.blue-card {
			background: var(--panel-blue);
			color: var(--text-light);
		}

		.warning-card {
			background: var(--panel-warn);
			color: var(--text-dark);
		}

		.status-box {
			margin-top: 12px;
			padding: 12px;
			border-radius: 10px;
			background: rgba(255, 255, 255, 0.92);
			color: #111;
			font-weight: bold;
		}

		button {
			background: var(--btn);
			color: #fff;
			border: none;
			border-radius: 10px;
			padding: 12px 18px;
			cursor: pointer;
			margin-top: 10px;
			font-size: 1rem;
		}

		button:hover {
			background: var(--btn-hover);
		}

		input[type="text"] {
			width: 100%;
			padding: 12px;
			border-radius: 10px;
			border: 1px solid #ccc;
			margin-top: 10px;
			box-sizing: border-box;
		}
	</style>
</head>
<body>
	<div class="container">
		<div class="dashboard">
			<div class="card blue-card span-2">
				<h2>Arduino Dashboard</h2>
				<p>Toggle LED and monitor button and warning states.</p>
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
		var isPolling = false;

		function setStatus(id, text) {
			var el = document.getElementById(id);
			if (el) {
				el.textContent = text;
			}
		}

		function requestText(path, onSuccess, onError) {
			var xhr = new XMLHttpRequest();
			var sep = path.indexOf('?') >= 0 ? '&' : '?';
			xhr.open('GET', path + sep + 'ts=' + Date.now(), true);
			xhr.timeout = 1800;

			xhr.onreadystatechange = function () {
				if (xhr.readyState === 4) {
					if (xhr.status >= 200 && xhr.status < 300) {
						onSuccess(xhr.responseText);
					} else {
						onError();
					}
				}
			};

			xhr.ontimeout = onError;
			xhr.onerror = onError;
			xhr.send();
		}

		function runControl1() {
			requestText('/control1', function (data) {
				setStatus('control1Status', 'Control 1: ' + data);
				updateStatus1();
			}, function () {
				setStatus('control1Status', 'Control 1: error');
			});
		}

		function updateSensor1() {
			requestText('/sensor1', function (data) {
				setStatus('sensor1', 'Sensor 1: ' + data);
			}, function () {
				setStatus('sensor1', 'Sensor 1: unavailable');
			});
		}

		function updateSensor2() {
			requestText('/sensor2', function (data) {
				setStatus('sensor2', 'Sensor 2: ' + data);
			}, function () {
				setStatus('sensor2', 'Sensor 2: unavailable');
			});
		}

		function updateStatus1() {
			requestText('/status1', function (data) {
				setStatus('status1', 'Status 1: ' + data);
			}, function () {
				setStatus('status1', 'Status 1: unavailable');
			});
		}

		function updateWarning1() {
			requestText('/warning1', function (data) {
				setStatus('warning1', 'Warning: ' + data);
			}, function () {
				setStatus('warning1', 'Warning: unavailable');
			});
		}

		function sendTextToArduino() {
			var input = document.getElementById('textToArduino');
			var text = input ? input.value : '';
			requestText('/sendText?message=' + encodeURIComponent(text), function (data) {
				setStatus('textSendStatus', 'Message: ' + data);
			}, function () {
				setStatus('textSendStatus', 'Message: error');
			});
		}

		function pollAll() {
			if (isPolling) {
				return;
			}

			isPolling = true;
			var remaining = 4;

			function doneOne() {
				remaining -= 1;
				if (remaining <= 0) {
					isPolling = false;
				}
			}

			requestText('/sensor1', function (data) {
				setStatus('sensor1', 'Sensor 1: ' + data);
				doneOne();
			}, function () {
				setStatus('sensor1', 'Sensor 1: unavailable');
				doneOne();
			});

			requestText('/sensor2', function (data) {
				setStatus('sensor2', 'Sensor 2: ' + data);
				doneOne();
			}, function () {
				setStatus('sensor2', 'Sensor 2: unavailable');
				doneOne();
			});

			requestText('/status1', function (data) {
				setStatus('status1', 'Status 1: ' + data);
				doneOne();
			}, function () {
				setStatus('status1', 'Status 1: unavailable');
				doneOne();
			});

			requestText('/warning1', function (data) {
				setStatus('warning1', 'Warning: ' + data);
				doneOne();
			}, function () {
				setStatus('warning1', 'Warning: unavailable');
				doneOne();
			});
		}

		pollAll();
		setInterval(pollAll, 2000);
	</script>
</body>
</html>
)rawliteral";

void sendResponse(WiFiClient &client, int statusCode, const char *statusText, const char *contentType, const String &body);
void sendHTMLPage(WiFiClient &client);
void sendNoContent(WiFiClient &client);
void sendCORSPreflight(WiFiClient &client);
bool parseRequestLine(const String &request, String &method, String &target);
String urlDecode(const String &input);
void handleRequest(WiFiClient &client, const String &request);

void setup() {
	pinMode(ledPin, OUTPUT);
	pinMode(button1Pin, INPUT_PULLUP);
	pinMode(button2Pin, INPUT_PULLUP);
	digitalWrite(ledPin, LOW);

	Serial.begin(9600);
	while (!Serial) {
	}

	Serial.println("Connecting to WiFi...");
	while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
		delay(1000);
		Serial.println("Trying again...");
	}

	server.begin();
	Serial.print("Connected! Open: http://");
	Serial.println(WiFi.localIP());
}

void loop() {
	WiFiClient client = server.available();
	if (!client) {
		return;
	}

	unsigned long timeout = millis();
	String request = "";

	while (client.connected() && millis() - timeout < 2000) {
		while (client.available()) {
			char c = (char)client.read();
			request += c;
			timeout = millis();

			if (request.length() > 2048) {
				sendResponse(client, 413, "Payload Too Large", "text/plain; charset=UTF-8", "Request too large");
				client.flush();
				delay(10);
				client.stop();
				return;
			}

			if (request.endsWith("\r\n\r\n")) {
				handleRequest(client, request);
				client.flush();
				delay(10);
				client.stop();
				return;
			}
		}
	}

	if (request.length() > 0) {
		handleRequest(client, request);
	} else {
		sendResponse(client, 408, "Request Timeout", "text/plain; charset=UTF-8", "Timeout waiting for request");
	}

	client.flush();
	delay(10);
	client.stop();
}

void handleRequest(WiFiClient &client, const String &request) {
	String method = "";
	String target = "";

	if (!parseRequestLine(request, method, target)) {
		sendResponse(client, 400, "Bad Request", "text/plain; charset=UTF-8", "Malformed HTTP request line");
		return;
	}

	Serial.print("REQUEST: ");
	Serial.print(method);
	Serial.print(" ");
	Serial.println(target);

	if (method == "OPTIONS") {
		sendCORSPreflight(client);
		return;
	}

	if (method != "GET") {
		sendResponse(client, 405, "Method Not Allowed", "text/plain; charset=UTF-8", "Only GET and OPTIONS are supported");
		return;
	}

	String path = target;
	String query = "";
	int q = target.indexOf('?');
	if (q >= 0) {
		path = target.substring(0, q);
		query = target.substring(q + 1);
	}

	if (path == "/" || path == "/index.html") {
		sendHTMLPage(client);
		return;
	}

	if (path == "/favicon.ico") {
		sendNoContent(client);
		return;
	}

	if (path == "/control1") {
		ledState = !ledState;
		digitalWrite(ledPin, ledState ? HIGH : LOW);
		sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", ledState ? "LED ON" : "LED OFF");
		return;
	}

	if (path == "/sensor1") {
		bool pressed = digitalRead(button1Pin) == LOW;
		sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", pressed ? "PRESSED" : "NOT PRESSED");
		return;
	}

	if (path == "/sensor2") {
		bool pressed = digitalRead(button2Pin) == LOW;
		sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", pressed ? "PRESSED" : "NOT PRESSED");
		return;
	}

	if (path == "/status1") {
		sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", ledState ? "LED IS ON" : "LED IS OFF");
		return;
	}

	if (path == "/warning1") {
		bool b1 = digitalRead(button1Pin) == LOW;
		bool b2 = digitalRead(button2Pin) == LOW;
		sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", (b1 && b2) ? "BOTH BUTTONS PRESSED" : "none");
		return;
	}

	if (path == "/sendText") {
		String message = "";
		int m = query.indexOf("message=");
		if (m >= 0) {
			message = query.substring(m + 8);
			int amp = message.indexOf('&');
			if (amp >= 0) {
				message = message.substring(0, amp);
			}
			message = urlDecode(message);
		}

		Serial.println("Message received from webpage:");
		Serial.println(message);
		sendResponse(client, 200, "OK", "text/plain; charset=UTF-8", "received");
		return;
	}

	sendResponse(client, 404, "Not Found", "text/plain; charset=UTF-8", "Route not found");
}

bool parseRequestLine(const String &request, String &method, String &target) {
	int lineEnd = request.indexOf("\r\n");
	if (lineEnd < 0) {
		lineEnd = request.indexOf('\n');
	}
	if (lineEnd <= 0) {
		return false;
	}

	String line = request.substring(0, lineEnd);
	line.trim();

	int s1 = line.indexOf(' ');
	if (s1 <= 0) {
		return false;
	}
	int s2 = line.indexOf(' ', s1 + 1);
	if (s2 <= s1 + 1) {
		return false;
	}

	String version = line.substring(s2 + 1);
	if (!version.startsWith("HTTP/")) {
		return false;
	}

	method = line.substring(0, s1);
	target = line.substring(s1 + 1, s2);

	if (target.startsWith("http://") || target.startsWith("https://")) {
		int slash = target.indexOf('/', target.indexOf("//") + 2);
		target = (slash >= 0) ? target.substring(slash) : "/";
	}

	return target.length() > 0;
}

String urlDecode(const String &input) {
	String out = "";
	for (unsigned int i = 0; i < input.length(); i++) {
		char c = input.charAt(i);
		if (c == '+') {
			out += ' ';
		} else if (c == '%' && i + 2 < input.length()) {
			char h1 = input.charAt(i + 1);
			char h2 = input.charAt(i + 2);

			int hi = (h1 >= '0' && h1 <= '9') ? (h1 - '0') : ((h1 >= 'A' && h1 <= 'F') ? (h1 - 'A' + 10) : ((h1 >= 'a' && h1 <= 'f') ? (h1 - 'a' + 10) : 0));
			int lo = (h2 >= '0' && h2 <= '9') ? (h2 - '0') : ((h2 >= 'A' && h2 <= 'F') ? (h2 - 'A' + 10) : ((h2 >= 'a' && h2 <= 'f') ? (h2 - 'a' + 10) : 0));

			out += (char)((hi << 4) | lo);
			i += 2;
		} else {
			out += c;
		}
	}
	return out;
}

void sendResponse(WiFiClient &client, int statusCode, const char *statusText, const char *contentType, const String &body) {
	client.print("HTTP/1.1 ");
	client.print(statusCode);
	client.print(" ");
	client.print(statusText);
	client.print("\r\n");
	client.print("Content-Type: ");
	client.print(contentType);
	client.print("\r\n");
	client.print("Cache-Control: no-store, no-cache, must-revalidate\r\n");
	client.print("Pragma: no-cache\r\n");
	client.print("Expires: 0\r\n");
	client.print("Access-Control-Allow-Origin: *\r\n");
	client.print("Connection: close\r\n");
	client.print("Content-Length: ");
	client.print(body.length());
	client.print("\r\n\r\n");
	client.print(body);
}

void sendHTMLPage(WiFiClient &client) {
	const size_t totalLength = strlen(webpage);

	client.print("HTTP/1.1 200 OK\r\n");
	client.print("Content-Type: text/html; charset=UTF-8\r\n");
	client.print("Cache-Control: no-store, no-cache, must-revalidate\r\n");
	client.print("Pragma: no-cache\r\n");
	client.print("Expires: 0\r\n");
	client.print("Access-Control-Allow-Origin: *\r\n");
	client.print("Connection: close\r\n");
	client.print("Content-Length: ");
	client.print(totalLength);
	client.print("\r\n\r\n");

	size_t sent = 0;
	unsigned long timeout = millis();
	while (sent < totalLength && millis() - timeout < 5000) {
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

	client.flush();

	Serial.print("HTML bytes sent: ");
	Serial.print(sent);
	Serial.print("/");
	Serial.println(totalLength);
}

void sendNoContent(WiFiClient &client) {
	client.print("HTTP/1.1 204 No Content\r\n");
	client.print("Access-Control-Allow-Origin: *\r\n");
	client.print("Connection: close\r\n");
	client.print("Content-Length: 0\r\n\r\n");
}

void sendCORSPreflight(WiFiClient &client) {
	client.print("HTTP/1.1 204 No Content\r\n");
	client.print("Access-Control-Allow-Origin: *\r\n");
	client.print("Access-Control-Allow-Methods: GET, OPTIONS\r\n");
	client.print("Access-Control-Allow-Headers: Content-Type\r\n");
	client.print("Access-Control-Max-Age: 86400\r\n");
	client.print("Connection: close\r\n");
	client.print("Content-Length: 0\r\n\r\n");
}
