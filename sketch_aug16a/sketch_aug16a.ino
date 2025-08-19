#include <WiFi.h>

const char* ssid     = "";
const char* password = "";

const uint16_t TCP_PORT = 9000;
WiFiServer server(TCP_PORT);
WiFiClient client;

const int   PIN_EMG   = A0;
const float ADC_REF_V = 3.3f;
const int   ADC_RES   = 4095;

const int LED = LED_BUILTIN;

void blinkLED(unsigned long periodMs) {
  static unsigned long last = 0;
  static bool state = false;
  unsigned long now = millis();
  if (now - last >= periodMs) {
    last = now;
    state = !state;
    digitalWrite(LED, state ? HIGH : LOW);
  }
}

void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    blinkLED(250); // fast blink while trying to connect
  }

  Serial.print("\nConnected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(115200);
  delay(1000);

  connectWifi();
  server.begin();
  Serial.print("TCP server started on: ");
  Serial.println(TCP_PORT);

  analogReadResolution(12); // 0..4095
}

void loop() {
  // check for connection
  if (WiFi.status() != WL_CONNECTED) {
    blinkLED(250);
    WiFi.begin(ssid, password);
    delay(500);
    return;
  }

  // accept client and blink slowly while waiting
  if (!client || !client.connected()) {
    client = server.available();
    if (!client) {
      blinkLED(600); // slow blink = wifi connected and waiting for client
      /*long rssi = WiFi.RSSI();
      Serial.print("rssi: ");
      Serial.println(rssi);
      */
      delay(20);
      return;
    } else {
      client.println("time_ms,raw,mV"); // header
    }
  }

  // Client connected -> solid LED
  digitalWrite(LED, HIGH); //TODO: LED doesn't turn on

  // Sample and stream
  unsigned long t = millis();
  int   raw = analogRead(PIN_EMG);
  float mV  = (raw * ADC_REF_V * 1000.0f) / ADC_RES;

  client.print(t); client.print(",");
  client.print(raw); client.print(",");
  client.print((int)mV); client.print("\n");

  // Handle disconnect
  if (!client.connected()) {
    client.stop();
    digitalWrite(LED, LOW);
  }

  delay(20); // ~50 Hz
}
