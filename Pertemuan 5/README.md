# Modul 5: Pengolahan & Analisis Data IoT

**Praktikum IoT | Teknik Komputer UNSOED | Semester 5 (2026)**
Nama: Finda Wulan Febrianti (H1H024055) | Asisten: Imedia Sholem Shoukat (H1D023088)

## Daftar Isi
1. [Tujuan](#1-tujuan)
2. [Detail Percobaan](#2-detail-percobaan)
3. [Dependencies](#3-dependencies)
4. [Skematik Rangkaian](#4-skematik-rangkaian)
5. [Percobaan 5A: Moving Average & Statistik (Edge)](#5-percobaan-5a-moving-average--statistik-edge)
6. [Percobaan 5B: Akuisisi, Analisis, Visualisasi](#6-percobaan-5b-akuisisi-analisis-visualisasi)
7. [Tugas Tambahan](#7-tugas-tambahan)
8. [Jawaban Pertanyaan Praktikum](#8-jawaban-pertanyaan-praktikum)
9. [Analisis Keseluruhan](#9-analisis-keseluruhan)
10. [Kendala & Catatan](#10-kendala--catatan)

## 1. Tujuan
1. Memahami tahapan pengolahan data IoT: akuisisi, pra-pemrosesan (pembersihan dan penyaringan derau), analisis statistik, visualisasi.
2. Memahami perbedaan edge processing (di perangkat) dan server processing (di komputer).
3. Mengimplementasikan moving average dan statistik min/max di mikrokontroler (5A).
4. Mengambil data IoT via MQTT dengan Python dan menyimpannya ke CSV (5B).
5. Menganalisis, memvisualisasikan data, dan mendeteksi anomali dengan z-score (5B).

## 2. Detail Percobaan

| Percobaan | Lokasi pengolahan | Isi |
|---|---|---|
| 5A | Di perangkat (ESP8266) | Moving average jendela 5 + nilai min/max, tampil di Serial Monitor tiap 2 detik |
| 5B | Di komputer (Python) | Subscribe 30 data dari MQTT, simpan CSV, statistik, z-score, grafik |

Topic: `unsoed/tk245004/kelompok3/data` (publish oleh ESP8266, subscribe oleh Python).

> **Penyesuaian:** ESP32 dan DHT22 tidak tersedia, jadi dipakai **NodeMCU ESP8266** dan **DHT11**. Pembacaan sensor di lapangan kurang stabil, jadi suhu di 5A dan 5B dihasilkan fungsi simulasi `bacaSuhuMock()` (pola gelombang sinus + derau acak + lonjakan anomali +8 °C tiap 25 pembacaan). Pola dan anomali yang terlihat dibangkitkan program, bukan murni dari kondisi lingkungan.

## 3. Dependencies

**Arduino**
| Library | Fungsi |
|---|---|
| `DHT sensor library` | Sensor DHT (5A) |
| `ESP8266WiFi` | Koneksi WiFi (5B publisher) |
| `PubSubClient` | Client MQTT (5B publisher) |
| `math.h` | `sin()`, `pow()`, `sqrt()` |

**Python**
```bash
pip install paho-mqtt pandas matplotlib
```
| Library | Fungsi |
|---|---|
| `paho-mqtt` (v2) | Subscribe ke broker MQTT |
| `pandas` | DataFrame, simpan CSV, statistik, `rolling()` |
| `matplotlib` | Grafik dan penanda anomali |
| `json`, `time` | Parsing payload dan stempel waktu |

## 4. Skematik Rangkaian
Mengikuti rangkaian Modul I: DHT VCC ke 3V3, DATA ke GPIO 4, GND ke GND.

<img width="720" height="1280" alt="komponen " src="https://github.com/user-attachments/assets/6bc7d309-3098-43f1-8d39-9e9dc1cd3c3b" />

![Foto Rangkaian Modul 5]

## 5. Percobaan 5A: Moving Average & Statistik (Edge)

File: `modul5_pengolahan_data`

### 5.1 Kode
```cpp
#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT11

// Mock function pengganti sensor DHT22
float bacaSuhuMock() {
  static unsigned long jumlahBaca = 0;
  jumlahBaca++;
  float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);
  float noise = random(-5, 6) * 0.1;
  float suhu = suhuDasar + noise;
  if (jumlahBaca % 25 == 0) {
    suhu += 8.0;   // simulasi anomali
  }
  return suhu;
}

DHT dht(DHTPIN, DHTTYPE);
const int N = 5;
float buffer[N];
int indeks = 0;
int jumlah = 0;
float suhuMin = 1000.0;
float suhuMax = -1000.0;

float hitungMovingAverage(float nilaiBaru) {
  buffer[indeks] = nilaiBaru;
  indeks = (indeks + 1) % N;
  if (jumlah < N) jumlah++;
  float total = 0;
  for (int i = 0; i < jumlah; i++) {
    total += buffer[i];
  }
  return total / jumlah;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  float suhu = bacaSuhuMock();
  if (isnan(suhu)) {
    Serial.println("Pembacaan gagal, data diabaikan");
  } else {
    float suhuFilter = hitungMovingAverage(suhu);
    if (suhu < suhuMin) suhuMin = suhu;
    if (suhu > suhuMax) suhuMax = suhu;

    Serial.print("Mentah: ");  Serial.print(suhu);
    Serial.print(" | Filter: "); Serial.print(suhuFilter);
    Serial.print(" | Min: ");    Serial.print(suhuMin);
    Serial.print(" | Max: ");    Serial.println(suhuMax);
  }
  delay(2000);
}
```

### 5.2 Penjelasan per fungsi

| Fungsi / variabel | Penjelasan |
|---|---|
| `DHTPIN`, `DHTTYPE` | Pin data (GPIO 4) dan tipe sensor (DHT11) |
| `bacaSuhuMock()` | Pengganti sensor. `suhuDasar` berosilasi dengan `sin()` agar mirip perubahan suhu ruangan, `noise` menambah fluktuasi kecil, dan tiap 25 pembacaan ditambah +8 °C sebagai anomali. `static jumlahBaca` menyimpan hitungan antar pemanggilan |
| `N`, `buffer[N]`, `indeks`, `jumlah` | Parameter dan penampung moving average (jendela 5 data, circular buffer) |
| `suhuMin`, `suhuMax` | Nilai ekstrem sejak program jalan; awalnya 1000 / -1000 agar pasti tergantikan di pembacaan pertama |
| `hitungMovingAverage(nilaiBaru)` | Simpan nilai ke buffer secara melingkar, jumlahkan isi buffer yang terisi, bagi dengan `jumlah` |
| `setup()` | Serial 115200 dan `dht.begin()` |
| `loop()` | Ambil suhu, validasi, hitung filter, perbarui min/max, tampilkan 4 nilai tiap 2 detik |

### 5.3 Penjelasan percabangan

| Kondisi | Penjelasan |
|---|---|
| `if (jumlahBaca % 25 == 0)` | Tiap pembacaan kelipatan 25, suhu ditambah 8 °C (anomali buatan) |
| `if (jumlah < N) jumlah++;` | Selama buffer belum penuh, rata-rata dihitung dari data yang ada saja (bukan dibagi N) |
| `if (isnan(suhu))` | Pembacaan gagal: cetak pesan dan data diabaikan. Valid: lanjut proses |
| `if (suhu < suhuMin)` / `if (suhu > suhuMax)` | Perbarui nilai minimum / maksimum bila ada nilai yang lebih ekstrem |
| `for (i = 0; i < jumlah; i++)` | Menjumlahkan seluruh isi buffer yang terisi |

### 5.4 Hasil

| Aspek | Hasil |
|---|---|
| Kompilasi & upload | Berhasil tanpa error |
| Data mentah vs filter | Mentah naik-turun dengan derau; hasil filter lebih halus dan landai |
| Min/Max | Terus diperbarui, termasuk saat lonjakan simulasi |
| Respon terhadap gangguan | Saat lonjakan, data mentah melonjak tajam sedangkan filter naik lebih bertahap |
| Kesesuaian spesifikasi | Sesuai |

<!-- TARUH GAMBAR: Serial Monitor 5A (Mentah / Filter / Min / Max) - foto menyusul -->
![Serial Monitor 5A](../docs/images/modul5-5a-serial.png)

<!-- TARUH GIF/VIDEO: demo 5A berjalan di Serial Monitor -->
![Demo 5A](../docs/media/modul5-5a-demo.gif)

## 6. Percobaan 5B: Akuisisi, Analisis, Visualisasi

### 6.1 Bagian ESP8266 (publisher)

File: `modul5_publisher_esp8266`

```cpp
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>

const char* WIFI_SSID = "NAMA_WIFI";
const char* WIFI_PASSWORD = "PASSWORD_WIFI";
const char* BROKER = "broker.hivemq.com";
const int PORT = 1883;
const char* TOPIC = "unsoed/tk245004/kelompok3/data";

WiFiClient espClient;
PubSubClient client(espClient);

float bacaSuhuMock() { /* sama seperti 5A */ }

void hubungkanWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi terhubung!");
  Serial.print("IP ESP8266: ");
  Serial.println(WiFi.localIP());
}

void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266-Kelompok3-";
    clientId += String(ESP.getChipId(), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Berhasil!");
    } else {
      Serial.print("Gagal, kode: ");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));
  hubungkanWiFi();
  client.setServer(BROKER, PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) hubungkanWiFi();
  if (!client.connected()) hubungkanMQTT();
  client.loop();

  float suhu = bacaSuhuMock();
  char payload[40];
  snprintf(payload, sizeof(payload), "{\"suhu\":%.2f}", suhu);

  if (client.publish(TOPIC, payload)) {
    Serial.println("Publish berhasil!");
  } else {
    Serial.println("Publish gagal!");
  }
  delay(2000);
}
```

| Fungsi | Penjelasan |
|---|---|
| `hubungkanWiFi()` | Mode Station, tunggu `WL_CONNECTED`, tampilkan IP |
| `hubungkanMQTT()` | Connect dengan client ID berbasis `ESP.getChipId()`; gagal: ulangi tiap 3 detik |
| `loop()` | Jaga koneksi WiFi + MQTT, `client.loop()`, baca suhu mock, susun JSON dengan `snprintf()`, publish tiap 2 detik |

| Percabangan | Penjelasan |
|---|---|
| `if (WiFi.status() != WL_CONNECTED)` | WiFi putus: sambung ulang |
| `if (!client.connected())` | Broker putus: sambung ulang |
| `if (client.publish(...))` | Berhasil: "Publish berhasil!". Gagal: "Publish gagal!" |

<img width="1509" height="1067" alt="mqtt 1" src="https://github.com/user-attachments/assets/0f6dcd1f-b8da-4f54-a4f3-0fd5da91ccf4" />

![MQTT Explorer]

<img width="1600" height="938" alt="Hasil Serial monitor A" src="https://github.com/user-attachments/assets/63629434-0105-48a3-a8e4-1fe0b5656f79" />

![Serial Monitor Publisher](../docs/images/modul5-publisher-serial.png)

### 6.2 Bagian Python

File: `modul5_analisis_data.py`

```python
import json
import time
import paho.mqtt.client as mqtt
import pandas as pd
import matplotlib.pyplot as plt

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "unsoed/tk245004/kelompok3/data"
JUMLAH_SAMPEL = 30
data = []

def on_connect(client, userdata, flags, reason_code, properties):
    print("Terhubung ke broker, kode:", reason_code)
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        suhu = float(payload["suhu"])
    except (ValueError, KeyError):
        print("Data tidak valid, diabaikan")
        return
    data.append({"waktu": time.strftime("%Y-%m-%d %H:%M:%S"), "suhu": suhu})
    print(f"[{len(data)}/{JUMLAH_SAMPEL}] suhu = {suhu}")
    if len(data) >= JUMLAH_SAMPEL:
        client.disconnect()

# Tahap 1: akuisisi
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT)
client.loop_forever()

# Tahap 2: simpan CSV
df = pd.DataFrame(data)
df.to_csv("data_suhu.csv", index=False)

# Tahap 3: statistik + z-score
rata2 = df["suhu"].mean()
std = df["suhu"].std()
if std == 0:
    df["zscore"] = 0.0
else:
    df["zscore"] = (df["suhu"] - rata2) / std
df["anomali"] = df["zscore"].abs() > 2

# Tahap 4: visualisasi
plt.figure(figsize=(10, 4))
plt.plot(df.index, df["suhu"], marker="o", label="Suhu")
anom = df[df["anomali"]]
plt.scatter(anom.index, anom["suhu"], color="red", s=100, label="Anomali", zorder=3)
plt.axhline(rata2, color="gray", linestyle="--", label="Rata-rata")
plt.xlabel("Urutan data"); plt.ylabel("Suhu (°C)")
plt.title("Data Suhu dan Deteksi Anomali")
plt.legend(); plt.tight_layout()
plt.savefig("grafik_suhu.png")
plt.show()
```

> Statistik (rata-rata, std, min, max, jumlah anomali) juga dicetak dengan `print()` di file asli.

Cara menjalankan:
```bash
pip install paho-mqtt pandas matplotlib
python modul5_analisis_data.py
```
Pastikan ESP8266 sudah publish sebelum/selagi script berjalan.

| Fungsi / tahap | Penjelasan |
|---|---|
| `on_connect()` | Dipanggil saat tersambung ke broker; cetak kode status dan langsung subscribe ke `TOPIC` |
| `on_message()` | Dipanggil tiap pesan masuk; parse JSON, ambil `suhu`, simpan bersama waktu terima ke list `data`, cetak progres `[n/30]` |
| Tahap 1: akuisisi | `loop_forever()` menerima pesan sampai `client.disconnect()` dipanggil |
| Tahap 2: CSV | `data` menjadi DataFrame lalu disimpan ke `data_suhu.csv` |
| Tahap 3: statistik & z-score | Hitung rata-rata dan simpangan baku, z-score tiap data, tandai `\|z\| > 2` sebagai anomali |
| Tahap 4: visualisasi | Plot garis suhu, titik merah untuk anomali, garis putus-putus rata-rata, simpan `grafik_suhu.png` |

| Percabangan | Penjelasan |
|---|---|
| `try ... except (ValueError, KeyError)` | Payload bukan JSON valid atau tak punya key `suhu`: data diabaikan, program tidak crash |
| `if len(data) >= JUMLAH_SAMPEL` | Sampel sudah 30: putuskan koneksi, akuisisi selesai |
| `if std == 0` | Hindari pembagian nol: semua z-score diisi 0.0 |
| `df["zscore"].abs() > 2` | Data di luar ±2 simpangan baku ditandai anomali |

### 6.3 Hasil

| Aspek | Hasil |
|---|---|
| Koneksi | ESP8266 tersambung ke WiFi dan broker; Python menampilkan "Terhubung ke broker, kode: Success" |
| Akuisisi & CSV | 30 data diterima (progres `[n/30]`), tersimpan di `data_suhu.csv` beserta waktu terima |
| Statistik & anomali | Suhu umumnya 28-30 °C, satu titik melonjak ke sekitar 37-38 °C dan terdeteksi anomali (\|z\| > 2) |
| Grafik | `grafik_suhu.png`: garis suhu, garis rata-rata abu-abu putus-putus, satu titik merah anomali |
| Kesesuaian spesifikasi | Sesuai |



<img width="1494" height="693" alt="Hasil grafik " src="https://github.com/user-attachments/assets/4b1a975b-cc3b-4987-b718-fa2d5ae91a49" />

![Grafik Suhu dan Anomali]



## 7. Tugas Tambahan

### 7.1 Simpangan baku di ESP (5A No. 4)
```cpp
#include <math.h>

float hitungStdDev(float rataRata) {
  float jumlahKuadrat = 0;
  for (int i = 0; i < jumlah; i++) {
    jumlahKuadrat += pow(buffer[i] - rataRata, 2);
  }
  return sqrt(jumlahKuadrat / jumlah);
}

// di loop():
float suhuStdDev = hitungStdDev(suhuFilter);
Serial.print(" | StdDev: "); Serial.println(suhuStdDev);
```

| Baris | Penjelasan |
|---|---|
| `#include <math.h>` | Agar `pow()` dan `sqrt()` tersedia |
| `hitungStdDev(float rataRata)` | Hitung simpangan baku isi buffer dengan rata-rata (hasil moving average) sebagai acuan |
| `jumlahKuadrat += pow(buffer[i] - rataRata, 2)` | Akumulasi kuadrat selisih tiap data terhadap rata-rata |
| `return sqrt(jumlahKuadrat / jumlah)` | Akar dari rata-rata kuadrat selisih = simpangan baku |
| `Serial.print(" \| StdDev: ")` | Menampilkan hasil setelah nilai Max |


### 7.2 Moving average sebelum z-score (5B No. 4)
```python
# Tahap 2.5: filter moving average (jendela 5)
df["suhu_ma"] = df["suhu"].rolling(window=5, min_periods=1).mean()

# Tahap 3: anomali dari data mentah
rata2 = df["suhu"].mean()
std = df["suhu"].std()
df["zscore"] = 0.0 if std == 0 else (df["suhu"] - rata2) / std
df["anomali"] = df["zscore"].abs() > 2

# Tahap 3b: anomali dari data hasil moving average
rata2_ma = df["suhu_ma"].mean()
std_ma = df["suhu_ma"].std()
df["zscore_ma"] = 0.0 if std_ma == 0 else (df["suhu_ma"] - rata2_ma) / std_ma
df["anomali_ma"] = df["zscore_ma"].abs() > 2

print(f"Jumlah anomali (data mentah)            : {df['anomali'].sum()}")
print(f"Jumlah anomali (setelah moving average) : {df['anomali_ma'].sum()}")
# visualisasi: plot suhu mentah (alpha 0.4) + suhu_ma, simpan grafik_suhu_ma.png
```

| Baris | Penjelasan |
|---|---|
| `rolling(window=5, min_periods=1).mean()` | Rata-rata bergerak 5 data; `min_periods=1` agar baris awal tetap punya nilai |
| `rata2_ma`, `std_ma` | Rata-rata dan simpangan baku dari data yang sudah difilter |
| `zscore_ma`, `anomali_ma` | z-score dan penanda anomali dari data hasil filter |
| Dua `print` | Membandingkan jumlah anomali sebelum dan sesudah filter |

Secara umum, moving average meredam lonjakan satu titik sehingga jumlah anomali setelah filter cenderung berkurang (lonjakan "dibagi rata" ke titik di sekitarnya). Isi nilai aktual dari hasil percobaanmu di sini: `Mentah = ... , Setelah MA = ...`

## 8. Jawaban Pertanyaan Praktikum

### Percobaan 5A

**1) Flowchart `hitungMovingAverage()`**

```
Mulai
  -> Terima suhu baru (nilaiBaru)
  -> buffer[indeks] = nilaiBaru
  -> indeks = (indeks + 1) % N
  -> jumlah < N ?
       Ya    -> jumlah++
       Tidak -> lanjut
  -> total = 0
  -> Ulangi i = 0 .. (jumlah - 1): total += buffer[i]
  -> rata-rata = total / jumlah
  -> Kembalikan suhuFilter
Selesai
```

**2) Fungsi `(indeks + 1) % N`**
Menggeser posisi penulisan buffer secara melingkar (circular buffer). Indeks bertambah satu tiap data baru; saat mencapai N, hasil modulo membuatnya kembali ke 0, sehingga data tertua ditimpa data terbaru. Buffer selalu berisi N pembacaan terakhir tanpa menggeser seluruh array secara manual.

**3) Jika N diubah**
- N = 20: hasil lebih halus karena dirata-ratakan dari lebih banyak sampel, tetapi respons terhadap perubahan nyata jauh lebih lambat (lag besar).
- N = 2: filter lebih cepat mengikuti data mentah, tetapi efek penghalusan lemah sehingga hasil masih berfluktuasi mirip data mentah.

**4) Modifikasi simpangan baku:** lihat [7.1](#71-simpangan-baku-di-esp-5a-no-4).

### Percobaan 5B

**1) Fungsi blok `try-except` di `on_message()`**
Menangani payload yang bukan JSON valid (`ValueError`) atau tidak punya key `"suhu"` (`KeyError`). Tanpa blok ini, pesan tidak valid akan menyebabkan exception yang tidak tertangani sehingga program berhenti (crash) dan akuisisi 30 sampel tidak selesai.

**2) Makna z-score dan alasan batas \|z\| > 2**
Z-score menyatakan seberapa jauh data menyimpang dari rata-rata dalam satuan simpangan baku: `z = (x - rata-rata) / simpangan baku`. Pada data berdistribusi mendekati normal, sekitar 95% data berada dalam ±2 simpangan baku, sehingga data di luar rentang itu (sekitar 5%) cukup menyimpang untuk ditandai anomali tanpa terlalu banyak false positive.

**3) Kelemahan metode z-score**
Rata-rata dan simpangan baku dihitung dari seluruh data sekaligus (statis). Jika data punya tren naik/turun tajam, rata-rata keseluruhan tidak mencerminkan kondisi "normal" tiap titik waktu. Akibatnya data di awal/akhir tren yang wajar bisa salah dianggap anomali, sedangkan perubahan mendadak di tengah tren bisa tidak terdeteksi karena tertutup pergeseran rata-rata.

**4) Modifikasi moving average sebelum z-score:** lihat [7.2](#72-moving-average-sebelum-z-score-5b-no-4).

## 9. Analisis Keseluruhan

**1) Hasil tiap percobaan**
5A: ESP8266 menghitung moving average 5 data serta memperbarui min/max secara berkala, hasil filter lebih halus dari data mentah. 5B: Python mengakuisisi 30 data via MQTT, menyimpan ke CSV beserta waktu terima, menghitung statistik, mendeteksi anomali dengan z-score, dan membuat grafik.

**2) Pengolahan di perangkat (5A) vs komputer (5B)**

| Aspek | Edge (ESP8266) | Server/komputer (Python) |
|---|---|---|
| Kecepatan | Lebih cepat merespons, diolah langsung saat data dibaca | Harus menunggu data terkumpul lewat jaringan |
| Kompleksitas analisis | Sederhana (moving average, min/max) | Kompleks: statistik lengkap, z-score, visualisasi dengan pandas/matplotlib |
| Sumber daya | Memori dan komputasi sangat terbatas | Besar, mampu menyimpan data historis dan analisis berat |

**3) Pengaruh ukuran jendela moving average pada kualitas data dan kendali**
Jendela besar: data lebih halus dan tahan derau, tetapi respons lambat (lag), sehingga aktuator terlambat bereaksi. Jendela kecil: respons cepat, tetapi rentan derau sehingga aktuator bisa berubah status terlalu sering (chattering). Ukuran jendela perlu menyeimbangkan kehalusan dan kecepatan respons sesuai kebutuhan sistem.

**4) Pemanfaatan deteksi anomali sebagai peringatan dini**
- Smart farming: anomali suhu/kelembaban tanah memicu notifikasi ke petani saat kondisi ekstrem, atau menghentikan sementara pompa air agar tidak bekerja berdasarkan data keliru.
- Smart home: anomali suhu bisa menandakan kebakaran atau sensor rusak, sehingga sistem memberi peringatan atau tindakan pengamanan otomatis.

## 10. Kendala & Catatan
- ESP32 dan DHT22 tidak tersedia, diganti NodeMCU ESP8266 dan DHT11 (sesuai penyesuaian Modul I).
- Data suhu 5A dan 5B berasal dari `bacaSuhuMock()` karena pembacaan sensor kurang stabil, jadi pola dan anomali dibangkitkan program.
- Dokumentasi foto rangkaian dan Serial Monitor Percobaan 5A belum diambil dan akan dilengkapi kemudian.
- Selain itu, tidak ada kendala berarti.
