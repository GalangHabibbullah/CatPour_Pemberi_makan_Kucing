#include "HX711.h"

#define DOUT D4   // Data pin HX711
#define CLK  D5   // Clock pin HX711

HX711 scale;

// Nilai kalibrasi awal (ubah setelah kalibrasi)
float calibration_factor = -7050; // Sesuaikan nanti!

void setup() {
  Serial.begin(115200);
  Serial.println("Kalibrasi Load Cell untuk Pakan Kucing");

  scale.begin(DOUT, CLK);
  scale.set_scale(); // Tidak ada kalibrasi dulu
  scale.tare();      // Set berat awal sebagai 0

  Serial.println("Letakkan beban referensi di atas piring...");
  delay(5000); // Tunggu pengguna letakkan beban
}

void loop() {
  float weight = scale.get_units(10); // Rata-rata 10 pembacaan

  Serial.print("Berat: ");
  Serial.print(weight, 2);
  Serial.println(" gram");

  delay(1000);
}
