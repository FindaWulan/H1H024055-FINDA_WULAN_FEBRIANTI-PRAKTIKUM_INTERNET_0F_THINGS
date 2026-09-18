#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

const char* ssid = "Oppo A31";
const char* password = "12345678";
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

    // Membuat koneksi HTTPS
    std::unique_ptr<BearSSL::WiFiClientSecure> client(
      new BearSSL::WiFiClientSecure
    );

    // Mengabaikan sertifikat SSL
    client->setInsecure();

    HTTPClient http;

    // Memulai koneksi ke server
    if (http.begin(*client, serverUrl)) {

      http.addHeader("Content-Type", "application/json");

      // Membuat objek data sensor dalam format JSON
      JsonDocument doc;
      doc["suhu"] = 28.5;
      doc["kelembaban"] = 65.0;

      String requestBody;
      serializeJson(doc, requestBody);

      Serial.print("Mengirim data: ");
      Serial.println(requestBody);

      // Mengirim data melalui HTTP POST
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

  delay(10000); // kirim data setiap 10 detik
}