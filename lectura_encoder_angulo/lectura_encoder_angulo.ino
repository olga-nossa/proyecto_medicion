#include <Wire.h>
#define AS5600_ADDR 0x36
#define ANGLE_H 0x0E
#define ANGLE_L 0x0F

// Pines I2C (ajusta si usas otros)
#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  delay(50);
}

// Lee ANGLE (12 bits). Devuelve 0xFFFF en caso de error I2C.
uint16_t readAngle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(ANGLE_H);
  Wire.endTransmission(false); // repeated start
  Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (Wire.available() < 2) return 0xFFFF; // error
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF; // 12 bits
}

void loop() {
  uint16_t angleCounts = readAngle();
  if (angleCounts == 0xFFFF) {
    Serial.println("I2C read error");
  } else {
    // float angleDeg = (float)angleCounts * 360.0f / 4096.0f;
    Serial.print("ANGLE counts: ");
    Serial.println(angleCounts);
    // Serial.print("  deg: ");
    // Serial.println(angleDeg, 3);
  }
  delay(50);
}
