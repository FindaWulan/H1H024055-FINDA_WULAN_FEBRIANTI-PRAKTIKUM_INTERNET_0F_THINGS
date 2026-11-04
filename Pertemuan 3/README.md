# Modul 3: Komunikasi HTTP dan MQTT

**Praktikum IoT | Teknik Komputer UNSOED | Semester 5 (2026)**
Nama: Finda Wulan Febrianti (H1H024055) | Asisten: Imedia Sholem Shoukat (H1D023088)

## Daftar Isi
1. [Tujuan](#1-tujuan)
2. [Detail Percobaan](#2-detail-percobaan)
3. [Dependencies](#3-dependencies)
4. [Percobaan 3A: HTTP POST](#5-percobaan-3a-http-post)
5. [Percobaan 3B: MQTT Publish](#6-percobaan-3b-mqtt-publish)
6. [Jawaban Pertanyaan Praktikum](#7-jawaban-pertanyaan-praktikum)
7. [Analisis Keseluruhan](#8-analisis-keseluruhan)
8. [Kendala](#9-kendala)

## 1. Tujuan
1. Memahami konsep dasar protokol komunikasi pada sistem IoT.
2. Memahami karakteristik dan perbedaan HTTP dan MQTT.
3. Mengirim data ESP8266 ke server dengan HTTP POST (3A).
4. Mengirim data ke broker MQTT dengan pola publish-subscribe berformat JSON (3B).
5. Menganalisis kelebihan dan kekurangan tiap protokol.

## 2. Detail Percobaan
Tidak ada rangkaian tambahan. NodeMCU ESP8266 cukup terhubung ke WiFi yang tersambung ke internet.

| Percobaan | Protokol | Tujuan data | Interval |
|---|---|---|---|
| 3A | HTTP POST | `https://httpbin.org/post` (server meng-echo data) | 10 detik |
| 3B | MQTT publish | `broker.hivemq.com:1883`, topic `unsoed/tk245004/kelompok3/sensor` | 5 detik |

Data yang dikirim berformat JSON: `{"suhu":28.5,"kelembaban":65.0}`.
Pada 3B, MQTT Explorer di laptop menjadi subscriber pada topic yang sama.

## 3. Dependencies

| Library | Dipakai di | Fungsi |
|---|---|---|
| `ESP8266WiFi` | 3A, 3B | Koneksi WiFi mode Station |
| `ESP8266HTTPClient` | 3A | Membuat request HTTP |
| `WiFiClientSecureBearSSL` | 3A | Koneksi HTTPS |
| `PubSubClient` | 3B | Client MQTT |
| `ArduinoJson` (v7) | 3A, 3B | Membuat JSON (`JsonDocument`) |

Tools: Arduino IDE 2.x, MQTT Explorer.
Board: NodeMCU 1.0 (ESP-12E Module), baud Serial Monitor 115200.


## 4. Percobaan 3A: HTTP POST

File: `modul3_komunikasi_http`

### 4.1 Kode
```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";
const char* serverUrl = "https://httpbin.org/post";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi berhasil terhubung!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient http;

    if (http.begin(*client, serverUrl)) {
      http.addHeader("Content-Type", "application/json");

      JsonDocument doc;
      doc["suhu"] = 28.5;
      doc["kelembaban"] = 65.0;
      String requestBody;
      serializeJson(doc, requestBody);

      Serial.print("Mengirim data: ");
      Serial.println(requestBody);

      int httpResponseCode = http.POST(requestBody);
      if (httpResponseCode > 0) {
        Serial.print("Kode Response HTTP: ");
        Serial.println(httpResponseCode);
        Serial.println("Isi Response:");
        Serial.println(http.getString());
      } else {
        Serial.print("Pengiriman gagal, kode error: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("Gagal memulai koneksi HTTP");
    }
  }
  delay(10000);
}
```

### 4.2 Penjelasan per fungsi

**`setup()`**
| Baris | Penjelasan |
|---|---|
| `Serial.begin(115200)` | Mengaktifkan Serial Monitor |
| `WiFi.begin(ssid, password)` | Memulai koneksi ke access point |
| `while (WiFi.status() != WL_CONNECTED)` | Menunggu sampai WiFi tersambung; tiap 500 ms mencetak titik |

**`loop()`**
| Bagian | Penjelasan |
|---|---|
| `BearSSL::WiFiClientSecure` + `client->setInsecure()` | Membuat koneksi HTTPS dan melewati validasi sertifikat (cukup untuk praktikum, tidak aman untuk produksi) |
| `http.begin(*client, serverUrl)` | Menyiapkan koneksi ke endpoint |
| `http.addHeader("Content-Type", "application/json")` | Memberi tahu server bahwa body berformat JSON |
| `JsonDocument doc` + `doc["suhu"]`, `doc["kelembaban"]` | Membuat objek JSON berisi pasangan key-value |
| `serializeJson(doc, requestBody)` | Mengubah objek JSON menjadi string |
| `http.POST(requestBody)` | Mengirim POST dan mengembalikan kode response |
| `http.getString()` | Mengambil isi balasan server |
| `http.end()` | Menutup koneksi |
| `delay(10000)` | Jeda 10 detik antar pengiriman |

### 4.3 Penjelasan percabangan

| Kondisi | Jika benar | Jika salah |
|---|---|---|
| `if (WiFi.status() == WL_CONNECTED)` | Lanjut proses pengiriman | Lewati pengiriman, coba lagi setelah `delay` |
| `if (http.begin(...))` | Susun dan kirim request | Cetak "Gagal memulai koneksi HTTP" |
| `if (httpResponseCode > 0)` | Request terkirim dan server membalas; kode dan isi response dicetak (kode bisa 200, 404, dll.) | Pengiriman gagal total; cetak kode error (nilai negatif) |

### 4.4 Hasil

| Aspek | Hasil |
|---|---|
| Koneksi WiFi | Berhasil, muncul "WiFi berhasil terhubung!" |
| Kode response | `200 (OK)` |
| Isi response | Echo JSON dari httpbin: field `data` dan `json` berisi suhu 28.5 dan kelembaban 65, header request, `origin`, dan `url` |
| Kesesuaian spesifikasi | Sesuai |

![Serial Monitor 3A](<img width="1600" height="900" alt="Hasil Konfirgurasi 3A HTTP" src="https://github.com/user-attachments/assets/051a8fac-5414-4b0b-9f42-99ee5fe32aaa" />
)


## 5. Percobaan 3B: MQTT Publish

File: `modul3_komunikasi_mqtt`

### 5.1 Kode
```cpp
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttTopic = "unsoed/tk245004/kelompok3/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

void hubungkanWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi berhasil terhubung!");
}

void hubungkanMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("berhasil terhubung!");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 2 detik");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  hubungkanWiFi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    hubungkanMQTT();
  }
  client.loop();

  JsonDocument doc;
  doc["suhu"] = 28.5;
  doc["kelembaban"] = 65.0;
  char buffer[128];
  serializeJson(doc, buffer);

  client.publish(mqttTopic, buffer);
  Serial.print("Data terkirim ke topic ");
  Serial.print(mqttTopic);
  Serial.print(": ");
  Serial.println(buffer);
  delay(5000);
}
```

### 5.2 Penjelasan per fungsi

| Fungsi / bagian | Penjelasan |
|---|---|
| `WiFiClient espClient` + `PubSubClient client(espClient)` | `espClient` adalah koneksi jaringan dasar yang dibungkus `client` untuk komunikasi MQTT |
| `hubungkanWiFi()` | Menyambungkan ke WiFi dan menunggu `WL_CONNECTED` |
| `hubungkanMQTT()` | Menyambungkan ke broker dengan client ID acak (agar tidak bentrok dengan perangkat lain) dan mengulang tiap 2 detik sampai berhasil |
| `setup()` | Inisialisasi Serial, koneksi WiFi, lalu `client.setServer()` menetapkan alamat dan port broker |
| `loop()` | Memastikan koneksi broker aktif, memanggil `client.loop()`, membuat JSON, lalu `client.publish()` tiap 5 detik |
| `client.loop()` | Menjaga koneksi (keep-alive) dan memproses pesan masuk/keluar; wajib dipanggil berkala |
| `client.publish(topic, buffer)` | Mengirim data JSON ke topic di broker |

### 5.3 Penjelasan percabangan

| Kondisi | Penjelasan |
|---|---|
| `while (!client.connected())` di `hubungkanMQTT()` | Terus mencoba selama belum terhubung |
| `if (client.connect(...))` | Berhasil: cetak "berhasil terhubung!". Gagal: cetak kode `rc` dari `client.state()`, tunggu 2 detik, ulangi |
| `if (!client.connected())` di `loop()` | Jika koneksi putus, panggil `hubungkanMQTT()` (auto-reconnect) |

### 5.4 Hasil

| Aspek | Hasil |
|---|---|
| Koneksi broker | Berhasil ke `broker.hivemq.com` |
| Topic | `unsoed/tk245004/kelompok3/sensor` |
| Data muncul di client MQTT | Ya, JSON dipublish tiap 5 detik tanpa gagal koneksi |
| Kesesuaian spesifikasi | Sesuai |


![<img width="1600" height="900" alt="Hasil Konfirgurasi percobaan 2 MQTT" src="https://github.com/user-attachments/assets/83498b87-5508-4033-8d40-d6f34aaf8043" />
]

## 6. Jawaban Pertanyaan Praktikum

### Percobaan 3A (3.5.4)

**1) Flowchart pengiriman data HTTP POST**

```
Mulai
  -> Inisialisasi Serial + WiFi.begin()
  -> Tunggu sampai WL_CONNECTED
  -> loop(): WiFi terhubung?
       Tidak -> delay 10 detik -> ulang
       Ya    -> http.begin() + addHeader(Content-Type)
             -> Buat JSON (suhu, kelembaban)
             -> serializeJson() jadi string
             -> http.POST()
             -> httpResponseCode > 0 ?
                  Ya    -> cetak kode + isi response
                  Tidak -> cetak kode error
             -> http.end()
             -> delay 10 detik -> ulang
```

**2) Fungsi `http.addHeader("Content-Type", "application/json")`**
Menambahkan header `Content-Type` bernilai `application/json` pada request. Header ini memberi tahu server bahwa body berformat JSON, sehingga server mem-parsing dengan benar, bukan sebagai teks biasa atau form-urlencoded.

**3) Arti kode response 200 dan contoh kode lain**
`200 (OK)` berarti request berhasil diterima, dipahami, dan diproses server. Contoh lain: `404 (Not Found)`, resource atau endpoint tidak ditemukan, misalnya karena URL salah.

**4) Modifikasi: menambah waktu `millis()` ke JSON**
```cpp
doc["suhu"] = 28.5;
doc["kelembaban"] = 65.0;
doc["waktu"] = millis();   // TAMBAHAN: waktu (ms) sejak ESP8266 menyala
```
`millis()` mengembalikan jumlah milidetik sejak board menyala, sehingga tiap data punya penanda waktu relatif.

### Percobaan 3B (3.6.4)

**1) Fungsi topic dan alasan harus unik**
Topic adalah alamat/label pengelompokan pesan; broker meneruskan pesan ke semua client yang subscribe pada topic yang sama. Topic harus unik (mis. memuat identitas kelompok) karena broker publik bisa diakses siapa saja. Jika topic umum dan sama dengan pengguna lain, data akan tercampur dan menimbulkan konflik.

**2) Fungsi `client.loop()` di setiap iterasi**
Menjaga koneksi ke broker tetap hidup (keep-alive), menangani pengiriman/penerimaan pesan secara internal, dan memanggil callback bila ada pesan masuk. Karena koneksi MQTT bersifat persisten, fungsi ini wajib dipanggil berkala agar koneksi tidak terputus.

**3) Jika koneksi ke broker terputus saat program berjalan**
`client.connected()` bernilai `false` di iterasi `loop()` berikutnya, sehingga `hubungkanMQTT()` dipanggil ulang dengan client ID baru dan mencoba tiap 2 detik sampai pulih. Selama itu, data tidak bisa dipublish (gagal atau tertunda).

## 7. Analisis Keseluruhan

**1) Hasil tiap percobaan**
Kedua percobaan mudah dikonfigurasi dan langsung berjalan/terhubung. 3A mendapat response 200 dengan echo data, 3B berhasil publish berkala ke broker.

