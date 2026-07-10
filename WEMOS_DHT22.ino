#include <DHT.h
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Konfigurasi DHT22
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char* ssid = "ISKANDARIAN";
const char* password = "LUPATERUS22"

// MQTT
const char* mqtt_server = "test.mosquitto.org";
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT broker... ");
    if (client.connect("wemosClient")) {
      Serial.println("BERHASIL!");
      client.publish("wemos/suhu", "Device Connected");
    } else {
      Serial.print("GAGAL, rc=");
      Serial.print(client.state());
      Serial.println(" | Coba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP STARTING...");
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (!isnan(suhu)) {
    String suhuStr = String(suhu, 2);
    client.publish("wemos/suhu", suhuStr.c_str());
    Serial.print("📡 Suhu dikirim: ");
    Serial.println(suhuStr + " °C");
  } else {
    Serial.println("⚠️ Gagal baca suhu!");
  }

  if (!isnan(kelembapan)) {
    String humStr = String(kelembapan, 2);
    client.publish("wemos/kelembapan", humStr.c_str());
    Serial.print("💧 Kelembapan dikirim: ");
    Serial.println(humStr + " %");
  } else {
    Serial.println("⚠️ Gagal baca kelembapan!");
  }

  delay(3000); // Update tiap 3 detik
}
