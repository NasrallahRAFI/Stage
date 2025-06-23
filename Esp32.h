#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WiFi credentials
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PWD  = "";

// MQTT Broker settings
const char *MQTT_BROKER   = "508205b2e19c4a7fad9828d3961d6424.s1.eu.hivemq.cloud";
const int   MQTT_PORT     = 8883;
const char *MQTT_USERNAME = "Device0001";
const char *MQTT_PASSWORD = "Aa12345678";

// Status LED
#define STATUS_LED 2

// MQTT Topics
struct {
  const char* connection    = "vfd/conection";
  const char* temperature   = "vfd/temperature";
  const char* speed         = "vfd/speed";
  const char* tensionIN     = "vfd/tensionIN";
  const char* courantIN     = "vfd/courantIN";
  const char* courantOUT    = "vfd/courantOUT";
  const char* niveauEau     = "vfd/niveau_eau";
  const char* alertes       = "vfd/alertes";
  const char* etatReseau    = "vfd/etat_reseau";
  const char* misesAJour    = "vfd/mises_a_jour";
  const char* speedInput    = "vfd/Speedinput";
  const char* status        = "vfd/status";
  const char* schedule      = "vfd/schedule";
} topics;

// Root CA certificate for HiveMQ Cloud
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// VFD state
int currentSpeed = 0;
String vfdStatus = "OFF";
unsigned long lastSensorUpdate = 0;
const long sensorInterval = 5000;

void setup_wifi() {
  Serial.begin(115200);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt++ < 30) {
    delay(500);
    Serial.print(".");
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(STATUS_LED, HIGH);
  } else {
    Serial.println("\nWiFi failed, restarting...");
    ESP.restart();
  }
}

void publishSensorData() {
  char payload[16];

  dtostrf(random(200, 350) / 10.0, 4, 1, payload); // temperature
  client.publish(topics.temperature, payload);

  dtostrf(random(210, 250), 3, 0, payload); // tension IN
  client.publish(topics.tensionIN, payload);

  dtostrf(random(5, 20) / 10.0, 3, 1, payload); // courant IN
  client.publish(topics.courantIN, payload);

  dtostrf(random(10, 30) / 10.0, 3, 1, payload); // courant OUT
  client.publish(topics.courantOUT, payload);

  itoa(random(0, 100), payload, 10); // niveau eau
  client.publish(topics.niveauEau, payload);

  const char* networkStates[] = {"STABLE", "UNSTABLE", "LOW_VOLTAGE"};
  client.publish(topics.etatReseau, networkStates[random(0, 3)]);

  if (random(0, 10) > 7) {
    const char* alerts[] = {"OVERTEMP", "LOW_WATER", "VOLTAGE_FLUCTUATION"};
    client.publish(topics.alertes, alerts[random(0, 3)]);
  }

  Serial.println("Sensor data published");
}

void handleSpeedInput(String value) {
  int speed = value.toInt();
  if (speed >= 0 && speed <= 100) {
    currentSpeed = speed;
    char msg[8];
    sprintf(msg, "%d", speed);
    client.publish(topics.speed, msg);
    Serial.printf("Speed set: %d%%\n", speed);
  } else {
    Serial.println("Invalid speed input");
  }
}

void handleStatusCommand(String value) {
  value.toUpperCase();
  if (value == "ON" || value == "OFF" || value == "AUTO") {
    vfdStatus = value;
    client.publish(topics.status, vfdStatus.c_str());
    Serial.printf("Status updated: %s\n", vfdStatus.c_str());
  } else {
    Serial.println("Invalid status command");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String msg = String((char*)payload);

  Serial.printf("MQTT Message [%s]: %s\n", topic, msg.c_str());

  if (strcmp(topic, topics.speedInput) == 0)
    handleSpeedInput(msg);
  else if (strcmp(topic, topics.status) == 0)
    handleStatusCommand(msg);
  else if (strcmp(topic, topics.schedule) == 0)
    Serial.printf("Schedule command: %s\n", msg.c_str());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      digitalWrite(STATUS_LED, HIGH);

      client.subscribe(topics.speedInput);
      client.subscribe(topics.status);
      client.subscribe(topics.schedule);

      client.publish(topics.connection, "ESP32 CONNECTED");
      client.publish(topics.status, vfdStatus.c_str());
      client.publish(topics.speed, "0");
    } else {
      Serial.printf("Failed, rc=%d. Retrying...\n", client.state());
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  espClient.setCACert(root_ca);
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
  randomSeed(analogRead(0));
}

void loop() {
  if (!client.connected())
    reconnect();
    
  client.loop();

  if (millis() - lastSensorUpdate > sensorInterval) {
    publishSensorData();
    lastSensorUpdate = millis();
  }
}
 
