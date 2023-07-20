/*
  MCU : ESP32
  Platform : Arduino
  IDE : Arduino IDE

  Author : REDWOLF DiGiTAL (Worakan Chanthima)
  Github : https://github.com/redwolf-digital

  7-19-2023
  Code : 0.1 Rev.A
*/


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HardwareSerial.h>


#include "TridentTD_LineNotify.h"
#include <TinyGPSPlus.h>


#define LINE_TOKEN      "vHYoeRgx5qN49CDdBcPs9y6r8f1AnCdcl83XKugM3YM"
#define WIFI_SSID       "Kiatsuda_210_2.4G"
#define WIFI_PASS       "omo901256"

#define GPS_Rx    17
#define GPS_Tx    16
#define GPS_Baud  9600

TinyGPSPlus GPS;
HardwareSerial gpsSerial(2);

unsigned int dummy1 = 0, dummy2 = 0, dummy3 = 0, dummy4 = 0;
char StrTemp[1500];
double LAT, LNG;
unsigned long sendTime;


void setup(void) {
  Serial.begin(115200);
  gpsSerial.begin(GPS_Baud);

  // init WiFi
  Serial.print("\n\n");
  Serial.print(F("Connect to "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.print("\n");
  Serial.println(F("Connect pass"));
  Serial.print(F("IP : "));
  Serial.println(WiFi.localIP());


  // Line init
  LINE.setToken(LINE_TOKEN);
  LINE.notify("DEVICE BOOT COMPLETE");
}



void loop(void) {
  dummy1 = random(0, 255);
  dummy2 = random(0, 255);
  dummy3 = random(0, 2000);
  dummy4 = random(0, 1000);

  while(gpsSerial.available() > 0) {
    GPS.encode(gpsSerial.read());
      if(GPS.location.isUpdated()) {
        
        // Latitude in degrees (double)
        LAT = GPS.location.lat();
        
        // Longitude in degrees (double)
        LNG = GPS.location.lng();
        Serial.print(" Longitude= ");
      }
  }

  if(millis() - sendTime >= 25000) {
    sprintf(StrTemp, "\r\n Sensor1 -> %d\r\n Sensor2 -> %d\r\n Sensor3 -> %d\r\n Sensor4 -> %d\r\n LAT -> %lf\r\n LNG -> %lf", dummy1, dummy2, dummy3, dummy4, LAT, LNG);
    LINE.notify(StrTemp);
    sendTime = millis();
  }
}
