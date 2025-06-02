import serial
import firebase_admin
from firebase_admin import credentials, db
import time
import re
from datetime import datetime
import threading

# === Konfigurasi Serial ===
PORT = 'COM7'
BAUD = 9600

# === Firebase Setup ===
cred = credentials.Certificate("catpour-f3802-firebase-adminsdk-fbsvc-3c26035daf.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://catpour-f3802-default-rtdb.firebaseio.com/'
})
ref_terbaru = db.reference('sensor_data/terbaru')
ref_log = db.reference('sensor_data/log')

# === Fungsi bantu ===
def hitung_persen_isi(jarak_cm):
    JARAK_MIN = 2.0    # Jarak saat penuh (cm)
    JARAK_MAKS = 14.0  # Jarak saat kosong (cm)

    try:
        jarak_cm = float(jarak_cm)
        if jarak_cm <= 0 or jarak_cm > 100:
            return "Sensor tidak dikenal"
        persen = ((JARAK_MAKS - jarak_cm) / (JARAK_MAKS - JARAK_MIN)) * 100
        return round(max(0, min(persen, 100)), 2)
    except:
        return "Sensor tidak dikenal"

def validasi_berat(berat_str):
    try:
        berat = float(berat_str)
        return round(berat, 2) if berat >= 0 else "Sensor tidak dikenal"
    except:
        return "Sensor tidak dikenal"

# === Koneksi Serial ===
try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"[INFO] Terhubung ke {PORT}")
except serial.SerialException as e:
    print(f"[ERROR] Gagal membuka port {PORT}: {e}")
    exit()

# === Variabel Penampung Data ===
data_dict = {
    "servo": "Sensor tidak dikenal",
    "sisa_pakan_persen": "Sensor tidak dikenal",
    "berat": "Sensor tidak dikenal",
    "jarak": "Sensor tidak dikenal",
    "waktu": ""
}

# === Fungsi Kirim Perintah ke Arduino ===
def beri_makan():
    try:
        ser.write(b'beri\n')
        print("[KIRIM] Perintah 'beri' dikirim ke Arduino.")
    except Exception as e:
        print(f"[ERROR] Gagal mengirim perintah: {e}")

# === Fungsi utama membaca & upload data ===
def baca_serial():
    try:
        while True:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
            except Exception as e:
                print(f"[ERROR] Baca serial gagal: {e}")
                continue

            if not line:
                continue

            print(f"[DATA] {line}")

            # Servo: "Servo : Buka (90)" atau "Servo : Tutup (0)"
            if line.lower().startswith("servo"):
                match = re.search(r'Servo\s+:\s+(Buka|Tutup)\s+\((\d+)\)', line, re.IGNORECASE)
                if match:
                    posisi = match.group(1).capitalize()
                    sudut = int(match.group(2))
                    data_dict["servo"] = f"{posisi} ({sudut}°)"

            # Berat: "Berat Timbangan : 123.45"
            elif "Berat Timbangan" in line:
                match = re.search(r'Berat Timbangan\s*:\s*([\d.]+)', line)
                if match:
                    berat = validasi_berat(match.group(1))
                    data_dict["berat"] = f"{berat} g" if isinstance(berat, float) else berat

            # Jarak: "Jarak Sensor : 5.67"
            elif "Jarak Sensor" in line:
                match = re.search(r'Jarak Sensor\s*:\s*([\d.]+)', line)
                if match:
                    jarak_cm = float(match.group(1))
                    persen = hitung_persen_isi(jarak_cm)
                    data_dict["jarak"] = f"{jarak_cm:.2f} cm"
                    data_dict["sisa_pakan_persen"] = f"{persen}%" if isinstance(persen, float) else persen

            # Separator: "====="
            elif line.startswith("===="):
                data_dict["waktu"] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                ref_terbaru.set(data_dict)
                ref_log.push(data_dict)
                print("[UPLOAD] Data dikirim ke Firebase:", data_dict)

            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n[EXIT] Program dihentikan.")
    finally:
        ser.close()
        print("[CLOSE] Port serial ditutup.")

# === Jalankan pembacaan serial di thread terpisah ===
threading.Thread(target=baca_serial, daemon=True).start()

# === Loop interaktif terminal ===
while True:
    cmd = input("Ketik 'beri' untuk memberi makan (atau 'exit' untuk keluar): ").strip().lower()
    if cmd == "beri":
        beri_makan()
    elif cmd == "exit":
        break
