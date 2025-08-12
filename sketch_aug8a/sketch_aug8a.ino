const int emgPins[] = {A0, A1};
const int N = 2;

const float ADC_REF_V = 3.3f;
const int ADC_RES = 4095;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(300);
  Serial.println("ch0,ch1");
}

void loop() {
  const int AVG = 4;
  float v[N];

  for (int i = 0; i < N; i++) {
    long sum = 0;
    for (int k = 0; k < AVG; k++) sum += analogRead(emgPins[i]);
    int raw = sum / AVG;
    v[i] = (raw * ADC_REF_V) / ADC_RES;
  }

  Serial.print(v[0], 3);
  Serial.print(",");
  Serial.print(v[1], 3);
  Serial.println();

  delay(5);

}
