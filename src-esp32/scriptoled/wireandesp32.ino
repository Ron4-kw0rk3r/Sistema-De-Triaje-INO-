#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

#define ANCHO_PANTALLA 128 // Ancho de la pantalla OLED en píxeles
#define ALTO_PANTALLA 64   // Alto de la pantalla OLED en píxeles

// Declaración para la pantalla SSD1306 conectada mediante I2C
#define PIN_RESET -1         // Pin de reinicio (reset)
#define DIRECCION_PANTALLA 0x3C
Adafruit_SSD1306 pantalla(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, PIN_RESET);

// Declaración del sensor
MAX30105 sensorPulso;

const byte TAMANO_PROMEDIO = 4; // Aumenta esto para obtener un promedio más largo, 4 es bueno.
byte pulsos[TAMANO_PROMEDIO];    // Arreglo de pulsaciones
byte posicionPulso = 0;
long ultimoLatido = 0; // Tiempo en el que ocurrió el último latido
float latidosPorMinuto;
int promedioLatidos;



// Parámetros para detección de latidos
//const int UMBRAL_LATIDO = 1000; // Umbral de valor IR para detectar un latido
const int UMBRAL_LATIDO = 1000; // Umbral de valor IR para detectar un latido
const int UMBRAL_DEDOPUESTO = 200;
// Para la gráfica en el monitor serial
//SerialPlot plotter;
bool dedoColocado = false;


void setup() {
  Serial.begin(9600);

  // Inicializa el objeto de la pantalla OLED
  if (!pantalla.begin(SSD1306_SWITCHCAPVCC, DIRECCION_PANTALLA)) {
    Serial.println(F("Error al asignar memoria SSD1306"));
    for (;;) {
    }
  }

  // Inicializa el sensor MAX30105
  if (!sensorPulso.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 no encontrado."));
    for (;;) {
    }
  }

  // Configura el sensor
  sensorPulso.setup();
  sensorPulso.setPulseAmplitudeRed(0x0A);
  sensorPulso.setPulseAmplitudeIR(0x0A);

  pantalla.clearDisplay();
  pantalla.display();
  pantalla.setTextSize(3);
}



int calcularSpO2(long redAC, long irAC) {
  // Calcula la relación entre las amplitudes de las señales de luz roja e infrarroja
  float Rf = (float)redAC / (float)irAC;

  // Calcula la SpO2 utilizando la relación Rf y las longitudes de onda
  int SpO2 = 100 - 25 * Rf;

  // Aplica corrección según la longitud de onda
  const int IR_LED = 940; // Longitud de onda infrarroja (nm)
  const int RED_LED = 660; // Longitud de onda roja (nm)
  if (IR_LED == 940 && RED_LED == 660) {
    SpO2 = 110 - 25 * Rf;
  }

  // Limita la SpO2 dentro del rango [0, 100]
  if (SpO2 > 100) {
    SpO2 = 100;
  } else if (SpO2 < 0) {
    SpO2 = 0;
  }

  return SpO2;
}


void loop() {
  // Lee los valores del sensor
  long valorIR = sensorPulso.getIR();
  long valorRojo = sensorPulso.getRed();

  if (valorIR > UMBRAL_DEDOPUESTO) {
    dedoColocado = true;
  } else {
    dedoColocado = false;
  }

  if (dedoColocado) {
    if (valorIR > UMBRAL_LATIDO) {
      long delta = millis() - ultimoLatido;
      ultimoLatido = millis();

      latidosPorMinuto = 60.0 / (delta / 1000.0);

      if (latidosPorMinuto < 255 && latidosPorMinuto > 20) {
        pulsos[posicionPulso++] = (byte)latidosPorMinuto;
        posicionPulso %= TAMANO_PROMEDIO;

        promedioLatidos = 0;
        for (byte x = 0; x < TAMANO_PROMEDIO; x++) {
          promedioLatidos += pulsos[x];
        }
        promedioLatidos /= TAMANO_PROMEDIO;
      }
    }
  } else {
    pantalla.clearDisplay();
    pantalla.setTextSize(2);
    pantalla.setTextColor(SSD1306_WHITE);
    pantalla.setCursor(0, 0);
    pantalla.println("Coloca el dedo en el sensor");
    pantalla.display();
  }

  // Muestra los valores en la pantalla OLED
  //pantalla.clearDisplay();
  //pantalla.setTextSize(3);
  //pantalla.setTextColor(SSD1306_WHITE);
  //pantalla.setCursor(0, 0);

  // Evalúa la frecuencia cardíaca en el rango de 60 a 100 (sin redondeo) y muestra el estado correspondiente
  if (dedoColocado) {
    pantalla.clearDisplay();
    pantalla.setCursor(5, 0);
    pantalla.setTextSize(1);

    long redAC = sensorPulso.getRed();
    long irAC = sensorPulso.getIR();
    int spop = calcularSpO2(redAC, irAC);
    // Mostrar la frecuencia cardíaca y su estado
    if (promedioLatidos >= 0 && promedioLatidos <= 100) {
      pantalla.println("Frecuencia Cardiaca (BPM): ");
      pantalla.println("FreCard: " + String(promedioLatidos) + " BPM");
      pantalla.println("Estado: Normal");
      //pantalla.setTextColor(SSD1306_BLACK);
      pantalla.println("SpO2 :> " + String(spop)+ "%");
      //pantalla.setTextColor(SSD1306_WHITE);

    } else if (promedioLatidos > 100 && promedioLatidos <= 120) {
      pantalla.print("Frecuencia Cardiaca (BPM): ");
      pantalla.println("FreCard: " + String(promedioLatidos) + " BPM");
      pantalla.println("Estado: Media");
      //pantalla.setTextColor(SSD1306_BLACK);
      pantalla.println("SpO2 :> " + String(spop)+ "%");
      //pantalla.setTextColor(SSD1306_WHITE);
    } else {
      pantalla.print("Frecuencia Cardiaca (BPM): ");
      pantalla.println("FreCard: " + String(promedioLatidos) + " BPM");
      pantalla.println("Estado: Alta");
      //pantalla.setTextColor(SSD1306_BLACK);
      pantalla.println("SpO2 :> " + String(spop)+ "%");
      //pantalla.setTextColor(SSD1306_WHITE);
    }
  }
  pantalla.setTextColor(SSD1306_WHITE);
  pantalla.display();

  // Muestra los valores en el monitor serial
  //Serial.print(latidosPorMinuto);
  //Serial.print(", Valor IR: ");
  //Serial.print(valorIR);
  //Serial.print(", Valor Rojo: ");
  //Serial.println(valorRojo);

  //delay(1000); // Retardo de 1 segundo
}