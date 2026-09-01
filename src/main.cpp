#include <Arduino.h>

#include <math.h>

#include "AdafruitIO_WiFi.h" 

#include "secrets.h"

#define pinNTC 34

AdafruitIO_WiFi io(
  IO_USERNAME,
  IO_KEY,
  WIFI_SSID,
  WIFI_PASSWORD
);

//Referencia ao feed temperatura
AdafruitIO_Feed *feedTemperatura = io.feed("temperatura");

const float Rfixo = 10000.0;
const float Beta = 3950.0;
const float R0 = 10000.0;
const float T0_kelvin = 298.15;
const float Vcc = 3.3;

//NAN indica que ainda não existe uma leitura anterior válida 
float temperaturaAnterior = NAN;

//Armazena o instante do último envio ao feed
unsigned long ultimoEnvio = 0;

const unsigned long INTERVALO_ENVIO = 3000;

float lerTemperaturaNTC(int pino, int numLeituras) 
  {
    long somaLeituras = 0;

  for (int i = 0; i < numLeituras; i++) {
    somaLeituras += analogRead(pino);
    delay(5);
  }
  float leituraMedia = somaLeituras / (float)numLeituras;

  float Vout = leituraMedia * (Vcc / 4095.0);

  float Rntc = Rfixo * ((Vcc / Vout) - 1.0);

  float tempK = 1.0 / ((1.0 / T0_kelvin) + (1.0 / Beta) * log(Rntc / R0));

  return tempK - 273.15;
}


void setup() {
  pinMode(pinNTC, INPUT);
  Serial.begin(115200);

  // Define o ADC da ESP32 com resolução de 12 bits (0 a 4095)
  analogReadResolution(12);

  Serial.println();

  Serial.println("Iniciando a ESP...");

  Serial.println("Conectando ao AdafruitIO...");

  //Imiciar a conexão Wi-Fi e com a AdaFruit IO
  io.connect();

  //Aguardar ate que a conexão seja estabelecida
  while(io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  //Exibe o estado atual da conexão
  Serial.println(io.statusText());
  Serial.println("Adafruit IO conectado!");
}

void loop()
{
  //Mantém ativa a comunicação com a Adafruit IO
  io.run();

  // Verifica se ja passaram 3 segundos
  if((millis() - ultimoEnvio) < INTERVALO_ENVIO){
    return;
  } 


ultimoEnvio = millis();

float temperaturaAtual = lerTemperaturaNTC(pinNTC, 10);

if(isnan(temperaturaAtual)){
  Serial.println("Erro de leitura do sensor...");
  return;
}

Serial.print("Temperatura: ");
Serial.print(temperaturaAtual, 2);
Serial.println("°C");

// Vai enviar a leitura somente quando a variação < 0,10
if(isnan (temperaturaAnterior) ||
  fabs (temperaturaAtual - temperaturaAnterior) >= 0.10){
    Serial.println("Enviando para a Adafruit IO...");

    // publicar a temperatura coletada no feed
    feedTemperatura->save(temperaturaAtual);
      

  }
}