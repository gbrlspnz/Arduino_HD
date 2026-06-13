#include <LiquidCrystal.h>

// Pines: RS=12, E=11, D4=5, D5=4, D6=3, D7=2
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int pinLedRojo = 8;
const int pinLedVerde = 9;
float temperatura = 0.0;
unsigned long ultimaPeticion = 0;

void setup() {
  Serial.begin(9600);
  pinMode(pinLedRojo, OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("Iniciando...");
  delay(1000);
  lcd.clear();
}

void loop() {
  // 1. Pedir temperatura al Esclavo cada 1 segundo
  if (millis() - ultimaPeticion >= 1000) {
    Serial.write(0xAA); // Enviar solicitud
    ultimaPeticion = millis();
  }

  // 2. Leer respuesta
  if (Serial.available() > 0) {
    temperatura = (float)Serial.read();
  }

  // 3. Mostrar en LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura);
  lcd.print(" C   ");

  // 4. Lógica de LED (Umbral 20°C)
  if (temperatura >= 20.0) {
    digitalWrite(pinLedRojo, HIGH);
    digitalWrite(pinLedVerde, LOW);
  } else {
    digitalWrite(pinLedRojo, LOW);
    digitalWrite(pinLedVerde, HIGH);
  }
}