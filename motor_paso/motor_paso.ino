#include <Stepper.h>

// Número de pasos por revolución (ajústalo a tu motor)
const int stepsPerRevolution = 200;

// Pines de conexión al L298N (igual que en tu ejemplo)
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

void setup() {
  myStepper.setSpeed(100);  // Velocidad en RPM
  Serial.begin(9600);
  Serial.println("=== Control de motor paso a paso (L298N + NEMA 17 6 cables) ===");
  Serial.println("Comandos disponibles:");
  Serial.println("  A  = avanzar 1 paso");
  Serial.println("  R  = retroceder 1 paso");
  Serial.println("  A10 = avanzar 10 pasos");
  Serial.println("  R20 = retroceder 20 pasos");
  Serial.println("---------------------------------------------------------------");
}

void loop() {
  // Espera hasta recibir un comando completo (terminado en Enter)
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');  // Leer hasta Enter
    comando.trim();  // Elimina espacios o saltos de línea

    if (comando.length() == 0) return; // si está vacío, no hace nada

    char dir = comando.charAt(0);  // Primer carácter: dirección
    int pasos = 1;                 // Valor por defecto

    // Si hay números después de la letra, los convierte a entero
    if (comando.length() > 1) {
      pasos = comando.substring(1).toInt();
      if (pasos <= 0) pasos = 1;
    }

    // Ejecuta la acción
    if (dir == 'A' || dir == 'a') {
      myStepper.step(pasos);
      Serial.print("→ Avance de ");
      Serial.print(pasos);
      Serial.println(" pasos");
    }
    else if (dir == 'R' || dir == 'r') {
      myStepper.step(-pasos);
      Serial.print("← Retroceso de ");
      Serial.print(pasos);
      Serial.println(" pasos");
    }
    else {
      Serial.println("Comando no reconocido. Usa A o R seguido de un número opcional.");
    }
  }
}
