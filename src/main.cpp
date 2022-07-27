#include <Arduino.h>

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

#define ledVerde 2
#define ledRojo 14
#define releOK 4
#define releBAD 3
#define J1 9
#define J2 8
#define J3 7
#define J4 6
#define selEsc 5 //Este es para configurar si es Escaner de entrada o salida. 0 Entrada, 1 salida

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ipServer[] = { 192, 168, 100, 199 };
int portRemote = 80;
byte ipLocal[] = { 192, 168, 100, 200 }; //IPAddress ipLocal (192, 168, 1, 170);

bool  leyoCedula = false, enviaCedula= false;
String cedulaLeida;        
String modEscaner;
int DipSwitch;
int conParpadeo;

char datoEnviar[13]; //El tamaño corresponde: 10#ced + : + 1#modoEscan
bool habMon = true;

char c, ult, Pult, aPult;
String tresUltimas;

int reintento;


EthernetClient client;

void setup() {

  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(releOK, OUTPUT);
  pinMode(releBAD, OUTPUT);
  pinMode(J1, INPUT_PULLUP);
  pinMode(J2, INPUT_PULLUP);
  pinMode(J3, INPUT_PULLUP); 
  pinMode(J4, INPUT_PULLUP); 
  pinMode(selEsc, INPUT_PULLUP);
  

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(releOK, LOW);
  digitalWrite(releBAD, LOW);
//--------------------------------------------------------------
//-------- Asignando IP, Hab Monitoreo y sentido Escaner -------
//--------------------------------------------------------------
  if (!digitalRead(selEsc))
  {
    modEscaner = "1";
  }
  else{
    modEscaner = "2";    
  }

  DipSwitch = 0;
  
  if (digitalRead(J1)) DipSwitch = DipSwitch + 1;
  if (digitalRead(J2)) DipSwitch = DipSwitch + 2;
  if (digitalRead(J3)) DipSwitch = DipSwitch + 4;
  if (digitalRead(J4)) DipSwitch = DipSwitch + 8;
    
  ipLocal[3] = 200 + DipSwitch;
  mac[5] = 200 + DipSwitch;

  //if (digitalRead(J3)) habMon = true;
//--------------------------------------------------------------
  Serial.begin(38400);
  if (habMon){
    Serial.print("Dip: ");
    Serial.println(DipSwitch);
    Serial.println("Inicio...");
    Serial.println();
  }
  
  
  Ethernet.begin(mac, ipLocal);   // start the Ethernet connection:
  delay(1000); //Esperar 1 seg para que la Ethernet shield inicie
  
if (habMon){
    Serial.print("Ip: ");
    Serial.print(ipLocal[0]);//Serial.println(Ethernet.localIP());
    Serial.print(".");
    Serial.print(ipLocal[1]);
    Serial.print(".");
    Serial.print(ipLocal[2]);
    Serial.print(".");
    Serial.print(ipLocal[3]);
    Serial.print("  Modo: ");
    Serial.println(modEscaner);
    Serial.println();
  }
  //if (Ethernet.hardwareStatus() != EthernetNoHardware){
  if (Ethernet.hardwareStatus() != EthernetW5500){
    
    if (habMon) Serial.print("No encontro la W5500");
    while(Ethernet.hardwareStatus() != EthernetW5500){
      if (habMon) Serial.print(".");
      Ethernet.begin(mac, ipLocal); // start the Ethernet
      delay(1000);
    }
    if (habMon) Serial.println();
  }
  if (habMon) Serial.println("Ok W5500!");
  digitalWrite(ledRojo, HIGH);  
  
  if (Ethernet.linkStatus() == LinkOFF) {
    if (habMon) Serial.print("Ethernet cable is not connected");
    while(Ethernet.linkStatus() == LinkOFF){
      if (habMon) Serial.print(".");
      delay(1000);
    }
    Serial.println();
  }
  
  if (habMon) Serial.println("Ok Cable!");  
  digitalWrite(ledVerde, HIGH);  
  digitalWrite(ledRojo, LOW);  
  
  if (habMon){
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
      Serial.println("" + cedulaLeida + " " + modEscaner + " " + ipLocal[3]);
    }  

    digitalWrite(ledRojo, HIGH);
    if (client.connect(ipServer, portRemote)) {  // Conexion con el servidor
        digitalWrite(ledRojo, LOW);
        //client.println("" + cedulaLeida + "*" + modEscaner);
        client.println("GET /Registro/Php/ActualizaEscaner.php?cedula=" + cedulaLeida + "&modo=" + modEscaner + "&id=" + ipLocal[3] + " HTTP/1.0");
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
        tresUltimas = tresUltimas.substring((tresUltimas.length()-3));
        //tresUltimas = tresUltimas.substring(0,3);
        if (habMon) Serial.println(tresUltimas);

        if (tresUltimas == "OK"+modEscaner){
          if (habMon)Serial.println("¡Abre!");
          digitalWrite(releOK, HIGH);
          delay(500);
          digitalWrite(releOK, LOW);
          
        }else{
          if (habMon)Serial.println("¡No abre!");
          digitalWrite(releBAD, HIGH);
          delay(500);
          digitalWrite(releBAD, LOW);
        }
        leyoCedula = false;
        cedulaLeida = "";
    } else {
        if (habMon)Serial.println("Fallo en la conexion");
        if (reintento > 1){
          enviaCedula = true;
          reintento -= 1;
        }else{
          leyoCedula = false;
          cedulaLeida = "";  
        }
    }
    client.stop();
    client.flush();
    delay(10);
    if (habMon)Serial.println("Desconectado!");
    if (habMon)Serial.println();
     
  }

  
  while (Serial.available() && !leyoCedula) {
    cedulaLeida = Serial.readStringUntil('\r');
    leyoCedula = true;
    enviaCedula = true;
    reintento = 3;
    cedulaLeida = "0000000000" + cedulaLeida;
    cedulaLeida = cedulaLeida.substring((cedulaLeida.length()-10));
    //cedulaLeida = cedulaLeida.substring(0,10);
    digitalWrite(ledVerde, LOW);
    delay(100);
    digitalWrite(ledVerde, HIGH);
  }
}