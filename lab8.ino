//Tech Trends Shameer
//Control LED Using Blynk 2.0/Blynk IOT

#define BLYNK_TEMPLATE_ID "TMPL2vJrMkueN"
#define BLYNK_TEMPLATE_NAME "CONTROL AC"
#define BLYNK_AUTH_TOKEN "jUNhM4JEYoOkHe99SaCgygyW2mHUM3rN"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>  
#include <BlynkSimpleEsp8266.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h> // Librería BMP280

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "CLARO1_F4A959";  // Wifi Username
char pass[] = "21938DEKzc";  // Wifi password

const int ledpin = 15; // D8
const int pulsador_on_off = 14; // D5
const int pulsador_modo = 12; // D6
bool ac_on = false;
bool ac_allowed = false; 
int modo = 0;

// Sensor BMP280 (I2C)
Adafruit_BMP280 bmp; // default I2C address 0x76 

// Parámetros del control por temperatura
float THRESHOLD_C = 29;    // umbral definido por el diseñador (puedes cambiarlo)
const float HYSTERESIS = 0.5; // evita oscilaciones: apagará si baja THRESHOLD - HYSTERESIS

// Temporización lectura sensor
unsigned long lastTempMillis = 0;
const unsigned long TEMP_INTERVAL_MS = 2000; // cada 2 s

void setup()
{     
  Serial.begin(115200);   
  //delay(2000);
  pinMode(ledpin,OUTPUT);
  pinMode(pulsador_on_off,INPUT_PULLUP);
  pinMode(pulsador_modo,INPUT_PULLUP);
  
  
  // Inicializar BMP280
  if (!bmp.begin(0x76)) { // prueba dirección 0x76; si falla, intenta 0x77
    Serial.println("No se encontró BMP280 en 0x76, intentando 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("ERROR: BMP280 no detectado. Revisa conexiones.");
      // No salir: dejamos el programa corriendo pero la lectura de temp quedará inactiva.
    } else {
      Serial.println("BMP280 detectado en 0x77.");
    }
  } else {
    Serial.println("BMP280 inicializado correctamente en 0x76.");
  }

  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V5, "BAJO (20%)");
  Blynk.virtualWrite(V6, 0); 
}


//--------------------------------------------------------------------------

void actualizarBloqueoPorTemperatura(float temperatura) {
  // Calcula ac_allowed con histeresis
  static bool previamente_permitido = false;

  if (isnan(temperatura)) {
    // Si no hay lectura válida, conservamos el estado previo y bloqueamos
    ac_allowed = false;
    return;
  }

  // Si antes estaba permitido, sólo mantenemos permitido hasta que baje por debajo de THRESHOLD - HYSTERESIS
  if (previamente_permitido) {
    if (temperatura >= (THRESHOLD_C - HYSTERESIS)) {
      ac_allowed = true;
    } else {
      ac_allowed = false;
    }
  } else {
    // Si antes no estaba permitido, permitimos solo si llega o supera el THRESHOLD
    if (temperatura >= THRESHOLD_C) {
      ac_allowed = true;
    } else {
      ac_allowed = false;
    }
  }
  previamente_permitido = ac_allowed;
}

float readTemperatureC() {
  // Lee la temperatura desde el BMP280, retorna nan si falla
  float t = bmp.readTemperature(); // devuelve °C
  return t;
}

