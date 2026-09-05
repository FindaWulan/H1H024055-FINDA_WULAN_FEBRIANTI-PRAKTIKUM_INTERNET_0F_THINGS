#include <DHT.h>

#define DHTPIN 4       // pin data DHT11 terhubung ke GPIO 4
#define DHTTYPE DHT11  // tipe sensor yang digunakan
#define RELAYPIN 5    // pin kendali relay/LED indikator

DHT dht(DHTPIN, DHTTYPE);

const float suhuThreshold = 30.0; // ambang batas suhu (°C)

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW); // pastikan aktuator mati di awal
}

void loop() {
  float suhu = dht.readTemperature();

  if (isnan(suhu)) {
    Serial.println("Gagal membaca data sensor!");
  } 
  else {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C -> ");

    // Kendali aktuator berdasarkan hasil pembacaan suhu
    if (suhu > suhuThreshold) {
      digitalWrite(RELAYPIN, HIGH); // aktifkan relay/LED
      Serial.println("Aktuator: ON");
    } 
    else {
      digitalWrite(RELAYPIN, LOW); // matikan relay/LED
      Serial.println("Aktuator: OFF");
    }
  }

  delay(2000); // pembacaan setiap 2 detik
}