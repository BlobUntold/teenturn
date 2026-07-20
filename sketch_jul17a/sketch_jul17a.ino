const int ledPin = D9;
const int buttonPin = D2;

bool ledState = false;
bool lastButtonState = HIGH;

void setup() {
 pinMode(ledPin, OUTPUT);
 pinMode(buttonPin, INPUT_PULLUP);
 digitalWrite(ledPin, LOW);
}

void loop() {
 bool buttonState = digitalRead(buttonPin);

 if (lastButtonState == HIGH && buttonState == LOW) {
 ledState =!ledState;
 digitalWrite(ledPin, ledState);
 delay(200); // simple debounce
 }

 lastButtonState = buttonState;
}
