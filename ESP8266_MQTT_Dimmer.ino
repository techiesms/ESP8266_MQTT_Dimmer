#include "hw_timer.h"
#include "UbidotsESPMQTT.h"
#include <stdio.h>
const byte zcPin = 0;
const byte pwmPin = 2;
// String temp;
byte fade = 1;
byte state = 1;
byte tarBrightness = 255;
byte curBrightness = 0;
byte zcState = 0; // 0 = ready; 1 = processing;

void zcDetectISR();
void dimTimerISR();


#define WIFINAME "SSID" //Your SSID
#define WIFIPASS "PASS" // Your Wifi Pass
#define TOKEN "YOUR_TOKEN_NUMBER" // Your Ubidots TOKEN


Ubidots client(TOKEN);


/****************************************
   Auxiliar Functions
 ****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(1, LOW);
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");

  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    // temp = temp + payload[i
    //Serial.println();
    // //Serial.println(temp);
    int mytopic = atoi(topic);
    payload[length] = '\0';
    int mymsg = atoi ((const char*)payload);

    int val = mymsg;
    if (val > 0) {
      tarBrightness = val;
      //Serial.println(tarBrightness);
      digitalWrite(1, LOW);
    }
    //  temp = "";
  }
}


void setup() {

  //Serial.begin(115200);
  client.ubidotsSetBroker("business.api.ubidots.com"); // Sets the broker properly for the business account
  client.setDebug(false); // Pass a true or false bool value to activate debug messages
  //Serial.begin(115200);
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);
  client.ubidotsSubscribe("esp-01", "bulb"); //Insert the dataSource and Variable's Labels

  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  pinMode(zcPin, INPUT_PULLUP);
  pinMode(pwmPin, OUTPUT);
  attachInterrupt(zcPin, zcDetectISR, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  hw_timer_init(NMI_SOURCE, 0);
  hw_timer_set_func(dimTimerISR);
  //client.ubidotsSubscribe("esp32", "relay1"); //Insert the dataSource and Variable's Labels
}



void loop() {

  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe("esp-01", "bulb"); //Insert the dataSource and Variable's Labels
  }
  client.loop();
}


void dimTimerISR() {
  if (fade == 1) {
    if (curBrightness > tarBrightness || (state == 0 && curBrightness > 0)) {
      --curBrightness;
    }
    else if (curBrightness < tarBrightness && state == 1 && curBrightness < 255) {
      ++curBrightness;
    }
  }
  else {
    if (state == 1) {
      curBrightness = tarBrightness;
    }
    else {
      curBrightness = 0;
    }
  }

  if (curBrightness == 0) {
    state = 0;
    digitalWrite(pwmPin, 0);
  }
  else if (curBrightness == 255) {
    state = 1;
    digitalWrite(pwmPin, 1);
  }
  else {
    digitalWrite(pwmPin, 1);
  }

  zcState = 0;
}

void zcDetectISR() {
  if (zcState == 0) {
    zcState = 1;

    if (curBrightness < 255 && curBrightness > 0) {
      digitalWrite(pwmPin, 0);

      int dimDelay = 30 * (255 - curBrightness) + 400;//400
      hw_timer_arm(dimDelay);
    }
  }
}
