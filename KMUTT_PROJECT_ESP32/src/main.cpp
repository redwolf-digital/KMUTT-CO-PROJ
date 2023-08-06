#include <Arduino.h>
#include "HTTPClient.h"

#include "WiFi.h"

#define SSID    "YOUR SSID"
#define PASS    "YOUR PASS"

String GOOGLE_SCRIPT_ID = "AKfycbyBmp4C_tCdRTgzQbnBfWeMDvXm6HUKO19lzwAEyskuF5Md5dFNsaFJrVCqz2bbmk7JrA";

struct node1data {
    float x;
    float y;
    float z;
};

struct node2data {
    float x;
    float y;
    float z;
};

struct node3data {
    float x;
    float y;
    float z;
};


struct gpsdata {
    String time;
    String date;
    float lat;
    float lon;
};


node1data N1;
node2data N2;
node3data N3;
gpsdata GPSDATA;

void setup(void) {
    Serial.begin(115200);       // UART0 - Debug
    
    // Wifi init and debug
    Serial.printf("Connect wifi : %s", SSID);
    WiFi.begin(SSID, PASS);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("*"));
    }


    N1.x = 123.008;
    N1.y = -1556.335;
    N1.z = -48.549;

    N2.x = 887.085;
    N2.y = -8512.451;
    N2.z = 892.668;

    N3.x = -561.799;
    N3.y = -513.195;
    N3.z = 3816.792;

    GPSDATA.lat = 1338.96632;
    GPSDATA.lon = 10029.23846;
    GPSDATA.time = "072928";
    GPSDATA.date = "6/8/2023";

}


void loop(void) {

    if(WiFi.status() == WL_CONNECTED) {
        String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?" + "date=" + GPSDATA.date + "&time=" + GPSDATA.time + "&lat=" + GPSDATA.lat + "&lon=" + GPSDATA.lon + "&n1x=" + N1.x + "&n1y=" + N1.y + "&n1z=" + N1.z + "&n2x=" + N2.x + "&n2y=" + N2.y + "&n2z=" + N2.z + "&n3x=" + N3.x + "&n3y=" + N3.y + "&n3z=" + N3.z;

        
        HTTPClient http;
        http.begin(urlFinal.c_str());
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        Serial.printf("HTTP CODE : %d\r\n", httpCode);
        if(httpCode == 200) {
            Serial.print(F("POST DATA TO GOOGLE SHEET COMPLEAT\r\n"));
        } 

        http.end();
    }
    
    delay(10000); // delay 10 sec
}
