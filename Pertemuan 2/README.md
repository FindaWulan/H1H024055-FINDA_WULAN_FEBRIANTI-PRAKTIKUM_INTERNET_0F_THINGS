# Modul 2 – Konfigurasi Jaringan (WiFi Station & Access Point) ESP32

**Mata Kuliah:** Praktikum IoT  
**Program Studi:** Teknik Komputer, UNSOED  
**Nama / NIM:** Finda Wulan Febrianti / H1H024055  
**Asisten:** Imedia Sholem Shoukat / H1D023088

---

## 1. Tujuan Praktikum
1. Memahami dan mengimplementasikan konfigurasi ESP32 menggunakan pustaka `WiFi.h` dalam mode Station (STA) dan Access Point (AP).
2. Menghubungkan ESP32 ke jaringan WiFi yang tersedia pada mode STA.
3. Membuat jaringan WiFi mandiri (AP) yang dapat diakses langsung tanpa router eksternal.
4. Mempelajari fungsi `WiFi.softAP()` untuk membangun jaringan pada ESP32.
5. Membaca dan menganalisis parameter jaringan (IP Address, MAC Address, RSSI, jumlah perangkat terhubung) melalui Serial Monitor.
6. Memahami proses provisioning awal perangkat IoT.

---

## 2. Rangkaian Percobaan
- LED indikator dihubungkan ke **GPIO 2** melalui resistor **220 Ω**, kaki lain LED ke **GND**.
- ESP32 dihubungkan ke laptop via kabel USB untuk pemrograman dan monitoring Serial.

---

## 3. Percobaan 2A — Mode Station (STA)

### Hasil Pengamatan
| Parameter | Hasil |
|---|---|
| Status Koneksi | Berhasil terhubung |
| IP Address | 10.118.80.182 |
| MAC Address | 84:F3:EB:E3:5C:15 |
| RSSI | -59 dBm |
| LED Indikator | Menyala |

### Dokumentasi Percobaan 2A 

<img width="960" height="1280" alt="Terhubung " src="https://github.com/user-attachments/assets/b6340eed-6ab7-48db-876f-112d8b5cac6f" />
Gambar 2.1 Jika terhubung

<img width="960" height="1280" alt="Tidak terhubung ssid salah" src="https://github.com/user-attachments/assets/2477b3a5-39d5-4cc8-bcf9-88cd1b61db5e" />
Gambar 2.2 Jika ssid salah maka led tidak menyala


### Program Dasar
Menggunakan `WiFi.mode(WIFI_STA)` lalu `WiFi.begin(ssid, password)`, dengan `while (WiFi.status() != WL_CONNECTED)` untuk menunggu koneksi sebelum menampilkan IP, MAC, dan RSSI ke Serial Monitor.


## 4. Percobaan 2B — Mode Access Point (AP)

### Hasil Pengamatan
| Parameter | Hasil |
|---|---|
| SSID Access Point | ESP32_AccessPoint |
| IP Address AP | 192.168.4.1 (default) |
| Perangkat berhasil terhubung? | Ya |
| Jumlah perangkat terhubung | 3 |

### Dokumentasi Percobaan2B

<img width="739" height="1600" alt="Terkoneksi " src="https://github.com/user-attachments/assets/242061c0-548f-4971-bb3d-e29e3b948a68" />
Gambar 2.3 Access Point dapat Terkonek di handphone

### Program Dasar
Menggunakan `WiFi.mode(WIFI_AP)` lalu `WiFi.softAP(ap_ssid, ap_password)`. IP default AP (`192.168.4.1`) didapat dari `WiFi.softAPIP()`, dan jumlah client dipantau lewat `WiFi.softAPgetStationNum()` di `loop()`.

---

## 5. Kendala Praktikum
- Sempat terjadi kesalahan rakitan pada resistor, sehingga akhirnya rangkaian dijalankan tanpa resistor.
- Access Point sempat tidak bisa diakses, namun setelah diulang kembali berjalan normal.

---

## 6. Jawaban Pertanyaan Analisis

**1. Pengaruh RSSI terhadap kestabilan koneksi WiFi pada perangkat IoT**  
RSSI menunjukkan kekuatan sinyal WiFi yang diterima (dBm), semakin mendekati 0 berarti semakin kuat. Semakin negatif nilainya (misal di bawah -80 dBm), semakin besar risiko packet loss, latensi tinggi, atau koneksi terputus karena data rentan mengalami interferensi. Pada IoT yang sering mengirim data berkala (misal pembacaan sensor), sinyal lemah bisa menyebabkan keterlambatan atau kegagalan transmisi, sehingga penempatan perangkat perlu memperhatikan jangkauan sinyal yang memadai.

**2. Cara ESP32 membedakan peran Station dan Access Point**  
Perannya ditentukan lewat `WiFi.mode()`. Saat `WIFI_STA`, ESP32 bertindak sebagai klien yang melakukan asosiasi/autentikasi ke access point tujuan via `WiFi.begin()` dan mendapat IP dari access point tersebut. Saat `WIFI_AP` (via `WiFi.softAP()`), ESP32 justru membangkitkan sinyalnya sendiri, memancarkan SSID, mengelola autentikasi perangkat yang bergabung, dan mengalokasikan IP ke tiap client. Kedua mode ini bisa berjalan bersamaan lewat `WIFI_AP_STA`.

**3. Manfaat kombinasi AP+STA dalam skenario provisioning IoT**  
Pada tahap provisioning, ESP32 mula-mula diaktifkan sebagai Access Point agar pengguna bisa terhubung langsung via smartphone tanpa jaringan lain, lalu melalui halaman web sederhana yang dihosting ESP32, pengguna memasukkan SSID/password WiFi rumah. Setelah kredensial tersimpan, ESP32 beralih/berjalan bersamaan pada mode Station untuk terhubung ke WiFi rumah sehingga bisa mengakses internet/server cloud untuk pengiriman data sensor, sementara AP tetap bisa dipertahankan aktif (mode AP+STA) bila sewaktu-waktu perlu konfigurasi ulang tanpa reset perangkat.
