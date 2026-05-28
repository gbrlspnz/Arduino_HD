const int pinSensor = A0;
const int pinBoton = 2;

const int ledVerde = 7;
const int ledRojo = 6;

const int motorPWM = 9;
const int motorIN1 = 8;
const int motorIN2 = 4;

float umbral = 30.0;

bool modoManual = false;
bool ultimoEstadoBoton = HIGH;
bool estadoBotonEstable = HIGH;

unsigned long ultimoTiempoCambio = 0;
const unsigned long tiempoDebounce = 50;

void setup() {
  Serial.begin(9600);

  pinMode(pinBoton, INPUT_PULLUP);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);

  pinMode(motorPWM, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);

  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
}

void loop() {
  manejarBoton();

  int adc = analogRead(pinSensor);
  float voltaje = adc * 5.0 / 1023.0;

  // Para sensor TMP36 de Tinkercad
  float temperatura = (voltaje - 0.5) * 100.0;

  int pwm = calcularPWM(temperatura);

  analogWrite(motorPWM, pwm);
  // prender led depende de la temperatura
  if (pwm == 0) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledRojo, LOW);
  } else {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledRojo, HIGH);
  }

  Serial.print("ADC: ");
  Serial.print(adc);
  Serial.print(" | Temp: ");
  Serial.print(temperatura);
  Serial.print(" C | PWM: ");
  Serial.println(pwm);

  delay(300);
}

int calcularPWM(float temperatura) {
  if (modoManual) {
    return 255;
  }

  if (temperatura < umbral) {
    return 0;
  }

  if (temperatura < umbral) {
    return map(temperatura, 30, 40, 90, 180);
  }

return 255;
}

void manejarBoton() {
  bool lecturaActual = digitalRead(pinBoton);

  if (lecturaActual != ultimoEstadoBoton) {
    ultimoTiempoCambio = millis();

  }

  if ((millis() - ultimoTiempoCambio) > tiempoDebounce) {
    if (lecturaActual != estadoBotonEstable) {
      estadoBotonEstable = lecturaActual;

      if (estadoBotonEstable == LOW) {
        modoManual = !modoManual;

      }

    }

  }

  ultimoEstadoBoton = lecturaActual;

}