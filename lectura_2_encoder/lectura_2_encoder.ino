#include <Wire.h>

#define AS5600_ADDR 0x36

// Direcciones de registros
#define ZPOS_H       0x01
#define MPOS_H       0x03
#define RAW_ANGLE_H  0x0C
#define ANGLE_H      0x0E
#define STATUS       0x0B
#define AGC_REG      0x1A
#define MAGNITUDE_H  0x1B

// Pines de cada bus I2C
#define SDA_1 21
#define SCL_1 22
#define SDA_2 25
#define SCL_2 26

// ============================================================
// Funciones genéricas de lectura/escritura (por bus)
// ============================================================

void writeRegister16(TwoWire &bus, uint8_t regH, uint16_t value) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(regH);
  bus.write((value >> 8) & 0x0F);
  bus.write(value & 0xFF);
  bus.endTransmission();
}

uint16_t readRegister16(TwoWire &bus, uint8_t regH) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(regH);
  bus.endTransmission(false);
  bus.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (bus.available() < 2) return 0xFFFF;
  uint8_t hi = bus.read();
  uint8_t lo = bus.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF;
}

uint8_t readRegister8(TwoWire &bus, uint8_t reg) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(reg);
  bus.endTransmission(false);
  bus.requestFrom(AS5600_ADDR, (uint8_t)1);
  if (!bus.available()) return 0xFF;
  return bus.read();
}

// ============================================================
// Funciones de alto nivel (por bus)
// ============================================================

uint16_t readRawAngle(TwoWire &bus) { return readRegister16(bus, RAW_ANGLE_H); }
uint16_t readAngle(TwoWire &bus)    { return readRegister16(bus, ANGLE_H); }
uint16_t readMagnitude(TwoWire &bus){ return readRegister16(bus, MAGNITUDE_H); }
uint8_t  readAGC(TwoWire &bus)      { return readRegister8(bus, AGC_REG); }
uint8_t  readStatus(TwoWire &bus)   { return readRegister8(bus, STATUS); }

void configureRange(TwoWire &bus, uint16_t zpos, uint16_t mpos) {
  writeRegister16(bus, ZPOS_H, zpos);
  delay(1);
  writeRegister16(bus, MPOS_H, mpos);
  delay(1);
}

// ============================================================
// Programa principal
// ============================================================

void setup() {
  Serial.begin(115200);

  // Inicializar los dos buses
  // Wire.begin(SDA_1, SCL_1);
  // Wire.setClock(400000);
  Wire1.begin(SDA_2, SCL_2);
  Wire1.setClock(400000);

  delay(100);

  // configureRange(Wire, 0, 4096);
  configureRange(Wire1, 0, 4096);

  Serial.println("AS5600 dual I2C iniciado.");
}

void loop() {
  // === Sensor 1 ===
  // uint16_t raw1 = readRawAngle(Wire);
  // uint16_t angle1 = readAngle(Wire);
  // uint8_t status1 = readStatus(Wire);
  // uint8_t agc1 = readAGC(Wire);
  // uint16_t mag1 = readMagnitude(Wire);

  // bool MH1 = status1 & 0x20;
  // bool ML1 = status1 & 0x10;
  // bool MD1 = status1 & 0x08;

  // === Sensor 2 ===
  uint16_t raw2 = readRawAngle(Wire1);
  uint16_t angle2 = readAngle(Wire1);
  uint8_t status2 = readStatus(Wire1);
  uint8_t agc2 = readAGC(Wire1);
  uint16_t mag2 = readMagnitude(Wire1);

  bool MH2 = status2 & 0x20;
  bool ML2 = status2 & 0x10;
  bool MD2 = status2 & 0x08;

  // === Conversión a grados ===
  // float deg1 = angle1 * 360.0 / 4096.0;
  float deg2 = angle2 * 360.0 / 4096.0;

  // === Salida CSV: todos los datos en una línea ===
  // Serial.print(raw1); Serial.print(",");
  // Serial.print(angle1); Serial.print(",");
  // Serial.print(deg1, 2); Serial.print(",");
  // Serial.print(agc1); Serial.print(",");
  // Serial.print(mag1); Serial.print(",");
  // Serial.print(MD1); Serial.print(",");
  // Serial.print(ML1); Serial.print(",");
  // Serial.print(MH1); Serial.print(",");

  Serial.print(raw2); Serial.print(",");
  Serial.print(angle2); Serial.print(",");
  Serial.print(deg2, 2); Serial.print(",");
  Serial.print(agc2); Serial.print(",");
  Serial.print(mag2); Serial.print(",");
  Serial.print(MD2); Serial.print(",");
  Serial.print(ML2); Serial.print(",");
  Serial.println(MH2);

  delay(200);
}
