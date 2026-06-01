const int pinSensor = A0;

const int pinBoton = 2;
const int ledVerde = 5; 
const int ledRojo = 6;  

// --- PINES DEL MOTOR (L293D) ---
const int motorPWM = 10;
const int motorIN1 = 9;
const int motorIN2 = 8;

// --- VARIABLES ---
float umbral = 15.0; // Temperatura base para que el motor empiece a girar y se realice un cambio de luz
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

  // El motor girará en un solo sentido
  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
}

void loop() {
  manejarBoton();

  // 1. Leer el sensor de temperatura
  int adc = analogRead(pinSensor);
  float voltaje = adc * 5.0 / 1023.0;
  float temperatura = (voltaje - 0.5) * 100.0; // Fórmula exclusiva del TMP36

  // 2. Calcular velocidad
  int pwm = calcularPWM(temperatura);
  analogWrite(motorPWM, pwm);

  // 3. Lógica del LED RGB
  if (pwm == 0) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledRojo, LOW);
  } else {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledRojo, HIGH);
  }

  // 4. Mostrar en el Monitor Serie
  Serial.print("Modo Manual: ");
  Serial.print(modoManual ? "SI" : "NO");
  Serial.print(" | Temp: ");
  Serial.print(temperatura);
  Serial.print(" C | PWM: ");
  Serial.println(pwm);

  delay(100);
}

int calcularPWM(float temperatura) {
  if (modoManual) {
    return 255; // Velocidad máxima si se aprieta el botón
  }
  if (temperatura < umbral) {
    return 0;   // Motor apagado si hace menos de 15 grados
  }
  if (temperatura < 40) {
    return map(temperatura, 15, 25, 90, 255); // Aumenta velocidad entre 15C y 25C
  }
  return 255; // Velocidad máxima si hace más de 25 grados
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