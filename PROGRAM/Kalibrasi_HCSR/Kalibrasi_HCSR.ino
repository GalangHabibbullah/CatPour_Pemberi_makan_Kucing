#define TRIG_PIN 2  // Trigger di pin D2
#define ECHO_PIN 3  // Echo di pin D3

long duration;
float distance;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Kirim trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Baca durasi echo
  duration = pulseIn(ECHO_PIN, HIGH);

  // Hitung jarak dalam cm
  distance = duration * 0.0343 / 2;

  // Kirim ke Serial Monitor
  Serial.print("U:");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500);
}
