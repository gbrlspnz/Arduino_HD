#include <LiquidCrystal.h>

// Inicializamos la librería con nuestros pines: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int pinTemp = A0; 
float temperatura = 0.0;
unsigned long ultimaActualizacionLCD = 0;

void setup() {
  Serial.begin(9600); // UART para hablar con el Maestro
  
  // Configuramos las columnas y filas de la pantalla
  lcd.begin(16, 2); 
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
}

void loop() {
  // 1. Lectura del sensor de temperatura
  int lectura = analogRead(pinTemp);
  float voltaje = lectura * (5.0 / 1024.0);
  temperatura = (voltaje - 0.5) * 100.0;
  
  // 2. Refresco de pantalla (cada medio segundo)
  if (millis() - ultimaActualizacionLCD > 500) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatura);
    lcd.print(" C    "); // Espacios para borrar números anteriores
    ultimaActualizacionLCD = millis();
  }
  
  // 3. Responder al Maestro
  procesarUART();
}

void procesarUART() {
  while (Serial.available() >= 4) {
    if (Serial.peek() == 0xAA) {
      Serial.read(); // Quitar SOF
      byte id = Serial.read();
      byte len = Serial.read();
      byte chkRecibido = Serial.read();
      
      // Validar la petición del Maestro
      if ((id ^ len) == chkRecibido && id == 0x01) {
        byte tempByte = (byte)temperatura; 
        byte respuesta[] = {0xAA, 0x02, 0x01, tempByte, (byte)(0x02 ^ 0x01 ^ tempByte)};
        Serial.write(respuesta, 5);
      }
    } else {
      Serial.read(); // Limpiar el buffer de posibles ruidos
    }
  }
}