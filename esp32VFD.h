#include <HardwareSerial.h>
#include <ModbusMaster.h>

#define RE_DE_PIN 4      // MAX485 DE/RE control pin
#define VFD_ADDRESS 1    // MODBUS address of VFD (set in F0.03)

ModbusMaster node;
HardwareSerial SerialRS485(1);  // UART1 (RX=16, TX=17)

// VFD Register Map (Veichi SI23)
typedef struct {
  const char* name;      // Parameter name
  uint16_t address;      // MODBUS register address
  float scale;           // Scaling factor
  const char* unit;      // Measurement unit
} VfdRegister;

VfdRegister vfdRegisters[] = {
  {"Temperature", 0x100D, 0.1, "°C"},      // Temperature
  {"Speed", 0x1001, 0.01, "Hz"},           // Output speed
  {"Status", 0x2001, 1.0, ""},             // Status code
  {"Input Voltage", 0x1005, 0.1, "V"},     // Input voltage
  {"Input Current", 0x1006, 0.01, "A"},    // Input current
  {"Output Current", 0x1003, 0.01, "A"},   // Output current
  {"Alarms", 0x2002, 1.0, ""},             // Alarm codes
  {"Network Status", 0x1007, 1.0, ""},     // Network status
  {"Speed Setpoint", 0x1000, 0.01, "Hz"}   // Speed input
};

void preTransmission() {
  digitalWrite(RE_DE_PIN, HIGH);  // Enable MAX485 transmission
}

void postTransmission() {
  digitalWrite(RE_DE_PIN, LOW);   // Disable transmission (return to receive mode)
}

void setup() {
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);   // Start in receive mode

  Serial.begin(115200);           // Debug serial (USB)
  SerialRS485.begin(9600, SERIAL_8N1, 16, 17);  // RS485 at 9600 baud (8N1)

  node.begin(VFD_ADDRESS, SerialRS485);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("\nInitialized MODBUS communication with Veichi SI23");
  Serial.println("-----------------------------------------------");
}

void loop() {
  for (int i = 0; i < sizeof(vfdRegisters) / sizeof(vfdRegisters[0]); i++) {
    uint8_t result = node.readHoldingRegisters(vfdRegisters[i].address, 1);
    
    if (result == node.ku8MBSuccess) {
      float value = node.getResponseBuffer(0) * vfdRegisters[i].scale;
      
      Serial.print(vfdRegisters[i].name);
      Serial.print(": ");
      Serial.print(value);
      Serial.print(" ");
      Serial.println(vfdRegisters[i].unit);
    } else {
      Serial.print("Failed to read ");
      Serial.println(vfdRegisters[i].name);
    }
    delay(500);  // Small delay between readings
  }
  Serial.println("-----------------------------------------------");
  delay(2000);  // Wait 2 seconds before next read cycle
}
