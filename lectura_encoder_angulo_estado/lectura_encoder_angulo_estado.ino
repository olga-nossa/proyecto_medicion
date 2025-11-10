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

#define I2C_SDA 21 // 21 y 25
#define I2C_SCL 22 // 22 y 26

// ============================================================
// Funciones básicas
// ============================================================

void writeRegister16(uint8_t regH, uint16_t value) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(regH);
  Wire.write((value >> 8) & 0x0F);
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

uint16_t readRegister16(uint8_t regH) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(regH);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (Wire.available() < 2) return 0xFFFF;
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF;
}

uint8_t readRegister8(uint8_t reg) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, (uint8_t)1);
  if (!Wire.available()) return 0xFF;
  return Wire.read();
}

// ============================================================
// Funciones de alto nivel
// ============================================================

uint16_t readRawAngle() { return readRegister16(RAW_ANGLE_H); }
uint16_t readAngle()    { return readRegister16(ANGLE_H); }
uint16_t readMagnitude(){ return readRegister16(MAGNITUDE_H); }
uint8_t  readAGC()      { return readRegister8(AGC_REG); }
uint8_t  readStatus()   { return readRegister8(STATUS); }

void configureRange(uint16_t zpos, uint16_t mpos) {
  writeRegister16(ZPOS_H, zpos);
  delay(1);
  writeRegister16(MPOS_H, mpos);
  delay(1);
}

// ============================================================
// Programa principal
// ============================================================

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  delay(100);

  configureRange(0, 4096); // Rango 0°– 360° revisar el valor de raw angle

  Serial.println("AS5600 iniciado.");
}

void loop() {
  uint16_t raw = readRawAngle();
  uint16_t angle = readAngle();
  uint8_t status = readStatus();
  uint8_t agc = readAGC();
  uint16_t mag = readMagnitude();

  // Decodificar bits de STATUS
  bool MH = status & 0x20;
  bool ML = status & 0x10;
  bool MD = status & 0x08;

  // calcular el valor del angulo
  float degRaw   = raw   * 360.0 / 4096.0;
  float degAngle = angle * 360.0 / 4096.0;

  Serial.print("RAW: "); Serial.print(raw);
  Serial.print("  RAW angle: "); Serial.print(degRaw);

  Serial.print(" | ANGLE: "); Serial.print(angle);
  Serial.print("  angle:  "); Serial.print(degAngle);

  Serial.print(" | AGC: "); Serial.print(agc); //indica la ganancia, debe estar en el medio del rango ≈ 128
  Serial.print(" | MAG: "); Serial.print(mag); // magnitud del campo magnetico

  Serial.print(" | STATUS -> ");
  Serial.print("MD="); Serial.print(MD); //  Magnet was detected (1) sino es (0), no esta reaccionando
  Serial.print(" ML="); Serial.print(ML); // AGC maximum gain overflow, magnet too weak (1) sino es (0)
  Serial.print(" MH="); Serial.println(MH); // AGC minimum gain overflow, magnet too strong (1) sino es (0)

  delay(200);
}
