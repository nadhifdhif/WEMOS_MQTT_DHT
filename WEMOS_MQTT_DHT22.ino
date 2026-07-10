#include <ESP8266WiFi.h> // Wemos pakai ini
#include <PubSubClient.h>
#include <DHT.h>

// --- Konfigurasi WiFi ---
const char* ssid = "ISKANDARIAN";
const char* password = "LUPATERUS22";

// --- Konfigurasi MQTT ---
const char* mqtt_server = "test.mosquitto.org";
WiFiClient espClient;
PubSubClient client(espClient);

// --- Konfigurasi DHT ---
#define DHTPIN D4 // Pin D4 di Wemos (pin fisik 2)
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Fungsi koneksi WiFi ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi tersambung");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- Fungsi reconnect MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "WemosClient-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Tersambung ke MQTT Broker");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

// --- Setup utama ---
void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  randomSeed(micros());
}

// --- Loop utama ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Gagal membaca data dari DHT22!");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.print("°C | Kelembapan: ");
  Serial.print(humidity);
  Serial.println("%");

  String payload = "{";
  payload += "\"temperature\":";
  payload += String(temperature, 2);
  payload += ",\"humidity\":";
  payload += String(humidity, 2);
  payload += "}";

  client.publish("nadhif/LOS_WEMOS_ARDUINOS/data", payload.c_str());

  delay(3000);
}
