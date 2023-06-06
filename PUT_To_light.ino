/*
 * Made By May Nesrine
 * Date 04-2023
 * Project Put To light
 * Send Data by the Modbus Protocol
 */
#define DEBUG
#include "DebugUtils.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#define C1_Pin 2
#define C2_Pin 4
#define C3_Pin 5
const int C[] = { C1_Pin, C2_Pin, C3_Pin };

#define LED1 12
#define LED2 14
#define LED3 16
const int LED[] = { LED1, LED2, LED3 };

#define id 1

int i;
const int Reg_R = 1;
const int Reg_w = 2;
IPAddress remote(192, 168, 137, 1);
ModbusIP mb;

uint16_t dataID;

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
#endif
  for (i = 0; i <= 2; i++) {
    pinMode(C[i], INPUT_PULLUP);
    pinMode(LED[i], OUTPUT);
    digitalWrite(LED[i], 1);
    delay(1000);
    digitalWrite(LED[i], 0);
  }

  //check wifi communication
  DEBUG_PRINTLN("check_wifi");
  WiFi.begin("your_ssid", "your_password");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    DEBUG_PRINTLN(".");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  delay(1000);
  //check Modbus communication
  DEBUG_PRINTLN("check_MB");
  mb.client();
  mb.connect(remote);

  while (!mb.isConnected(remote)) {
    mb.connect(remote);
    DEBUG_PRINTLN(".");
    delay(100);
  }
}
// goto bailout;
void loop() {
  // put your main code here, to run repeatedly:
  if (mb.isConnected(remote)) {
    uint16_t Rcv = mb.readHreg(remote, Reg_R, &dataID, 2);
    mb.task();  // Common local Modbus task
    delay(100);

    if (id == dataID || (id + 4) == dataID || (id + 8) == dataID) {
      uint16_t* r = &dataID;
      uint16_t ack;
      ack = *(r + 1);
      DEBUG_PRINT(dataID);
      DEBUG_PRINTLN(ack);
      if (ack == 1) {
        if (dataID == id) {
          DEBUG_PRINTLN("ActionPIN0");
          Action(0);
        }
        if (dataID == id + 4) {
          DEBUG_PRINTLN("ActionPIN1");
          Action(1);
        }
        if (dataID == id + 8) {
          DEBUG_PRINTLN("ActionPIN2");
          Action(2);
        }
        dataID = 0;
      }
    }
  }

  else {
    DEBUG_PRINTLN(".");
    mb.connect(remote);
    delay(100);
  }
}
void StateLeds() {
  for (i = 0; i <= 2; i++) {
    digitalWrite(LED[i], 0);
    DEBUG_PRINTLN("LED_off");
    delay(1000);
  }
}

void Action(int P) {
  StateLeds();
  delay(100);
  digitalWrite(LED[P], 1);
  DEBUG_PRINTLN("ledon");
  mb.writeHreg(remote, Reg_w, 2);
  mb.writeHreg(remote, Reg_w - 1, &dataID, 1);
  mb.task();  // Common local Modbus task
  delay(100);

  while (digitalRead(C[P])) {
    delay(100);
    if (check() != dataID) {
      StateLeds();
      return;
    }
  }
  mb.writeHreg(remote, Reg_w, 3);
  mb.writeHreg(remote, Reg_w - 1, &dataID, 1);
  mb.task();  // Common local Modbus task
  delay(100);
  DEBUG_PRINTLN("box_out");

  while (digitalRead(C[P]) == 0) {
    delay(100);
    if (check() != dataID) {
      StateLeds();
      return;
    }
  }
  digitalWrite(LED[P], 0);
  mb.writeHreg(remote, Reg_w, 4);
  mb.writeHreg(remote, Reg_w - 1, &dataID, 1);
  DEBUG_PRINTLN("box_in");
  DEBUG_PRINTLN("led off");
}
int check() {
  if (mb.isConnected(remote)) {
    uint16_t dataID2;
    uint16_t SCAN2 = mb.readHreg(remote, Reg_R, &dataID2, 1);
    while (mb.isTransaction(SCAN2)) {
      mb.task();
      delay(50);
    }
    //Serial.println(dataID2 != dataID);
    return dataID2;
  }
}
