#include <SFE_BMP180.h>
#include <Wire.h>
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//#include <LiquidCrystal_I2C.h>


//LiquidCrystal_I2C lcd(0x27, 20, 4);
const int PulseWire = 0;
int Threshold = 550;
int x = 0;
bool primeraMedidaTomada = false;
unsigned long tiempoInicio = 0;
const unsigned long tiempoEspera = 3000;  // Tiempo de espera en milisegundos (3 segundos)

float presionAnterior = 0.0;
int repeticiones = 0;
bool tomandoMediciones = false;

SFE_BMP180 bmp180;
PulseSensorPlayground pulseSensor;

void setup()
{
  Serial.begin(9600);
  pulseSensor.analogInput(PulseWire);
  pulseSensor.setThreshold(Threshold);
 // lcd.init();
  //lcd.backlight();
 // lcd.clear();
 // lcd.setCursor(0, 0);

  if (bmp180.begin())
  {
    Serial.println("Presión y Temperatura externa iniciados con éxito!");
  }
  if (pulseSensor.begin())
  {
    Serial.println("BPM Iniciado con éxito!");
  }
  else
  {
    Serial.println("Error al iniciar");
    while (1); // bucle infinito
  }
  mlx.begin();
 
}

void loop()
{
  char status;
  double T, P, y;
  unsigned long tiempoActual = millis();  // Obtener el tiempo actual en milisegundos

  status = bmp180.startTemperature(); // Inicio de lectura de temperatura
  if (status != 0)
  {
    delay(status); // Pausa para que finalice la lectura
    status = bmp180.getTemperature(T); // Obtener la temperatura
    if (status != 0)
    {
      status = bmp180.startPressure(3); // Inicio lectura de presión
      if (status != 0)
      {
        delay(status); // Pausa para que finalice la lectura
        status = bmp180.getPressure(P, T); // Obtenemos la presión
        y = (P * 0.750062) - 479;
        int z = int(y);
        if (status != 0)
        {
          //Serial.print("Temperatura: ");
          //Serial.print(T, 2);
          //Serial.print(" *C , ");
          Serial.print("Presión Sistólica: ");
          Serial.print(z);
          Serial.println(" mmHg");
          /*
          lcd.setCursor(0, 0);
          lcd.print("Presion Systolica:  ");
          lcd.print(z);
          lcd.print("[mmHg]");
          */

          if (!tomandoMediciones)
          {
            tomandoMediciones = true;
            presionAnterior = z; // Guardar el valor actual de la presión
            repeticiones = 0; // Reiniciar el contador de repeticiones
          }

          if (tomandoMediciones)
          {
            if (z == presionAnterior)
            {
              repeticiones++;
            }
            else
            {
              repeticiones = 0;
            }

            if (repeticiones > 3)
            {
              Serial.println("Resultado de la presión: ");
              Serial.println(presionAnterior);
            //  lcd.setCursor(0, 0);
            //  lcd.print("Presion Systolica:  ");
            //  lcd.print(presionAnterior);
            //  lcd.print("[mmHg]");
              
              tomandoMediciones = false;
                delay(tiempoEspera);
            }
          }

          presionAnterior = z; // Actualizar el valor anterior de la presión
        }
      }
    }
  }
// Pausa por el tiempo de espera antes de reanudar las mediciones
delay(1000);
float tempObjeto = mlx.readObjectTempC();
   
   if (tempObjeto >= 33 && tempObjeto < 36) {
    // La temperatura está entre 34 y 35 grados, mostrar "frio".
    Serial.print(" Temperatura = "); Serial.print(mlx.readObjectTempC()); Serial.println(" °C");
    Serial.println("Hipotermia");
  } else if (tempObjeto >= 36 && tempObjeto < 37.5) {
    // La temperatura está entre 35 y 36.5 grados, mostrar "caliente".
    Serial.print(" Temperatura = "); Serial.print(mlx.readObjectTempC()); Serial.println(" °C");
    Serial.println("Normal");
  } else if (tempObjeto >= 37.5 && tempObjeto < 39.5) {
    // La temperatura está entre 35 y 36 grados, mostrar "caliente".
    Serial.print(" Temperatura = "); Serial.print(mlx.readObjectTempC()); Serial.println(" °C");
    Serial.println("Fiebre");
  } else if (tempObjeto >= 39.5 && tempObjeto < 41) {
    // La temperatura está entre 35 y 36 grados, mostrar "caliente".
    Serial.print(" Temperatura = "); Serial.print(mlx.readObjectTempC()); Serial.println(" °C");
    Serial.println("Fiebre Alta");
  }
  //Serial.println(mlx.readObjectTempC());
  //delay(1000);
}
