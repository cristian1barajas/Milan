/*
 * File:   main.cpp
 * Author: Ing. Christian Barajas
 *
 * Programa especifico para el control de acceso con una conexión a base de
 * datos mediante shield ethernet.
 *
 * BotLAB co
 * Created on 27 de julio de 2022, 03:39 PM
 */

#include <SPI.h>
#include <Ethernet.h>

// ** Constantes salidas digitales ** //
#define ledVerde 2
#define ledRojo 14
#define releOK 3
#define releBAD 2

// ** Variables de configuración de la tarjeta Ethernet ** //
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ipServer[] = {192, 168, 1, 90};
int portRemote = 80;
byte ipLocal[] = {192, 168, 1, 91};

String texto1 = "GET /api/v1/registrovisitas/visitantes/validarAcceso?codigoBuscar=";
String texto2 = "&estacion=P-03&lugar=5";
String lectura = "";

bool leyoCedula = false, enviaCedula = false;
bool habMon = true;
int reintento;
int conParpadeo;

// ** Objeto Ethernet ** //
EthernetClient client;
String tresUltimas;

void setup()
{

  Serial.begin(115200);
  Serial.println("Hola inicio!!");

  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(releOK, OUTPUT);
  pinMode(releBAD, OUTPUT);

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(releOK, HIGH);
  digitalWrite(releBAD, HIGH);

  Ethernet.begin(mac, ipLocal); // start the Ethernet connection:
  delay(1000);                  // Esperar 1 seg para que la Ethernet shield inicie

  if (Ethernet.hardwareStatus() != EthernetW5500)
  {
    if (habMon)
      Serial.print("No encontro la W5500");
    while (Ethernet.hardwareStatus() != EthernetW5500)
    {
      if (habMon)
        Serial.print(".");
      Ethernet.begin(mac, ipLocal); // start the Ethernet
      delay(1000);
    }
    if (habMon)
      Serial.println();
  }
  if (habMon)
    Serial.println("Ok W5500!");
  digitalWrite(ledRojo, HIGH);

  if (Ethernet.linkStatus() == LinkOFF)
  {
    if (habMon)
      Serial.print("Ethernet cable is not connected");
    while (Ethernet.linkStatus() == LinkOFF)
    {
      if (habMon)
        Serial.print(".");
      delay(1000);
    }
    Serial.println();
  }

  if (habMon) Serial.println("Ok Cable!");
  //digitalWrite(ledVerde, HIGH);
  digitalWrite(ledRojo, LOW);

  if (habMon)
  {
    Serial.println();
    Serial.println("Esperando detecte Cédula");
  }
}

void loop()
{
  if (enviaCedula){
    enviaCedula = false;
    conParpadeo = 0;
    if (habMon) {
      Serial.println("Dato que va a enviar: ");
      Serial.println(texto1 + lectura + texto2);
    }

    digitalWrite(ledRojo, HIGH);
    if (client.connect(ipServer, 9081)) 
    {  // Conexion con el servidor
        digitalWrite(ledRojo, LOW);
        //client.println(texto1 + lectura + texto2 + "&id=" + ipLocal[3] + " HTTP/1.0");
        client.println(texto1 + lectura + texto2 + " HTTP/1.0");
        client.println("User-Agent: Escaner 1.0");
        client.println();
        if (habMon)Serial.println("Respuesta:");

        while (client.connected())
        {
          if (client.available())
          {
            tresUltimas = client.readStringUntil('\n');
          }
        }
        //client.stop();
        //client.flush();
        tresUltimas = tresUltimas.substring(11, 15);
        //tresUltimas = tresUltimas.substring(0,3);
        if (habMon) Serial.println(tresUltimas);

        if (tresUltimas == "0000" && lectura[21] == 'S')
        {
          if (habMon)Serial.println("¡Abre!");
          digitalWrite(releOK, LOW);
          delay(1000);
          digitalWrite(releOK, HIGH);
        }
         if (tresUltimas == "0000" && lectura[21] == 'E')
        {
          if (habMon)Serial.println("¡Abre!");
          digitalWrite(releBAD, LOW);
          delay(1000);
          digitalWrite(releBAD, HIGH);
        }
        else
        {
          if (habMon)Serial.println("¡No abre!");
         
        }
        leyoCedula = false;
        lectura = "";
    } else {
        if (habMon)Serial.println("Fallo en la conexion");
        if (reintento > 1){
          enviaCedula = true;
          reintento -= 1;
        }else{
          leyoCedula = false;
          lectura = "";
        }
    }
    client.stop();
    client.flush();
    delay(10);
    if (habMon)Serial.println("Desconectado!");
    if (habMon)Serial.println();
  }

  while (Serial.available() && !leyoCedula)
  {
    lectura = Serial.readStringUntil('\r');
    leyoCedula = true;
    enviaCedula = true;
    reintento = 3;
    // Serial.println(texto1 + lectura + texto2);
    // digitalWrite(ledVerde, LOW);
    // delay(100);
    // digitalWrite(ledVerde, HIGH);
  }
}