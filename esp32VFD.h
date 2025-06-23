#include <HardwareSerial.h>
#include <ModbusMaster.h>

#define RE_DE_PIN 4  // Control pin for RE/DE
#define VFD_ADDRESS 1  // Match the station number set on VFD

ModbusMaster node;
HardwareSerial SerialRS485(1);

void setup() {
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);  // Receive mode
  
  Serial.begin(115200);
  SerialRS485.begin(9600, SERIAL_8N1, 16, 17);
  
  node.begin(VFD_ADDRESS, SerialRS485);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  
  Serial.println("Initialized MODBUS communication with Veichi SI23");
}

void preTransmission() {
  digitalWrite(RE_DE_PIN, HIGH);  // Enable transmission
}

void postTransmission() {
  digitalWrite(RE_DE_PIN, LOW);  // Disable transmission
}

void loop() {
  // Example: Read output frequency (register 0x1001)
  uint8_t result = node.readHoldingRegisters(0x1001, 1);
  
  if (result == node.ku8MBSuccess) {
    float frequency = node.getResponseBuffer(0) / 100.0;
    Serial.print("Output Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");
  } else {
    Serial.println("Failed to read register");
  }
  
  // Example: Set frequency to 30 Hz (register 0x1000)
  // uint16_t speed = 30 * 100;  // 30.00 Hz
  // node.writeSingleRegister(0x1000, speed);
  
  delay(2000);
}
