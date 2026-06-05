Este proyecto implementa un sistema automatizado en Arduino para el control de un motor de corriente continua (DC) basado en la temperatura ambiente. El sistema utiliza un sensor de temperatura analógico para determinar cuándo activar la ventilación y ajusta la velocidad del motor de forma proporcional al calor detectado mediante modulación por ancho de pulso (PWM).

El hardware incluye un puente H (TB6612FNG) para aislar la etapa de potencia del motor de la lógica del Arduino, un LED RGB que proporciona retroalimentación visual del estado de operación (reposo o activo), y un botón de anulación manual que permite al usuario encender el sistema independientemente de la temperatura leída.

Esquema de Conexiones
A continuación, se detalla el mapeo exacto de los pines entre la placa Arduino, el controlador de motores y los periféricos.

1.- Alimentación principal
Origen (Arduino),Destino (Protoboard),Descripción
5V,Línea Roja (+),Alimentación lógica y de potencia (VCC/VM)
GND,Línea Azul/Negra (-),Tierra común del sistema

2.-Sensor de Temperatura TMP36
Pin del Sensor,Conexión
Izquierda (Frente),5V (Línea Roja)
Centro (Señal),Arduino A0
Derecha,GND (Línea Azul/Negra)

3.- Entrada y Salida de interfaz
Componente,Conexión,Notas
Botón (Manual),Arduino 2 y GND,Configurado mediante código como INPUT_PULLUP.
LED Pata Verde,Arduino 5,Conectado a través de una resistencia limitadora.
LED Pata Roja,Arduino 6,Conectado a través de una resistencia limitadora.
LED Cátodo,GND (Línea Azul/Negra),Pata más larga del LED RGB.

4.-Controlador de motores
Pin TB6612FNG,Conexión,Función
VM,5V (Línea Roja),Alimentación de potencia para el motor
VCC,5V (Línea Roja),Alimentación lógica del chip
GND (Todos),GND (Línea Azul/Negra),Tierra común
STBY,5V (Línea Roja),Desactiva el modo suspensión (Standby)
PWMA,Arduino 11,Señal PWM (Control de velocidad)
AIN1,Arduino 9,Control de dirección (Input 1)
AIN2,Arduino 8,Control de dirección (Input 2)
A01,Cable Rojo del Motor,Salida de potencia al motor
A02,Cable Negro del Motor,Salida de potencia al motor
