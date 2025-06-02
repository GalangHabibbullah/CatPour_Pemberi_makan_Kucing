#include <Servo.h>
#include "HX711.h"

// === Pin Definition ===
#define DT 4
#define SCK 5
#define SERVO_PIN 6
#define TRIG_PIN 2
#define ECHO_PIN 3

// === Global ===
Servo myServo;
HX711 scale;

String serialInput = "";
bool feeding = false;
int currentAngle = 0;

// === Fungsi ===
void handleCommand(String cmd);
float readDistanceCM();

void setup() {
  Serial.begin(9600);

  // Servo setup
  myServo.attach(SERVO_PIN);
  myServo.write(0); // Posisi awal
  currentAngle = 0;

  // HX711 setup
  scale.begin(DT, SCK);
  scale.set_scale(984.0);  // kalibrasi sesuai sensor
  delay(2000);
  scale.tare();

  // Ultrasonik setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Sistem siap. Ketik 'beri' untuk memberi makan.");
}

void loop() {
  // === Serial input ===
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialInput.length() > 0) {
        handleCommand(serialInput);
        serialInput = "";
      }
    } else {
      serialInput += c;
    }
  }

  // === Baca sensor ===
  float jarak = readDistanceCM();
  float berat = scale.get_units();

  // Hitung kapasitas
  float kapasitas = (14.0 - jarak) / 12.0 * 100.0;
  if (kapasitas < 0) kapasitas = 0;
  if (kapasitas > 100) kapasitas = 100;

  // Jika sedang feeding dan berat cukup
  if (feeding && berat >= 200.0) {
    myServo.write(0);
    currentAngle = 0;
    feeding = false;
    Serial.println("Berat 200g tercapai. Servo ditutup.");
  }

  // === Tampilkan data ke Serial Monitor ===
  Serial.println("======================================");
  Serial.print("Servo           : ");
  Serial.print(currentAngle == 0 ? "TUTUP" : "BUKA");
  Serial.print(" (");
  Serial.print(currentAngle);
  Serial.println(" derajat)");

  Serial.print("Kapasitas       : ");
  Serial.print(kapasitas, 1);
  Serial.println(" %");

  Serial.print("Berat Timbangan : ");
  Serial.print(berat, 2);
  Serial.println(" gram");

  Serial.print("Jarak Sensor    : ");
  Serial.print(jarak, 2);
  Serial.println(" cm");
  Serial.println("======================================\n");

  delay(1000);
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();
  if (cmd == "beri") {
    myServo.write(180);
    currentAngle = 180;
    feeding = true;
    Serial.println("Memberi makan...");
  } else {
    Serial.println("Perintah tidak dikenal.");
  }
}

// Fungsi jarak ultrasonik
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  float distance = duration * 0.0343 / 2;

  return distance;
}