//Tech Trends Shameer
//Control LED Using Blynk 2.0/Blynk IOT

#define BLYNK_TEMPLATE_ID "TMPL2vJrMkueN"
#define BLYNK_TEMPLATE_NAME "CONTROL LED"
#define BLYNK_AUTH_TOKEN "jUNhM4JEYoOkHe99SaCgygyW2mHUM3rN"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>  
#include <BlynkSimpleEsp8266.h>
 

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "CLARO1_F4A959";  // Enter your Wifi Username
char pass[] = "21938DEKzc";  // Enter your Wifi password

const int ledpin = 15; // D8
const int pulsador_on_off = 14; // D5
const int pulsador_modo = 12; // D6
bool ac_on = false;
int modo = 0;

void setup()
{     
  Serial.begin(115200);
  //Blynk.begin(auth, ssid, pass);    
  delay(2000);
  pinMode(ledpin,OUTPUT);
  pinMode(pulsador_on_off,INPUT_PULLUP);
  pinMode(pulsador_modo,INPUT_PULLUP);
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

  int valor;
  switch (modo){
    case 0: // modo bajo AC 20%
      valor = (20*1023) / 100; // valor = 100 (opcion 2)
      Serial.print("MODO: BAJO (20%) - PWM: "); // IMPRIMIR EL MODO
      break;
    case 1: // modo medio AC 60%
      valor = (60*1023) / 100; // valor = 700 (opcion 2)
      Serial.print("MODO: MEDIO (60%) - PWM: "); // IMPRIMIR EL MODO 
      break;
    case 2: // modo alto AC 100%
      valor = 1023; 
      Serial.print("MODO: ALTO (100%) - PWM: "); // IMPRIMIR EL MODO
  }
  Serial.println(valor); // IMPRIMIR EL VALOR 
  analogWrite(ledpin, valor);

}


void loop()
{
  if (digitalRead(pulsador_on_off) == LOW) { // como es pullup, el estado LOW es presionado
    ac_on = !ac_on;
  }
  delay(150);

  if(ac_on){
    modos();
  } else {
    analogWrite(ledpin, 0);
    modo = 0;
  }


  Blynk.run(); 
}