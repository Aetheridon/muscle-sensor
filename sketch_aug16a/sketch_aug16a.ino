#include <WiFi.h>

const char* ssid     = "";
const char* password = "";

const uint16_t TCP_PORT = 9000;
WiFiServer server(TCP_PORT);
WiFiClient client;

const float ADC_REF_V = 3.3f;   // ADC reference voltage (V)
const int   ADC_RES   = 4095;   // 12-bit resolution -> counts 0..4095

struct Channel {
  int pin;
  const char* label;
};

const Channel CHANNELS[] = {
  { A0, "left_bicep" },
  { A1, "right_bicep" }
};
const size_t N_CH = sizeof(CHANNELS) / sizeof(CHANNELS[0]);

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

void sendCsvHeader(WiFiClient& c) {
  c.print("time_ms");

  for (size_t i = 0; i < N_CH; ++i) {
    c.print(","); 
    c.print(CHANNELS[i].label); 
    c.print("_mV");
  }
  c.print("\n");
}

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(115200);
  delay(200);

  connectWifi(); // Error checking of some kind, really need a display rigged up to this thing.
  server.begin();
  Serial.print("TCP server started on: ");
  Serial.println(TCP_PORT);

  analogReadResolution(12); // 0..4095
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    blinkLED(250);
    WiFi.begin(ssid, password);
    delay(500);
    return;
  }

  if (!client || !client.connected()) {
    client = server.available();
    if (!client) {
      blinkLED(600);
      delay(20);
      return;
    } else {
      sendCsvHeader(client);
    }
  }

  digitalWrite(LED, HIGH);

  unsigned long t = millis();
  client.print(t);

  for (size_t i = 0; i < N_CH; ++i) {
    int   raw = analogRead(CHANNELS[i].pin);
    float mV  = (raw * ADC_REF_V * 1000.0f) / ADC_RES;

    client.print(",");
    client.print((int)mV);
  }
  client.print("\n");

  if (!client.connected()) {
    client.stop();
    digitalWrite(LED, LOW);
  }

  delay(20); // ~50 Hz
}
