const int pinTemp = A0;

void setup() {
  Serial.begin(9600); 
}

void loop() {
  // PARCHE TINKERCAD: Forzamos la ejecución
  serialEvent(); 
}

void serialEvent() {
  while (Serial.available() >= 4) {
    if (Serial.peek() == 0xAA) {
      Serial.read(); 
      
      byte id = Serial.read();
      byte len = Serial.read();
      byte chkRecibido = Serial.read();
      
      byte chkCalculado = id ^ len; 
      
      if (chkCalculado == chkRecibido && id == 0x01) {
        enviarTemperatura();
      }
    } else {
      Serial.read(); 
    }
  }
}

void enviarTemperatura() {
  int lectura = analogRead(pinTemp);
  float voltaje = lectura * (5.0 / 1023.0);
  float temperatura = (voltaje - 0.5) * 100.0;
  
  byte tempByte = (byte)temperatura; 
  
  byte sof = 0xAA;
  byte id = 0x02;
  byte len = 0x01;
  
  // Separamos el cálculo del XOR
  byte chk = id ^ len;
  chk = chk ^ tempByte;
  
  Serial.write(sof);
  Serial.write(id);
  Serial.write(len);
  Serial.write(tempByte);
  Serial.write(chk);
}