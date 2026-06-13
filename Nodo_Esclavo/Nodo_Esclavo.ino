const int pinTemp = A0; 

void setup() {
  Serial.begin(9600);
}

void loop() {
  // 1. Leer sensor
  int lectura = analogRead(pinTemp);
  float voltaje = lectura * (5.0 / 1024.0);
  // Cálculo preciso para TMP36
  float tempC = (voltaje - 0.5) * 100.0;
  
  // 2. Esperar petición del Maestro (Byte 0xAA)
  if (Serial.available() > 0) {
    if (Serial.read() == 0xAA) {
      // Enviar la temperatura al Maestro
      Serial.write((byte)tempC);
    }
  }
}