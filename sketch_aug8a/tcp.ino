#include <WiFi.h>

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const int   PIN_EMG   = A0;
const float ADC_REF_V = 3.3f;
const int   ADC_RES   = 4095;

WiFiServer server(9000);
WiFiClient client;

void connectWiFi() {
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < 20000) {
    delay(300); Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK. IP: "); Serial.println(WiFi.localIP());
    server.begin();
    Serial.println("TCP server started on :9000");
  } else {
    Serial.println("WiFi NOT connected. Will retry in loop.");
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogReadAveraging(8);
  delay(200);
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    delay(1000);
    return;
  }

  // Accept a new client if none or if previous disconnected
  if (!client || !client.connected()) {
    client = server.available();
    return; // wait for a client to connect
  }

  unsigned long t = millis();
  int   raw = analogRead(PIN_EMG);
  float mV  = (raw * ADC_REF_V * 1000.0f) / ADC_RES;

  // CSV line
  client.print(t);
  client.print(",");
  client.print(raw);
  client.print(",");
  client.print((int)mV);
  client.print("\n");

  // If client closed, drop it
  if (!client.connected()) client.stop();

  delay(20); // 50 Hz
}