**2) Overhead dan pola komunikasi HTTP vs MQTT**
- HTTP: overhead besar karena tiap request membawa header lengkap dan bersifat stateless (koneksi dibuka dari awal tiap kirim). Pola request-response satu ke satu.
- MQTT: overhead kecil karena koneksi persisten; publish berikutnya hanya membawa topic dan payload. Pola publish-subscribe, satu data bisa diterima banyak subscriber lewat broker.

**3) Protokol untuk data sensor terus-menerus jangka panjang**
MQTT lebih sesuai: dirancang untuk komunikasi kontinu dengan overhead rendah dan koneksi terjaga, jadi lebih hemat daya dan bandwidth (penting untuk perangkat berbaterai). HTTP lebih cocok untuk komunikasi yang jarang.

**4) Peran JSON untuk interoperabilitas**
JSON ringan, terstruktur, dan independen terhadap bahasa/platform, sehingga data dari ESP8266 mudah dibaca server Python, aplikasi web JavaScript, maupun aplikasi mobile. Struktur key-value memudahkan penambahan parameter baru tanpa mengubah seluruh format.

## 8. Kendala
Arduino IDE di laptop sendiri tidak bisa dipakai (penyebab tidak diketahui meski sudah dicoba berulang), sehingga praktikum dilanjutkan memakai laptop kelompok lain dan berhasil.
