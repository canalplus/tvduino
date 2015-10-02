/* 
 *  TVduino
 * 
 *  This program exposes an REST API to multiple readings
 *  to retrieve the status of a television
 * 
 *  Filipe Caldas (fcaldas@canalplus.fr)
 */

#include <TimerOne.h>
#include <SPI.h>
#include <EthernetUdp.h>
#include <EthernetServer.h>
#include <Ethernet.h>
#include "restapi.h"
#include "jsonParser.h"
#include "timeinter.h"
#include "zapTime.h"

//Network configuration for arduino
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = {10, 0, 2, 160};
byte gateway[] = { 10, 0, 0, 1 };
byte subnet[] = { 255, 255, 0, 0 };

restServer *myServer;
EthernetClient http_client;

/*
 * Simple callback that returns readings of the two photoresistors
 * in ports A4 and A5.
 */
void getVideo(EthernetClient *client, char args[]){
  int v1 = analogRead(A4);
  int v2 = analogRead(A5);
  client->print("{\"video1\":");
  client->print(v1);
  client->print(",\"video2\":");
  client->print(v2);
  client->println("}"); 
}

void setup() {
  Serial.begin(9600);
  myServer = new restServer(mac, ip, gateway, subnet,80);
  delay(1000);
  //create routes on our rest server
  myServer->addRoute("/video", GET, &getVideo);
  myServer->addRoute("/getlivestatus", POST, &getLiveStatus);
  myServer->addRoute("/status", POST, &stbState);
  myServer->addRoute("/zap", POST, &dozap);
  myServer->addRoute("/wakeup", POST, &wakeup);
  //Configure time interruptions for 50ms
  //this will be used to measure the time to switch
  //channels.
  Serial.println("Starting TVduino");
  TimeInterruption::init(50000);
}

void loop() {
  myServer->serve();
}
