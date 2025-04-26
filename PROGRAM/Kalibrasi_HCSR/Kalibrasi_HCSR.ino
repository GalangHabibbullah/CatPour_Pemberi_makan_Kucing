#define TRIG_PIN D6
#define ECHO_PIN D7

// Tinggi maksimum tampungan (dari sensor ke dasar tampungan), misal 20 cm
#define MAX_TANK_HEIGHT_CM 20.0

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Kalibrasi Sensor HC-SR04 untuk Wadah Pakan Kucing");
}

void loop() {
  long duration;
  float distance_cm;

  // Kirim pulsa trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Baca durasi echo
  duration = pulseIn(ECHO_PIN, HIGH);

  // Hitung jarak dalam cm
  distance_cm = duration * 0.0343 / 2;

  // Konversi ke kapasitas tampungan dalam persen
  float height_filled = MAX_TANK_HEIGHT_CM - distance_cm;
  float percent_full = (height_filled / MAX_TANK_HEIGHT_CM) * 100.0;

  // Cegah nilai negatif
  if (percent_full < 0) percent_full = 0;
  if (percent_full > 100) percent_full = 100;

  // Tampilkan di Serial Monitor
  Serial.print("Tinggi terukur: ");
  Serial.print(distance_cm);
  Serial.print(" cm | Kapasitas tampungan: ");
  Serial.print(percent_full);
  Serial.println(" %");

  delay(1000);
}
