#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ModbusMaster.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_task_wdt.h>

// ===== WiFi Configuration =====
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PWD  = "";

// ===== MQTT Configuration =====
const char *MQTT_BROKER = "your-broker.com";
const int MQTT_PORT = 8883;
const char *MQTT_USER = "username";
const char *MQTT_PASS = "password";
const char *MQTT_CLIENT_ID = "ESP32Client";

// Root CA certificate for HiveMQ Cloud
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// ===== Hardware Pins =====
#define RX_PIN 16
#define TX_PIN 17
#define DE_PIN 15
#define LED_PIN 2

// ===== Modbus and MQTT Objects =====
ModbusMaster node;
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);


struct ModbusParam {
  const char* name;    // Key in JSON
  uint16_t reg;        // Modbus register address
  uint8_t length;      // Number of registers
};

ModbusParam parameters[] = {
  {"frequency", 0x1000, 1},
  {"voltage",   0x1001, 1},
  {"current",   0x1002, 1},
  {"power",     0x1003, 1}
};
const uint8_t paramCount = sizeof(parameters) / sizeof(parameters[0]);

void initWatchdog() {
  esp_task_wdt_init(10, true);  // 10 sec timeout
  esp_task_wdt_add(NULL);
}

void setupModbus() {
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  pinMode(DE_PIN, OUTPUT);
  digitalWrite(DE_PIN, LOW);

  node.begin(1, Serial2);  // Slave ID 1
  node.preTransmission([]() { digitalWrite(DE_PIN, HIGH); });
  node.postTransmission([]() { digitalWrite(DE_PIN, LOW); });
}

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  digitalWrite(LED_PIN, HIGH);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void setupMQTT() {
  espClient.setCACert(root_ca);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setKeepAlive(30);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect(
          MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS,
          "vfd/status/availability", 1, true, "offline")) {
      Serial.println("connected.");
      mqttClient.publish("vfd/status/availability", "online", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" — retrying in 5s");
      delay(5000);
    }
  }
}

void publishAllModbusData() {
  if (time(nullptr) < 1000000000) {
    Serial.println("Time not synced yet. Skipping publish.");
    return;
  }

  StaticJsonDocument<256> doc;
  bool success = false;

  for (uint8_t i = 0; i < paramCount; i++) {
    ModbusParam param = parameters[i];
    uint8_t result = node.readHoldingRegisters(param.reg, param.length);
    if (result == node.ku8MBSuccess) {
      uint16_t value = node.getResponseBuffer(0);
      doc[param.name] = value;
      success = true;
    } else {
      Serial.printf("Failed to read %s (code %d)\n", param.name, result);
    }
  }

  if (success) {
    doc["timestamp"] = time(nullptr);
    doc["rssi"] = WiFi.RSSI();

    String payload;
    serializeJson(doc, payload);
    if (mqttClient.publish("vfd/status", payload.c_str())) {
      Serial.println("Published: " + payload);
      // Blink LED for feedback
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
    } else {
      Serial.println("MQTT publish failed.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  setupModbus();
  connectWiFi();
  setupMQTT();
  reconnectMQTT(); 
  initWatchdog();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 5000) {
    publishAllModbusData();
    lastUpdate = millis();
  }

  esp_task_wdt_reset();  // Feed watchdog
}