void modos()
{
  if (digitalRead(pulsador_modo) == LOW) { // como es pullup, el estado LOW es presionado
    modo = modo + 1;
    if (modo >= 3) {
      modo = 0; 
    }
  }
  delay(100); // para evitar sobrecarga de lectura

  // Actualizar interfaz Blynk
  Blynk.virtualWrite(V1, 0);
  
  String textoModo; 
  int valor;
  switch (modo){
    case 0: // modo bajo AC 20%
      valor = (20*1023) / 100; // valor = 100 (opcion 2)
      textoModo = "MODO: BAJO (20%) "; // IMPRIMIR EL MODO
      break;
    case 1: // modo medio AC 60%
      valor = (60*1023) / 100; // valor = 700 (opcion 2)
      textoModo = "MODO: MEDIO (60%) "; // IMPRIMIR EL MODO 
      break;
    case 2: // modo alto AC 100%
      valor = 1023; 
      textoModo = "MODO: ALTO (100%) "; // IMPRIMIR EL MODO
  }
  Serial.print(textoModo); // IMPRIMIR EL MODO
  Serial.println(valor); // IMPRIMIR EL VALOR 
  analogWrite(ledpin, valor);

  Blynk.virtualWrite(V5, textoModo); // texto del modo
  Blynk.virtualWrite(V6, valor);  // valor PWM numérico

}

// BOTONES DE BLYNK 

// Botón ON/OFF en Blynk
BLYNK_WRITE(V0) {
  if (param.asInt() == 1) { // solo actúa al presionar
    if (!ac_allowed) {
      Serial.println("Intento de encendido desde Blynk rechazado: temperatura baja.");
      ac_on = false;
      Blynk.virtualWrite(V0, 0); // reflejar apagado
      return;
    }

    // Alternar estado
    ac_on = !ac_on;
    Serial.print("AC cambiado desde Blynk. Nuevo estado: ");
    Serial.println(ac_on ? "ON" : "OFF");

    // Reflejar el nuevo estado en la app
    Blynk.virtualWrite(V0, 0); 
  }
}

// Botón MODO en Blynk
BLYNK_WRITE(V1) {
  if (param.asInt() == 1) { // push
    modo++;
    if (modo >= 3) modo = 0;
    Serial.print("Cambio de modo desde Blynk: ");
    Serial.println(modo);
  }
}

// --------------------------------------------------------------------------------------

void loop()
{
   // Lectura temperatura periódica
  unsigned long now = millis();
  if (now - lastTempMillis >= TEMP_INTERVAL_MS) {
    lastTempMillis = now;
    float tempC = readTemperatureC();
    if (!isnan(tempC)) {
      Serial.print("Temperatura: ");
      Serial.print(tempC);
      Serial.print(" °C  | Umbral: ");
      Serial.print(THRESHOLD_C);
      Serial.print(" °C  | ");
    } else {
      Serial.print("Temperatura: (lectura inválida)  | Umbral: ");
      Serial.print(THRESHOLD_C);
      Serial.print(" °C  | ");
    }

      // Actualizar bloqueo por temperatura (histeresis incluida)
    actualizarBloqueoPorTemperatura(tempC);

    Serial.print("AC permitido? ");
    Serial.println(ac_allowed ? "SI" : "NO");

    // enviar variables a Blynk
    Blynk.virtualWrite(V3, isnan(tempC) ? 0 : tempC);
    Blynk.virtualWrite(V4, ac_allowed ? 1 : 0);
  }

  //botón ON/OF
  if (digitalRead(pulsador_on_off) == LOW) {
    // verificamos si está permitido por temperatura
    if (!ac_allowed) {
      Serial.println("Intento de encendido rechazado: temperatura por debajo del umbral.");
      ac_on = false; // reforzamos bloqueo
    } else {
      ac_on = !ac_on;
      Serial.print("AC cambiado localmente. Nuevo estado ac_on = ");
      Serial.println(ac_on ? "ON" : "OFF");
    }
  }
  delay(100); 


  // Si el AC no está permitido por temperatura, forzamos su apagado
  if (!ac_allowed) {
    analogWrite(ledpin, 0);
    ac_on = false; 
    modo = 0;
    Blynk.virtualWrite(V5, "MODO: DESHABILITADO"); 
    Blynk.virtualWrite(V6, 0);  
  } else {
    if (ac_on) {
      modos();
    } else {
      analogWrite(ledpin, 0);
    }
  }

  Blynk.run(); 
}