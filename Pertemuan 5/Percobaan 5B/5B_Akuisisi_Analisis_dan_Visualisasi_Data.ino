#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>

// ================= WIFI =================
const char* WIFI_SSID = "Lula";
const char* WIFI_PASSWORD = "12345678910";

// ================= MQTT =================
const char* BROKER = "broker.hivemq.com";
const int PORT = 1883;
const char* TOPIC = "unsoed/tk245004/kelompok3/data";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= MOCK DATA SUHU =================
float bacaSuhuMock() {
  static unsigned long jumlahBaca = 0;
  jumlahBaca++;

  // Suhu dasar sekitar 26-30 derajat Celsius
  float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);

  // Fluktuasi suhu kecil
  float noise = random(-5, 6) * 0.1;

  float suhu = suhuDasar + noise;

  // Lonjakan suhu setiap 25 pembacaan
  if (jumlahBaca % 25 == 0) {
    suhu += 8.0;
  }

  return suhu;
}

// ================= WIFI =================
void hubungkanWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi terhubung!");
  Serial.print("IP ESP8266: ");
  Serial.println(WiFi.localIP());
}

// ================= MQTT =================
void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266-Kelompok3-";
    clientId += String(ESP.getChipId(), HEX);

    Serial.print("Menghubungkan ke MQTT...");

    if (client.connect(clientId.c_str())) {
      Serial.println("Berhasil!");
    } else {
      Serial.print("Gagal, kode: ");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));

  hubungkanWiFi();

  client.setServer(BROKER, PORT);

  Serial.println("Sistem Mock Suhu ESP8266 + MQTT");
  Serial.print("Topic: ");
  Serial.println(TOPIC);
}

// ================= LOOP =================
void loop() {
  // Periksa WiFi
  if (WiFi.status() != WL_CONNECTED) {
    hubungkanWiFi();
  }

  // Periksa koneksi MQTT
  if (!client.connected()) {
    hubungkanMQTT();
  }

  client.loop();

  // Membuat suhu simulasi
  float suhu = bacaSuhuMock();

  // Format JSON HARUS sesuai dengan Python
  char payload[40];

  snprintf(payload, sizeof(payload),
           "{\"suhu\":%.2f}", suhu);

  // Tampilkan dan publish data
  Serial.print("Data dikirim: ");
  Serial.println(payload);

  if (client.publish(TOPIC, payload)) {
    Serial.println("Publish berhasil!");
  } else {
    Serial.println("Publish gagal!");
  }

  delay(2000);
}