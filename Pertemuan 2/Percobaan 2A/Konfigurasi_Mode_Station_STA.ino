#include <ESP8266WiFi.h>

// Ganti sesuai WiFi/hotspot kamu
const char* ssid = "ssssss";
const char* password = "amandamaisentosa";

// LED indikator
const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);

  // GPIO 2 sebagai output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ESP32 sebagai Station
  WiFi.mode(WIFI_STA);

  // Mulai koneksi WiFi
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi berhasil terhubung!");

  // Menampilkan informasi jaringan
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());

  Serial.print("MAC Address : ");
  Serial.println(WiFi.macAddress());

  Serial.print("RSSI : ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // LED menyala jika berhasil terhubung
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
}