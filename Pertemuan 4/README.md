# Modul 4: Pertukaran Data Dua Arah (MQTT)

**Praktikum IoT | Teknik Komputer UNSOED | Semester 5 (2026)**
Nama: Finda Wulan Febrianti (H1H024055) | Asisten: Imedia Sholem Shoukat (H1D023088)

## Daftar Isi
1. [Tujuan](#1-tujuan)
2. [Detail Percobaan](#2-detail-percobaan)
3. [Dependencies](#3-dependencies)
4. [Skematik Rangkaian](#4-skematik-rangkaian)
5. [Percobaan 4A: Subscribe & Deserialisasi JSON](#5-percobaan-4a-subscribe--deserialisasi-json)
6. [Percobaan 4B: Publish & Subscribe Bersamaan](#6-percobaan-4b-publish--subscribe-bersamaan)
7. [Tugas Tambahan](#7-tugas-tambahan)
8. [Jawaban Pertanyaan Praktikum](#8-jawaban-pertanyaan-praktikum)
9. [Analisis Keseluruhan](#9-analisis-keseluruhan)
10. [Kendala](#10-kendala)

## 1. Tujuan
1. Memahami pertukaran data dua arah (bidirectional) pada IoT.
2. Memahami subscribe dan deserialisasi JSON pada ESP8266.
3. Menerima perintah kendali via MQTT untuk menggerakkan aktuator real-time (4A).
4. Membuat sistem yang publish data sensor sekaligus menerima perintah (full duplex) (4B).
5. Menganalisis mekanisme pertukaran data IoT pada sistem yang saling terhubung.

## 2. Detail Percobaan

| Percobaan | Peran ESP8266 | Topic |
|---|---|---|
| 4A | Subscriber (terima perintah ON/OFF dari MQTT Explorer, kendalikan LED) | Subscribe: `unsoed/tk245004/kelompok3/perintah` |
| 4B | Publisher + subscriber (kirim suhu DHT11 berkala, tetap terima perintah LED) | Publish: `unsoed/tk245004/kelompok3/data`, Subscribe: `.../perintah` |

Format perintah: `{"perintah":"ON"}` atau `{"perintah":"OFF"}`. Broker: `broker.hivemq.com:1883`.

## 3. Dependencies

| Library | Fungsi |
|---|---|
| `ESP8266WiFi` | Koneksi WiFi |
| `PubSubClient` | Client MQTT (publish, subscribe, callback) |
| `ArduinoJson` (v7) | Serialisasi dan deserialisasi JSON |
| `DHT sensor library` | Membaca DHT11 (hanya 4B) |

Tools: Arduino IDE 2.x, MQTT Explorer.

## 4. Skematik Rangkaian

| Komponen | Pin NodeMCU |
|---|---|
| LED (anoda, lewat resistor) | D2 |
| LED (katoda) | GND |
| DHT11 VCC | 3V3 |
| DHT11 DATA | D5 |
| DHT11 GND | GND |
| Buzzer (tugas tambahan) | D1 |

<<img width="960" height="1280" alt="Rangkaian hadware" src="https://github.com/user-attachments/assets/d1238d64-1a37-4df9-bea8-07619925345d" />
![Foto Rangkaian](../docs/images/modul4-skematik.png)

## 5. Percobaan 4A: Subscribe & Deserialisasi JSON

File: `modul4_subscribe_json`

### 5.1 Kode
```cpp
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topicPerintah = "unsoed/tk245004/kelompok3/perintah";
const int ledPin = D2;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(pesan);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, pesan);
  if (error) {
    Serial.print("Gagal parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* perintah = doc["perintah"];
  if (!perintah) return;

  if (String(perintah) == "ON") {
    digitalWrite(ledPin, HIGH);
    Serial.println("Aktuator: ON");
  } else if (String(perintah) == "OFF") {
    digitalWrite(ledPin, LOW);
    Serial.println("Aktuator: OFF");
  }
}

void hubungkanWiFi() { /* sama seperti Modul 3B */ }

void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(topicPerintah);
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  hubungkanWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) hubungkanMQTT();
  client.loop();
}
```
> Versi lengkap (dengan log Serial) ada di file `modul4_subscribe_json`.

### 5.2 Penjelasan per fungsi

| Fungsi | Penjelasan |
|---|---|
| `callback(topic, payload, length)` | Dipanggil otomatis tiap ada pesan masuk di topic yang di-subscribe. Menyalin payload (deretan byte) ke `String pesan`, mengurai JSON, lalu menggerakkan LED |
| `hubungkanWiFi()` | Menyambungkan ke WiFi dan menunggu `WL_CONNECTED` |
| `hubungkanMQTT()` | Menyambung ke broker dengan client ID acak; setelah berhasil langsung `client.subscribe(topicPerintah)`; gagal diulang tiap 2 detik |
| `setup()` | Inisialisasi Serial, pin LED sebagai OUTPUT (awal LOW), koneksi WiFi, lalu `setServer()` dan `setCallback()` |
| `loop()` | Reconnect bila putus, dan `client.loop()` terus dipanggil agar pesan masuk diterima |

### 5.3 Penjelasan percabangan

| Kondisi | Penjelasan |
|---|---|
| `if (error)` | Jika `deserializeJson` gagal (bukan JSON valid), cetak "Gagal parsing JSON" lalu `return`, tidak ada perintah dieksekusi |
| `if (!perintah) return;` | Jika key `"perintah"` tidak ada di JSON, fungsi dihentikan agar tidak error |
| `if (String(perintah) == "ON")` | LED dinyalakan (`HIGH`) dan cetak "Aktuator: ON" |
| `else if (String(perintah) == "OFF")` | LED dimatikan (`LOW`) dan cetak "Aktuator: OFF" |
| Nilai selain ON/OFF | Tidak ada aksi |
| `if (!client.connected())` di `loop()` | Memanggil `hubungkanMQTT()` untuk reconnect dan subscribe ulang |

### 5.4 Hasil

| Aspek | Hasil |
|---|---|
| Koneksi WiFi & broker | Berhasil |
| Topic di-subscribe | `unsoed/tk245004/kelompok3/perintah` |
| Pesan `{"perintah":"ON"}` | LED D2 menyala, Serial: "Aktuator: ON" |
| Pesan `{"perintah":"OFF"}` | LED mati, Serial: "Aktuator: OFF" |
| Kesesuaian spesifikasi | Sesuai |

<img width="1280" height="720" alt="LED Mati" src="https://github.com/user-attachments/assets/1a1fc537-742e-43b6-a5c5-3cd99aba3d73" />

![Serial Monitor 4A](../docs/images/modul4-4a-serial.png)

<img width="1280" height="720" alt="LED Menyala A" src="https://github.com/user-attachments/assets/99db045f-1389-4e36-a505-644af57a93a3" />

![MQTT Explorer dan LED 4A](../docs/images/modul4-4a-led.png)

<img width="1600" height="946" alt="percobaa 4A serial monitor" src="https://github.com/user-attachments/assets/930d5b28-3260-49a0-92ee-fade968b02fb" />

![Demo 4A](../docs/media/modul4-4a-demo.gif)

## 6. Percobaan 4B: Publish & Subscribe Bersamaan

File: `modul4_dua_arah_mqtt`

### 6.1 Kode (bagian yang berbeda dari 4A)
```cpp
#include <DHT.h>

const char* topicData = "unsoed/tk245004/kelompok3/data";
const char* topicPerintah = "unsoed/tk245004/kelompok3/perintah";
#define DHTPIN D5
#define DHTTYPE DHT11
const int ledPin = D2;

DHT dht(DHTPIN, DHTTYPE);
unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000;

void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;
  for (unsigned int i = 0; i < length; i++) pesan += (char)payload[i];
  JsonDocument doc;
  if (deserializeJson(doc, pesan)) return;
  const char* perintah = doc["perintah"];
  if (!perintah) return;
  digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
  Serial.print("Perintah diterima -> Aktuator: ");
  Serial.println(perintah);
}

void loop() {
  if (!client.connected()) hubungkanMQTT();
  client.loop();

  if (millis() - waktuTerakhirPublish > intervalPublish) {
    waktuTerakhirPublish = millis();
    float suhu = dht.readTemperature();
    if (!isnan(suhu)) {
      JsonDocument doc;
      doc["suhu"] = suhu;
      char buffer[128];
      serializeJson(doc, buffer);
      client.publish(topicData, buffer);
      Serial.print("Data terkirim: ");
      Serial.println(buffer);
    } else {
      Serial.println("Gagal baca DHT11, cek kabel data");
    }
  }
}
```
> `hubungkanWiFi()`, `hubungkanMQTT()`, dan `setup()` mengikuti 4A, ditambah `dht.begin()` di `setup()`.

### 6.2 Penjelasan per fungsi

| Fungsi / bagian | Penjelasan |
|---|---|
| `topicData` & `topicPerintah` | Dua topic terpisah: data suhu (publish) dan perintah kendali (subscribe) |
| `waktuTerakhirPublish`, `intervalPublish` | Penyimpan waktu publish terakhir dan jeda 5000 ms untuk pewaktuan non-blocking |
| `callback()` | Seperti 4A tetapi lebih ringkas: salin payload, parse JSON, nyalakan/matikan LED dengan operator ternary |
| `hubungkanMQTT()` | Connect lalu subscribe ke `topicPerintah`; diulang otomatis tiap koneksi putus |
| `dht.begin()` | Inisialisasi sensor DHT11 di `setup()` |
| `loop()` | `client.loop()` jalan terus, publish suhu hanya bila interval terpenuhi |

### 6.3 Penjelasan percabangan

| Kondisi | Penjelasan |
|---|---|
| `if (deserializeJson(doc, pesan)) return;` | JSON tidak valid, pesan diabaikan |
| `if (!perintah) return;` | Key `"perintah"` tidak ada, diabaikan |
| `String(perintah) == "ON" ? HIGH : LOW` | Operator ternary: "ON" menjadi HIGH, selain itu LOW |
| `if (millis() - waktuTerakhirPublish > intervalPublish)` | Jika sudah lebih dari 5 detik sejak publish terakhir, perbarui waktu dan kirim data. Jika belum, loop lanjut tanpa menunggu (non-blocking) |
| `if (!isnan(suhu))` | Pembacaan valid: buat JSON dan publish. NaN: cetak "Gagal baca DHT11, cek kabel data" |

### 6.4 Hasil

| Aspek | Hasil |
|---|---|
| Koneksi WiFi & broker | Berhasil, otomatis subscribe ke topic perintah |
| Publish | `unsoed/tk245004/kelompok3/data` tiap 5 detik (`{"suhu":...}`) |
| Subscribe | `unsoed/tk245004/kelompok3/perintah` |
| Respon LED saat publish berjalan | LED tetap cepat merespons ON/OFF, tidak ada blocking |
| Kesesuaian spesifikasi | Sesuai |

<img width="1600" height="936" alt="Hasil Serial monitor B" src="https://github.com/user-attachments/assets/26168108-1d1b-4f0f-9325-53c6825d2f1b" />

![Serial Monitor 4B](../docs/images/modul4-4b-serial.png)

<img width="1280" height="720" alt="LED Menyala B" src="https://github.com/user-attachments/assets/d155ce5a-5f9a-4825-96b0-52fedc73ac02" />

![MQTT Explorer dan LED 4B](../docs/images/modul4-4b-led.png)


## 7. Tugas Tambahan

### 7.1 Intensitas PWM (4A No. 4)
Perintah bisa membawa kecerahan, contoh: `{"perintah":"ON","intensitas":128}`.

```cpp
int intensitas = doc["intensitas"] | 0;       // default 0 jika key tidak ada
intensitas = constrain(intensitas, 0, 255);   // batasi 0-255

if (String(perintah) == "ON") {
  analogWrite(ledPin, intensitas);
  Serial.print("Aktuator: ON, intensitas = ");
  Serial.println(intensitas);
} else if (String(perintah) == "OFF") {
  analogWrite(ledPin, 0);
  Serial.println("Aktuator: OFF");
}
// di setup():
analogWriteRange(255);   // rentang PWM ESP8266 jadi 0-255
analogWrite(ledPin, 0);
```

| Baris | Penjelasan |
|---|---|
| `doc["intensitas"] \| 0` | Ambil nilai intensitas; default 0 jika tidak ada |
| `constrain(intensitas, 0, 255)` | Membatasi nilai pada rentang 0-255 |
| `analogWrite(ledPin, intensitas)` | Mengatur kecerahan LED dengan PWM |
| `analogWriteRange(255)` | Mengubah rentang PWM ESP8266 dari bawaan 0-1023 menjadi 0-255 |


### 7.2 Topic terpisah untuk buzzer (4B No. 4)
```cpp
const char* topicLed    = "unsoed/tk245004/kelompok3/perintah/led";
const char* topicBuzzer = "unsoed/tk245004/kelompok3/perintah/buzzer";
const int buzzerPin = D1;

// callback():
int nilai = (String(perintah) == "ON") ? HIGH : LOW;
if (strcmp(topic, topicLed) == 0) {
  digitalWrite(ledPin, nilai);
  Serial.print("LED -> "); Serial.println(perintah);
} else if (strcmp(topic, topicBuzzer) == 0) {
  digitalWrite(buzzerPin, nilai);
  Serial.print("Buzzer -> "); Serial.println(perintah);
}

// hubungkanMQTT():
client.subscribe(topicLed);
client.subscribe(topicBuzzer);
```

| Baris | Penjelasan |
|---|---|
| `topicLed`, `topicBuzzer` | Dua topic perintah terpisah |
| `strcmp(topic, topicLed) == 0` | Membedakan asal pesan berdasarkan topic, lalu meneruskan ke aktuator yang tepat |
| Dua `client.subscribe()` | Kedua topic di-subscribe di `hubungkanMQTT()` |
| Publish suhu | Tetap non-blocking dengan `millis()` |

## 8. Jawaban Pertanyaan Praktikum

### Percobaan 4A (4.5.4)

**1) Flowchart callback**

```
Mulai
  -> Pesan masuk di topic
  -> Salin payload ke String
  -> deserializeJson(doc, pesan)
  -> Error parsing?
       Ya    -> cetak "Gagal parsing JSON" -> return
       Tidak -> ambil doc["perintah"]
             -> Key ada?
                  Tidak -> return
                  Ya    -> perintah == "ON"?
                             Ya    -> ledPin HIGH, cetak "Aktuator: ON"
                             Tidak -> perintah == "OFF"?
                                        Ya    -> ledPin LOW, cetak "Aktuator: OFF"
                                        Tidak -> tidak ada aksi
  -> Selesai, kembali ke loop()
```

**2) Jika pesan bukan JSON valid**
`deserializeJson()` mengembalikan error. Program mencetak "Gagal parsing JSON" beserta jenis errornya, lalu `return`, sehingga tidak ada perintah dieksekusi dan LED tetap di kondisi terakhir.

**3) Alasan `client.subscribe()` di dalam `hubungkanMQTT()`**
Subscribe hanya bisa dilakukan setelah koneksi ke broker berhasil, padahal keberhasilan itu belum pasti saat `setup()` berjalan. Selain itu subscribe bersifat per-sesi: bila koneksi putus lalu reconnect, `hubungkanMQTT()` dipanggil lagi sehingga subscribe otomatis diulang. Jika diletakkan di `setup()`, ESP8266 tidak lagi menerima pesan setelah reconnect.

**4) Modifikasi PWM:** lihat [7.1](#71-intensitas-pwm-4a-no-4).

### Percobaan 4B (4.6.4)

**1) Mengapa `delay()` panjang dihindari**
`delay()` bersifat blocking: menghentikan seluruh program termasuk `client.loop()`. Akibatnya pesan MQTT masuk tidak diproses, koneksi bisa putus, dan sistem tidak responsif terhadap perintah.

**2) Cara kerja pendekatan non-blocking `millis()`**
Waktu publish terakhir disimpan di `waktuTerakhirPublish`. Di tiap iterasi `loop()` dicek apakah `millis() - waktuTerakhirPublish > intervalPublish`. Jika ya, data dipublish dan waktu diperbarui; jika belum, loop tetap berjalan sehingga `client.loop()` terus dipanggil dan pesan masuk tetap diproses.

**3) Jika `client.loop()` jarang dipanggil**
Pesan masuk tertunda atau terlewat, koneksi bisa putus karena keep-alive tidak terjaga, dan sistem tidak responsif terhadap perintah.

**4) Modifikasi topic buzzer:** lihat [7.2](#72-topic-terpisah-untuk-buzzer-4b-no-4).

## 9. Analisis Keseluruhan

**1) Hasil tiap percobaan**
4A: ESP8266 berhasil subscribe, menerima JSON, deserialisasi, dan mengendalikan LED ON/OFF. 4B: publish suhu tiap 5 detik sekaligus menerima perintah LED, LED cepat merespons, tanpa blocking, koneksi stabil.

**2) Komunikasi satu arah vs dua arah**
Satu arah (publish saja): perangkat hanya mengirim data dan tidak bisa dikendalikan jarak jauh. Dua arah (publish + subscribe): perangkat mengirim data sekaligus menerima perintah, memungkinkan kendali real-time dan interaksi timbal balik.

**3) Mengapa non-blocking lebih sesuai untuk sistem dua arah**
Sistem dua arah harus selalu siap menerima pesan. `delay()` memblokir `client.loop()` sehingga pesan tertunda atau koneksi terputus, sedangkan `millis()` memungkinkan publish terjadwal tanpa menghentikan penerimaan.

**4) Contoh penerapan**
- Smart farming: sensor kelembaban tanah mempublish data, petani mengirim ON/OFF ke pompa air dari jauh.
- Smart home: sensor suhu mempublish data, pengguna menyalakan/mematikan AC lewat MQTT.

Manfaat: kendali jarak jauh, respons real-time, dan efisiensi.

## 10. Kendala
LED sempat redup setelah DHT11 dipasang, padahal di 4A (LED saja) nyalanya terang. Awalnya dicurigai nilai resistor, tetapi kode dan resistor tidak berubah, jadi yang dicek ulang adalah wiring DHT11 (urutan VCC, DATA, GND, dan pin D5).
