/*
6-8-2023
*/
#include <Arduino.h>
#include "HTTPClient.h"

#include "WiFi.h"
#include "TinyGPSPlus.h"

#define SSID        ""
#define PASS        ""
// #define GPS_Rx      17
// #define GPS_Tx      16
#define GPS_Baud    9600
#define Sensor_Baud 9600

String GOOGLE_SCRIPT_ID = "AKfycbyBmp4C_tCdRTgzQbnBfWeMDvXm6HUKO19lzwAEyskuF5Md5dFNsaFJrVCqz2bbmk7JrA";


// String *global_RxData;

struct node1data {
    float x;
    float y;
    float z;
    float temp;
    float humi;
};

struct node2data {
    float x;
    float y;
    float z;
    float temp;
    float humi;
};

struct node3data {
    float x;
    float y;
    float z;
    float temp;
    float humi;
};


struct gpsdata {
    String time;
    String date;
    float lat, lon;
    int hr_time, min_time, sec_time, day, month, year;
};

// GNSS RTC
RTC_DATA_ATTR float RTC_lat = 0;
RTC_DATA_ATTR float RTC_lon = 0;
RTC_DATA_ATTR int RTC_hr_time = 0;
RTC_DATA_ATTR int RTC_min_time = 0;
RTC_DATA_ATTR int RTC_sec_time = 0;
RTC_DATA_ATTR int RTC_day = 0;
RTC_DATA_ATTR int RTC_month = 0;
RTC_DATA_ATTR int RTC_year = 0;


struct systemStatus {
    unsigned char gpsReady;
    unsigned char sensorReady;
    unsigned char sheetFailCounter;
    unsigned char WiFiTimeoutCounter;
    unsigned long lastSendTime;
    unsigned char GNSSCounter;
    unsigned char failCounter;
    unsigned char SensorfailCounter;
};


struct systemBuffer {
    char httpStringBuff[2048];
};


node1data N1;
node2data N2;
node3data N3;
gpsdata GPSDATA;
systemStatus sys;
systemBuffer sysBuff;

TinyGPSPlus GPS;
HardwareSerial sensorSerial(1);
HardwareSerial gpsSerial(2);


int countEvent(String str, char target) {
    int count = 0;
    for(int i = 0; i < str.length(); i++) {
        if(str.charAt(i) == target) {
            count++;
        }
    }
    return count;
}


void setup(void) {
    // init valule at startup
    sys.sheetFailCounter = 0;
    sys.WiFiTimeoutCounter = 0;
    sys.lastSendTime = 0;
    sys.gpsReady = 0;
    sys.sensorReady = 0;
    sys.GNSSCounter = 0;
    sys.failCounter = 0;
    sys.SensorfailCounter = 0;

    N1.x = 0;
    N1.y = 0;
    N1.z = 0;
    N1.temp = 0;
    N1.humi = 0;

    N2.x = 0;
    N2.y = 0;
    N2.z = 0;
    N2.temp = 0;
    N2.humi = 0;

    N3.x = 0;
    N3.y = 0;
    N3.z = 0;
    N3.temp = 0;
    N3.humi = 0;

    GPSDATA.lat = 0;
    GPSDATA.lon = 0;
    GPSDATA.hr_time = 0;
    GPSDATA.min_time = 0;
    GPSDATA.sec_time = 0;
    GPSDATA.day = 0;
    GPSDATA.month = 0;
    GPSDATA.year = 0;

    GPSDATA.time = "0";
    GPSDATA.date = "0";



    Serial.begin(115200);                                           // UART0 - Debug
    sensorSerial.begin(Sensor_Baud, SERIAL_8N1, 21, 19);            // UART1 - Sensor Master node | Rx = GPIO21 Tx = GPIO19
    gpsSerial.begin(GPS_Baud);                                      // UART2 - GNSS module

    // Wifi init and debug
    Serial.printf("Connect wifi : %s", SSID);
    WiFi.begin(SSID, PASS);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("-"));
        sys.WiFiTimeoutCounter++;

        if(sys.WiFiTimeoutCounter == 120) {
            sys.WiFiTimeoutCounter = 0;
            ESP.restart();
        }
    }

    Serial.print(F("\nCONNECT WIFI COMPLEAT\n"));
} // END Void setup


