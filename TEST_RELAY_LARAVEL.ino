#include <WiFi.h>
#include <PubSubClient.h>

// ===========================
// KONFIGURASI WIFI & MQTT
// ===========================
const char* ssid = "ISKANDARIAN";
const char* password = "LUPATERUS22";

// Broker publik Mosquitto
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

// Client ID unik (pastikan beda dari Laravel)
const char* client_id = "ESP32_DIPDOP";

// ===========================
// PIN RELAY
// ===========================
#define RELAY_PIN 27   // ubah sesuai pin relay lo
#define RELAY_ACTIVE_HIGH true  // ganti ke false kalau relay aktif LOW

WiFiClient espClient;
PubSubClient client(espClient);

// ===========================
// FUNGSI SETUP WIFI
// ===========================
void setup_wifi() {
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi terkoneksi ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ===========================
// CALLBACK MQTT (PESAN MASUK)
// ===========================
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Pesan diterima di topic [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Pisahkan perintah setelah tanda ':'
  int separatorIndex = message.indexOf(':');
  String command = (separatorIndex != -1) ? message.substring(separatorIndex + 1) : message;
  command.trim();

  // Kontrol relay sesuai pesan
  if (command.equalsIgnoreCase("ON")) {
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? HIGH : LOW);
    Serial.println("Relay ON ✅");
    if (client.publish("relay/status", "relay1:ON", false))
      Serial.println("Status terkirim ke broker: relay1:ON");
    else
      Serial.println("⚠️ Gagal kirim status MQTT");
  } 
  else if (command.equalsIgnoreCase("OFF")) {
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH);
    Serial.println("Relay OFF ❌");
    if (client.publish("relay/status", "relay1:OFF", false))
      Serial.println("Status terkirim ke broker: relay1:OFF");
    else
      Serial.println("⚠️ Gagal kirim status MQTT");
  }
}

// ===========================
// RECONNECT MQTT
// ===========================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT broker... ");
    if (client.connect(client_id)) {
      Serial.println("Berhasil terhubung ke broker MQTT! 🛰️");
      client.subscribe("relay/control");
      Serial.println("Subscribe ke topic: relay/control");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" | Coba lagi dalam 2 detik...");
      delay(2000);
    }
  }
}

// ===========================
// SETUP
// ===========================
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH); // default mati

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(15);   // bikin ping MQTT lebih sering = respon cepat
  client.setBufferSize(512); // jaga-jaga kalau pesan panjang
}

// ===========================
// LOOP
// ===========================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(10);  // mini delay biar CPU gak full 100%
}
