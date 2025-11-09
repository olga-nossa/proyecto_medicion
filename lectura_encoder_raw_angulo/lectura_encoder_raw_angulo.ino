#include <Wire.h>

#define AS5600_ADDR 0x36

// Direcciones de registros (Para escribir en ellos)
#define ZPOS_H       0x01 // configurar el valor inicial del rango de medida
#define MPOS_H       0x03 // configurar el valor final del rango de medida
#define MANG_H       0x05 // configurar el rango maximo de medida, se puede usar este o MPOS_H
#define CONF_H       0x07
#define RAW_ANGLE_H  0x0C // leer el valor del angulo crudo (sin filtrar ni escalar)
#define ANGLE_H      0x0E // leer el angulo filtrado y escalado

#define I2C_SDA 21
#define I2C_SCL 22

// ============================================================
// Funciones básicas de lectura/escritura por I2C
// ============================================================

// Escritura de un valor de 16 bits (12 bits efectivos en este sensor)
void writeRegister16(uint8_t regH, uint16_t value) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(regH);
  Wire.write((value >> 8) & 0x0F); // parte alta (solo 12 bits válidos)
  Wire.write(value & 0xFF);        // parte baja
  Wire.endTransmission();
}

// Lectura de un valor de 16 bits (devuelve 12 bits útiles)
uint16_t readRegister16(uint8_t regH) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(regH);
  Wire.endTransmission(false); // repeated start
  Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (Wire.available() < 2) return 0xFFFF; // error
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF;
}

// ============================================================
// Funciones de alto nivel
// ============================================================

// Leer ángulo crudo
uint16_t readRawAngle() {
  return readRegister16(RAW_ANGLE_H);
}

// Leer ángulo procesado (escalado)
uint16_t readAngle() {
  return readRegister16(ANGLE_H);
}

// Configurar temporalmente rango angular
void configureRange(uint16_t zpos, uint16_t mpos) {
  writeRegister16(ZPOS_H, zpos); // posición inicial
  delay(1);
  writeRegister16(MPOS_H, mpos); // posición final
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

  Serial.println("Configurando rango 0° a 90°...");
  configureRange(700, 2800); // 0°–90° (0, 1024) (4096 * 90/360) Se debe escribir el valor de RAWANGLE para el valor inicial y final del rango
  Serial.println("Configuración temporal aplicada.");
}

void loop() {
  uint16_t raw = readRawAngle();
  uint16_t angle = readAngle();

  if (raw == 0xFFFF) {
    Serial.println("I2C read error");
    return;
  }

  float degRaw   = raw   * 360.0 / 4096.0;
  float degAngle = angle * 180.0 / 4096.0;

  Serial.print("RAW: ");
  Serial.print(raw);
  Serial.print(" (");
  Serial.print(degRaw, 2);
  Serial.print("°)\t");

  Serial.print("ANGLE: ");
  Serial.print(angle);
  Serial.print(" (");
  Serial.print(degAngle, 2);
  Serial.println("°)");

  delay(100);
}
