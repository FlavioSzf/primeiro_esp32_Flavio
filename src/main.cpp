#include <Arduino.h>
#include <math.h>

#include "AdafruitIO_WiFi.h"

#include "secrets.h"

AdafruitIO_WiFi io(
    IO_USERNAME,
    IO_KEY,
    WIFI_SSID,
    WIFI_PASS);

// Referencia ao feed temperatura
AdafruitIO_Feed *feedTemperatura = io.feed("temperatura");

const int LED_PIN_VERMELHO = 14;
const int LED_PIN_VERDE = 4;

const float TEMPERATURA_LIMITE = 22.0;

void handleTemperatura(AdafruitIO_Data *data);

void piscaLed(int pino, int qtdePiscas){
  for(byte i= 0; i < qtdePiscas; i++){
    digitalWrite(pino, HIGH);
    delay(500);
    digitalWrite(pino, LOW);
    delay(500);
  }
};

void setup()
{
  pinMode(LED_PIN_VERMELHO, OUTPUT);
  pinMode(LED_PIN_VERDE, OUTPUT);
  Serial.begin(115200);

  // Define o ADC da ESP32 com resolução de 12 bits (0 a 4095)
  analogReadResolution(12);

  Serial.println();

  Serial.println("Iniciando a ESP...");

  Serial.print("Conectando ao AdafruitIO");

  // Iniciar a conexão WI-FI com a Adafruit IO
  io.connect();

  // Registra função que será acionada quando chegar um novo valor no feed
  feedTemperatura-> onMessage(handleTemperatura);

  // Aguardar até que a conexão seja estabelecida
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  };

  Serial.println();

  piscaLed(LED_PIN_VERMELHO, 10);

  // Exibe o estado atual da conexão
  Serial.println(io.statusText());
  Serial.println("Adafruit IO conectado!");

  digitalWrite(LED_PIN_VERMELHO, 1);
}

void handleTemperatura(AdafruitIO_Data *data){
  float temperatura = data->toFloat();

  Serial.print("Temperatura recebida: ");
  Serial.print(temperatura, 2);
  Serial.println("°C");

  if(temperatura > TEMPERATURA_LIMITE){
    digitalWrite(LED_PIN_VERDE, 1);
    digitalWrite(LED_PIN_VERMELHO, 0);
    Serial.println("Alerta ligado!!!");
  }
  else{
    digitalWrite(LED_PIN_VERDE, 0);
    digitalWrite(LED_PIN_VERMELHO, 1);
    Serial.println("Temperatura normal: LED VERDE DESLIGADO!!!");
  }
}

void loop()
{
  io.run();
}