void loop(void) {
    // GOOGLE SHEET API
    if((millis() - sys.lastSendTime) >= 60000) {        // Demo 1 min period
        
        // Wait GNSS module
        GPSTASK :
        while(sys.gpsReady == 0) {
            while(gpsSerial.available() > 0) {
                if(GPS.encode(gpsSerial.read())) sys.GNSSCounter++;

                if(sys.GNSSCounter >= 5) {
                    if(GPS.location.isValid() && GPS.date.isValid() && GPS.time.isValid()) {
                        Serial.print(F("GPS VALID\n"));
                        sys.gpsReady = 1;
                        sys.GNSSCounter = 0;
                    }else {
                        sys.GNSSCounter = 0;
                        sys.failCounter++;
                        Serial.print(F("GNSS : RETRY\n"));

                        while(sys.failCounter >= 20) {
                            Serial.print(F("GPS FAIL\n"));
                            sys.GNSSCounter = 0;
                            sys.failCounter = 0;
                            sys.gpsReady = 2;
                        }
                        goto GPSTASK;
                    }
                }
            }
        }


        // Call data from master sensor
        SENSORTASK :
        if(sys.sensorReady == 0) {

            sensorSerial.write(0x42);       // Send B

            if(sensorSerial.available()) {
                String RxBuffer = sensorSerial.readString();
                RxBuffer.trim();

                int numValue = countEvent(RxBuffer, ',') + 1;
                // numValue = countEvent(RxBuffer, ',') + 1;
                String RxData[numValue];
                unsigned char startIndex = 0;

                for(int i = 0; i < numValue; i++) {
                    int commaIndex = RxBuffer.indexOf(',', startIndex);

                    if(commaIndex != -1) {
                        RxData[i] = RxBuffer.substring(startIndex, commaIndex);
                        startIndex = commaIndex + 1;
                    }else {
                        RxData[i] = RxBuffer.substring(startIndex);
                    }
                }

                // Data process
                char headerByte[1], stopByte[1];
                RxData[1].toCharArray(headerByte, RxData[1].length()+1);    // 0x56 - V
                RxData[36].toCharArray(stopByte, RxData[36].length()+1);    // 0x7A - z

                if(headerByte[0] == 0x56 && stopByte[0] == 0x7A) {
                    Serial.print(F("SENSOR VALID\n"));
                    

                    // Dump buffer for debug
                    for(int i = 0; i < numValue; i++) {
                        Serial.println(RxData[i]);
                    }
                    //Process data
                    N1.x = RxData[3].toFloat();
                    N1.y = RxData[4].toFloat();
                    N1.z = RxData[5].toFloat();

                    N2.x = RxData[7].toFloat();
                    N2.y = RxData[8].toFloat();
                    N2.z = RxData[9].toFloat();

                    N3.x = RxData[11].toFloat();
                    N3.y = RxData[12].toFloat();
                    N3.z = RxData[13].toFloat();

                    N1.temp = RxData[16].toFloat();
                    N2.temp = RxData[18].toFloat();
                    N3.temp = RxData[20].toFloat();

                    N1.humi = RxData[23].toFloat();
                    N2.humi = RxData[25].toFloat();
                    N3.humi = RxData[27].toFloat();


                    sys.SensorfailCounter = 0;
                    sys.sensorReady = 1;
                }else {
                    sys.SensorfailCounter++;
                    Serial.print(F("SENSOR : RETRY\n"));

                    while(sys.SensorfailCounter >= 10) {
                        Serial.print(F("SENSOR FAIL\n"));
                        sys.SensorfailCounter = 0;
                        sys.sensorReady = 2;
                    }

                    delay(5);
                    goto SENSORTASK;
                }
            }else {
                sys.SensorfailCounter++;
                Serial.print(F("SENSOR : RETRY\n"));

                while(sys.SensorfailCounter >= 10) {
                    Serial.print(F("SENSOR FAIL\n"));
                    sys.SensorfailCounter = 0;
                    sys.sensorReady = 2;
                }
                delay(5);
                goto SENSORTASK;
            }
        }
        


        // Check WiFi connection
        while(WiFi.status() != WL_CONNECTED) {
            WiFi.begin(SSID, PASS);
            while(WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(F("-"));
                sys.WiFiTimeoutCounter++;

                if(sys.WiFiTimeoutCounter == 120) {     // Timeout 1 min
                    sys.WiFiTimeoutCounter = 0;
                    ESP.restart();
                }
            }
        }

        //  Put data to google sheet
        if(WiFi.status() == WL_CONNECTED && sys.gpsReady >= 1 && sys.sensorReady >= 1) {

            Serial.printf("GOOGLE SHEET TASK\n");

            // If GNSS data pass
            while(sys.gpsReady == 1) {
                GPSDATA.lat = GPS.location.lat();
                GPSDATA.lon = GPS.location.lng();
                GPSDATA.hr_time = GPS.time.hour();
                GPSDATA.min_time = GPS.time.minute();
                GPSDATA.sec_time = GPS.time.second();
                GPSDATA.day = GPS.date.day();
                GPSDATA.month = GPS.date.month();
                GPSDATA.year = GPS.date.year();

                // Put data to RTC RAM
                RTC_lat = GPSDATA.lat;
                RTC_lon = GPSDATA.lon;
                RTC_hr_time = GPSDATA.hr_time;
                RTC_min_time = GPSDATA.min_time;
                RTC_sec_time = GPSDATA.sec_time;
                RTC_day = GPSDATA.day;
                RTC_month = GPSDATA.month;
                RTC_year = GPSDATA.year;

                break;
            }
            // If GNSS data fail
            while(sys.gpsReady == 2) {
                //get last data from RTC RAM
                GPSDATA.lat = RTC_lat;
                GPSDATA.lon = RTC_lon;
                GPSDATA.hr_time = RTC_hr_time;
                GPSDATA.min_time = RTC_min_time;
                GPSDATA.sec_time = RTC_sec_time;
                GPSDATA.day = RTC_day;
                GPSDATA.month = RTC_month;
                GPSDATA.year = RTC_year;

                break;
            }

            
            // while(sys.sensorReady == 1) {
            //     N1.x = global_RxData[3].toFloat();
            //     N1.y = global_RxData[4].toFloat();
            //     N1.z = global_RxData[5].toFloat();

            //     N2.x = global_RxData[7].toFloat();
            //     N2.y = global_RxData[8].toFloat();
            //     N2.z = global_RxData[9].toFloat();

            //     N3.x = global_RxData[11].toFloat();
            //     N3.y = global_RxData[12].toFloat();
            //     N3.z = global_RxData[13].toFloat();

            //     N1.temp = global_RxData[16].toFloat();
            //     N2.temp = global_RxData[18].toFloat();
            //     N3.temp = global_RxData[20].toFloat();

            //     N1.humi = global_RxData[23].toFloat();
            //     N2.humi = global_RxData[25].toFloat();
            //     N3.humi = global_RxData[27].toFloat();

            //     break;
            // }
            // If sensor hook fail
            while(sys.sensorReady == 2) {
                N1.x = 0;
                N1.y = 0;
                N1.z = 0;

                N2.x = 0;
                N2.y = 0;
                N2.z = 0;

                N3.x = 0;
                N3.y = 0;
                N3.z = 0;

                N1.temp = 0;
                N2.temp = 0;
                N3.temp = 0;

                N1.humi = 0;
                N2.humi = 0;
                N3.humi = 0;

                break;
            }




            GPSDATA.time = String(GPSDATA.hr_time) + ":" + String(GPSDATA.min_time) + ":" + String(GPSDATA.sec_time);
            GPSDATA.date = String(GPSDATA.day) + "/" + String(GPSDATA.month) + "/" + String(GPSDATA.year);
            
            Serial.printf("LAT = %6f\r\nLON = %6f\r\nTime = %s\r\nDATE = %s\r\n", GPSDATA.lat, GPSDATA.lon, GPSDATA.time, GPSDATA.date);

            sprintf(sysBuff.httpStringBuff,"date=%s&time=%s&lat=%6f&lon=%6f&n1x=%f&n1y=%f&n1z=%f&n2x=%f&n2y=%f&n2z=%f&n3x=%f&n3y=%f&n3z=%f",GPSDATA.date, GPSDATA.time, GPSDATA.lat, GPSDATA.lon, N1.x, N1.y, N1.z, N2.x, N2.y, N2.z, N3.x, N3.y, N3.z);
            Serial.println(sysBuff.httpStringBuff);
            goto SHEETEVENT;



        /*
            If http not report code 200 -> loop 
        */
        SHEETEVENT :    
            // String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?" + "date=" + GPSDATA.date + "&time=" + GPSDATA.time + "&lat=" + GPSDATA.lat + "&lon=" + GPSDATA.lon + "&n1x=" + N1.x + "&n1y=" + N1.y + "&n1z=" + N1.z + "&n2x=" + N2.x + "&n2y=" + N2.y + "&n2z=" + N2.z + "&n3x=" + N3.x + "&n3y=" + N3.y + "&n3z=" + N3.z;
            String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?" + String(sysBuff.httpStringBuff);

            
            HTTPClient http;
            http.begin(urlFinal.c_str());
            http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
            int httpCode = http.GET();
            Serial.printf("HTTP CODE : %d\r\n", httpCode);
            if(httpCode == 200) {
                Serial.print(F("POST DATA TO GOOGLE SHEET COMPLEAT\r\n"));
            } else {
                sys.sheetFailCounter++;

                if(sys.sheetFailCounter >= 10) {
                    sys.sheetFailCounter = 0;
                    ESP.restart();
                }

                goto SHEETEVENT;
            }

            sys.gpsReady = 0;
            sys.sensorReady = 0;
            http.end();
        }

        sys.lastSendTime = millis();
    }

} // END VOID LOOP

