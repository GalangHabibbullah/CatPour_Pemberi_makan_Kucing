#include <Servo.h>

Servo servoMotor;

int angleClosed = 0;   // Ubah sesuai posisi katup tertutup
int angleOpen = 90;    // Ubah sesuai posisi katup terbuka

void setup() {
  Serial.begin(9600);
  servoMotor.attach(D3);  // Gunakan pin digital 9 di Arduino Uno/Nano
  Serial.println("Kalibrasi Servo Katup Pakan");

  // Mulai dengan posisi tertutup
  servoMotor.write(angleClosed);
  Serial.println("Posisi: TERTUTUP");
  delay(3000);
}

void loop() {
  // Buka servo
  Serial.println("Buka katup pakan...");
  servoMotor.write(angleOpen);
  delay(3000); // Tunggu 3 detik (pantau seberapa banyak pakan yang keluar)

  // Tutup kembali
  Serial.println("Tutup katup pakan...");
  servoMotor.write(angleClosed);
  delay(5000); // Jeda sebelum loop berikutnya
}
