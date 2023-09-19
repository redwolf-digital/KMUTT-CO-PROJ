/*
Rev 1.0B
17-8-2023

Author : REDWOLF DiGiTAL (Worakan Chanthima)
Github : https://github.com/redwolf-digital
*/


#include <Arduino.h>
#include "HTTPClient.h"
#include "WiFi.h"

#include "TinyGPSPlus.h"

#define SSID        "ZephC_LTE"
#define PASS        "51064298"

// #define SSID        "Kiatsuda_210_2.4G"
// #define PASS        "omo901256"


#define GPS_Baud    9600
#define Sensor_Baud 9600

String GOOGLE_SCRIPT_ID = "AKfycbyD3CHTqBnTaJ5iUKPkNszIAGMYXAts4X6Fbed2qPAJ2jbAFPH5oC4c_9kurWPkMxhV";

// GPIO Define
#define SensorReadySignal   GPIO_NUM_4
#define RTSpin              GPIO_NUM_18


struct node1data {
    String time, date;
    float x;
    float y;
    float z;
    float temp;
    float humi;
    unsigned char batt;
};

struct node2data {
    String time, date;
    float x;
    float y;
    float z;
    float temp;
    float humi;
    unsigned char batt;
};

struct node3data {
    String time, date;
    float x;
    float y;
    float z;
    float temp;
    float humi;
    unsigned char batt;
};

struct node4data {
    String time, date;
    float x;
    float y;
    float z;
    float temp;
    float humi;
    float ath;
    float flow;
    unsigned char batt;
};

struct node5data {
    String time, date;
    float x;
    float y;
    float z;
    float temp;
    float humi;
    float ath;
    float flow;
    unsigned char batt;
};


struct gpsdata {
    float lat, lon;
};

struct systemStatus {
    unsigned char gpsDataReady;
    unsigned char sensorDataReady;

    unsigned char sheetFailCounter;
    unsigned char WiFiTimeoutCounter;
    unsigned char GNSSCounter;
    unsigned char failCounter;
    unsigned char SensorfailCounter;

    unsigned char reqNode;
    unsigned char reqInvalid;
    
    unsigned char nodeStatus[5];

    bool processFlag;
};


struct systemBuffer {
    char httpStringBuff[2048];
};


// GNSS RTC
RTC_DATA_ATTR float RTC_lat = 0;
RTC_DATA_ATTR float RTC_lon = 0;


node1data N1;
node2data N2;
node3data N3;
node4data N4;
node5data N5;

gpsdata GPSDATA;
systemStatus sys;
systemBuffer sysBuff;


TinyGPSPlus GPS;
HardwareSerial sensorSerial(1);
HardwareSerial gpsSerial(2);




void InitStruct(void) {
    N1.time = "00.00.00";
    N1.date = "00/00/0000";
    N1.x = 0;
    N1.y = 0;
    N1.z = 0;
    N1.temp = 0;
    N1.humi = 0;
    N1.batt = 0;

    N2.time = "00.00.00";
    N2.date = "00/00/0000";
    N2.x = 0;
    N2.y = 0;
    N2.z = 0;
    N2.temp = 0;
    N2.humi = 0;
    N2.batt = 0;

    N3.time = "00.00.00";
    N3.date = "00/00/0000";
    N3.x = 0;
    N3.y = 0;
    N3.z = 0;
    N3.temp = 0;
    N3.humi = 0;
    N3.batt = 0;
    
    N4.time = "00.00.00";
    N4.date = "00/00/0000";
    N4.x = 0;
    N4.y = 0;
    N4.z = 0;
    N4.temp = 0;
    N4.humi = 0;
    N4.batt = 0;
    N4.ath = 0;
    N4.flow = 0;
    
    N5.time = "00.00.00";
    N5.date = "00/00/0000";
    N5.x = 0;
    N5.y = 0;
    N5.z = 0;
    N5.temp = 0;
    N5.humi = 0;
    N5.batt = 0;
    N5.ath = 0;
    N5.flow = 0;

    for(unsigned char i = 0; i <= 4; i++) {
        sys.nodeStatus[i] = 1;
    }
}



int countEvent(String str, char target) {
    int count = 0;
    for(int i = 0; i < str.length(); i++) {
        if(str.charAt(i) == target) {
            count++;
        }
    }
    return count;
}


void mainprocess(void) {
    Serial.print(F("MAIN PROCESS\r\n"));
    InitStruct();
    // Sensor Event
    while(sys.sensorDataReady == 0) {
        
        SERIALREQEVENT :

            if(sys.reqNode == 0) {
                delay(50);
                sensorSerial.write(0x41);
                Serial.print("SENSOR -> REQ N1\r\n");
                goto PROCESSDATA;
            }
            else if(sys.reqNode == 1) {
                delay(50);
                sensorSerial.write(0x42);
                Serial.print("SENSOR -> REQ N2\r\n");
                goto PROCESSDATA;
            }
            else if(sys.reqNode == 2) {
                delay(50);
                sensorSerial.write(0x43);
                Serial.print("SENSOR -> REQ N3\r\n");
                goto PROCESSDATA;
            }
            else if(sys.reqNode == 3) {
                delay(50);
                sensorSerial.write(0x44);
                Serial.print("SENSOR -> REQ N4\r\n");
                goto PROCESSDATA;
            }
            else if(sys.reqNode == 4) {
                delay(50);
                sensorSerial.write(0x45);
                Serial.print("SENSOR -> REQ N5\r\n");
                goto PROCESSDATA;
            }

           

        PROCESSDATA :
            if(sensorSerial.available()) {
                
                sys.SensorfailCounter = 0;

                String RxBuffer = sensorSerial.readString();
                RxBuffer.trim();

                int numValue = countEvent(RxBuffer, ',') + 1;
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
                               
                // // Dump buffer
                for(unsigned char i = 0; i < 12; i++) {
                    Serial.print("INDEX ");
                    Serial.print(i);
                    Serial.print(" : ");
                    Serial.println(RxData[i]);
                }

                char HeaderByte[4];
                RxData[0].toCharArray(HeaderByte, RxData[0].length()+1);
                // for(unsigned char i = 0; i < 4; i++) {
                //     Serial.print(HeaderByte[i]);
                // }

                // Node A
                if(sys.reqNode == 0) {
                    if(HeaderByte[0] == 0x41) {
                        N1.time = RxData[1];
                        N1.date = RxData[2];
                        N1.x = RxData[3].toFloat();
                        N1.y = RxData[4].toFloat();
                        N1.z = RxData[5].toFloat();
                        N1.temp = RxData[6].toFloat();
                        N1.humi = RxData[7].toFloat();
                        N1.batt = RxData[8].toInt();

                        sys.reqInvalid = 0;
                        sys.nodeStatus[0] = 0;
                        sys.reqNode = 1;
                        
                        memset(RxData, 0, sizeof(RxData));
                        delay(500);              // For good respond

                        goto SERIALREQEVENT;
                    }else {
                        sys.reqInvalid++;

                        if(sys.reqInvalid > 10) {
                            sys.reqInvalid = 0;
                            sys.nodeStatus[0] = 1;
                            sys.reqNode = 1;
                            goto SERIALREQEVENT;
                        }

                        goto SERIALREQEVENT;
                    }
                }

                // Node B
                if(sys.reqNode == 1) {
                    if(HeaderByte[0] == 0x42) {
                        N2.time = RxData[1];
                        N2.date = RxData[2];
                        N2.x = RxData[3].toFloat();
                        N2.y = RxData[4].toFloat();
                        N2.z = RxData[5].toFloat();
                        N2.temp = RxData[6].toFloat();
                        N2.humi = RxData[7].toFloat();
                        N2.batt = RxData[8].toInt();

                        sys.reqInvalid = 0;
                        sys.nodeStatus[1] = 0;
                        sys.reqNode = 2;

                        memset(RxData, 0, sizeof(RxData));
                        delay(500);              // For good respond

                        goto SERIALREQEVENT;
                    }else {
                        sys.reqInvalid++;

                        if(sys.reqInvalid > 10) {
                            sys.reqInvalid = 0;
                            sys.nodeStatus[1] = 1;
                            sys.reqNode = 2;
                            goto SERIALREQEVENT;
                        }

                        goto SERIALREQEVENT;
                    }
                }

                // Node C
                if(sys.reqNode == 2) {
                    if(HeaderByte[0] == 0x43) {
                        N3.time = RxData[1];
                        N3.date = RxData[2];
                        N3.x = RxData[3].toFloat();
                        N3.y = RxData[4].toFloat();
                        N3.z = RxData[5].toFloat();
                        N3.temp = RxData[6].toFloat();
                        N3.humi = RxData[7].toFloat();
                        N3.batt = RxData[8].toInt();

                        sys.reqInvalid = 0;
                        sys.nodeStatus[2] = 0;
                        sys.reqNode = 3;

                        memset(RxData, 0, sizeof(RxData));
                        delay(500);              // For good respond

                        goto SERIALREQEVENT;
                    }else {
                        sys.reqInvalid++;

                        if(sys.reqInvalid > 10) {
                            sys.reqInvalid = 0;
                            sys.nodeStatus[2] = 1;
                            sys.reqNode = 3;
                            goto SERIALREQEVENT;
                        }

                        goto SERIALREQEVENT;
                    }
                }

                // BUG
                // Node D
                if(sys.reqNode == 3) {
                    if(HeaderByte[0] == 0x44) {
                        N4.time = RxData[1];
                        N4.date = RxData[2];
                        N4.x = RxData[3].toFloat();
                        N4.y = RxData[4].toFloat();
                        N4.z = RxData[5].toFloat();
                        N4.temp = RxData[6].toFloat();
                        N4.humi = RxData[7].toFloat();
                        N4.batt = RxData[8].toInt();
                        N4.ath = RxData[9].toFloat();
                        N4.flow = RxData[10].toFloat();

                        sys.reqInvalid = 0;
                        sys.nodeStatus[3] = 0;
                        sys.reqNode = 4;
                        
                        memset(RxData, 0, sizeof(RxData));
                        delay(500);              // For good respond

                        goto SERIALREQEVENT;
                    }else {
                        sys.reqInvalid++;

                        if(sys.reqInvalid > 10) {
                            sys.reqInvalid = 0;
                            sys.nodeStatus[3] = 1;
                            sys.reqNode = 4;
                            goto SERIALREQEVENT;
                        }

                        goto SERIALREQEVENT;
                    }
                }

                // Node E
                if(sys.reqNode == 4) {
                    if(HeaderByte[0] == 0x45) {
                        N5.time = RxData[1];
                        N5.date = RxData[2];
                        N5.x = RxData[3].toFloat();
                        N5.y = RxData[4].toFloat();
                        N5.z = RxData[5].toFloat();
                        N5.temp = RxData[6].toFloat();
                        N5.humi = RxData[7].toFloat();
                        N5.batt = RxData[8].toInt();
                        N5.ath = RxData[9].toFloat();
                        N5.flow = RxData[10].toFloat();

                        sys.reqInvalid = 0;
                        sys.nodeStatus[4] = 0;
                        sys.reqNode = 0;
                        sys.sensorDataReady = 1;

                        memset(RxData, 0, sizeof(RxData));

                        goto GNSSEVENT;
                    }else {
                        sys.reqInvalid++;

                        if(sys.reqInvalid > 10) {
                            sys.reqInvalid = 0;
                            sys.nodeStatus[4] = 1;
                            sys.reqNode = 0;
                            sys.sensorDataReady = 1;
                            goto GNSSEVENT;
                        }

                        goto SERIALREQEVENT;
                    }
                }


            }else {
                sys.SensorfailCounter++;

                if(sys.SensorfailCounter > 10) {
                    sys.reqNode++;
                    sys.SensorfailCounter = 0;
                    Serial.printf("SENSOR -> FAIL\r\n");
                    while(sys.reqNode >= 5) {
                        sys.reqNode = 0;
                        sys.SensorfailCounter = 0;
                        sys.sensorDataReady = 1;
                        goto GNSSEVENT;
                    }
                    goto SERIALREQEVENT;
                }

                goto SERIALREQEVENT;
            }
    }


    // GNSS Event
    GNSSEVENT :
        while(sys.gpsDataReady == 0) {
            while(gpsSerial.available()) {
                if(GPS.encode(gpsSerial.read())) sys.GNSSCounter++;

                if(sys.GNSSCounter >= 5) {
                    if(GPS.location.isValid()) {
                        Serial.print(F("GPS VALID\r\n"));
                        sys.gpsDataReady = 1;
                        sys.GNSSCounter = 0;
                    }else {
                        sys.GNSSCounter = 0;
                        sys.failCounter++;
                        Serial.print(F("GNSS : RETRY\n"));

                        while(sys.failCounter >= 20) {
                            Serial.print(F("GPS FAIL\n"));
                            sys.GNSSCounter = 0;
                            sys.failCounter = 0;
                            sys.gpsDataReady = 2;
                        }
                        goto GNSSEVENT;
                    }
                }
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


    // Google sheet Event
    Serial.printf("[STATUS]\r\nSENSOR : %d\r\nGPS : %d\r\nSENSOR REQ : %d\r\n", sys.sensorDataReady, sys.gpsDataReady, sys.reqNode);
    // Google sheet Evient
    if(WiFi.status() == WL_CONNECTED && sys.sensorDataReady >= 1 && sys.gpsDataReady >= 1) {
        Serial.printf("GOOGLE SHEET TASK\n");

        // Packing data procrss
        // GNSS data pass
        while(sys.gpsDataReady == 1) {
            GPSDATA.lat = GPS.location.lat();
            GPSDATA.lon = GPS.location.lng();

            // Put data to RTC me
            RTC_lat = GPSDATA.lat;
            RTC_lon = GPSDATA.lon;

            break;
        }
        // GNSS data fail
        while(sys.gpsDataReady == 2) {
            GPSDATA.lat = RTC_lat;
            GPSDATA.lon = RTC_lon;

            break;
        }


        // Packing data
        sprintf(sysBuff.httpStringBuff, "lat=%.6f&lon=%.6f&gnssStat=%d&n1stat=%d&n1time=%s&n1date=%s&n1x=%.2f&n1y=%.2f&n1z=%.2f&n1temp=%.2f&n1hium=%.2f&n1batt=%d&n2stat=%d&n2time=%s&n2date=%s&n2x=%.2f&n2y=%.2f&n2z=%.2f&n2temp=%.2f&n2hium=%.2f&n2batt=%d&n3stat=%d&n3time=%s&n3date=%s&n3x=%.2f&n3y=%.2f&n3z=%.2f&n3temp=%.2f&n3hium=%.2f&n3batt=%d&n4stat=%d&n4time=%s&n4date=%s&n4x=%.2f&n4y=%.2f&n4z=%.2f&n4temp=%.2f&n4hium=%.2f&n4batt=%d&n5stat=%d&n5time=%s&n5date=%s&n5x=%.2f&n5y=%.2f&n5z=%.2f&n5temp=%.2f&n5hium=%.2f&n5batt=%d&n4ath=%.2f&n5ath=%.2f&n4flowRate=%.2f&n5flowRate=%.2f", GPSDATA.lat, GPSDATA.lon, sys.gpsDataReady, sys.nodeStatus[0], N1.time, N1.date, N1.x, N1.y, N1.z, N1.temp, N1.humi, N1.batt, sys.nodeStatus[1], N2.time, N2.date, N2.x, N2.y, N2.z, N2.temp, N2.humi, N2.batt, sys.nodeStatus[2], N3.time, N3.date, N3.x, N3.y, N3.z, N3.temp, N3.humi, N3.batt, sys.nodeStatus[3], N4.time, N4.date, N4.x, N4.y, N4.z, N4.temp, N4.humi, N4.batt, sys.nodeStatus[4], N5.time, N5.date, N5.x, N5.y, N5.z, N5.temp, N5.humi, N5.batt, N4.ath, N5.ath, N4.flow, N5.flow);
        Serial.println(sysBuff.httpStringBuff);
        goto GOOGLESHEETEVENT;


        GOOGLESHEETEVENT :
            String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?" + String(sysBuff.httpStringBuff);

            HTTPClient http;
            http.begin(urlFinal.c_str());
            http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
            int httpCode = http.GET();

            Serial.printf("HTTP CODE : %d\r\n", httpCode);
    
            if(httpCode == 200) {
                Serial.print(F("POST DATA TO GOOGLE SHEET COMPLEAT\r\n"));
            }else {
                sys.sheetFailCounter++;

                if(sys.sheetFailCounter >= 10) {
                    sys.sheetFailCounter = 0;
                    ESP.restart();
                }

                goto GOOGLESHEETEVENT;
            }
        // sys.gpsDataReady = 0;
        // sys.sensorDataReady = 0;
        // sys.processFlag = 0;        // END

    }
} // END MAINPROCESS


// ISR
void IRAM_ATTR isr() {
    sys.processFlag = 1;
    //delay(10);
}


void pulseRTS(void) {
    digitalWrite(RTSpin, HIGH);
    delay(10);
    digitalWrite(RTSpin, LOW);
}





void setup(void) {
    // init valule at startup
    sys.sheetFailCounter = 0;
    sys.WiFiTimeoutCounter = 0;
    sys.gpsDataReady = 0;
    sys.sensorDataReady = 0;
    sys.GNSSCounter = 0;
    sys.failCounter = 0;
    sys.SensorfailCounter = 0;
    sys.processFlag = 0;

    sys.reqNode = 0;
    sys.reqInvalid = 0;

    GPSDATA.lat = 0;
    GPSDATA.lon = 0;

    InitStruct();


    Serial.begin(115200);                                           // UART0 - Debug
    sensorSerial.begin(Sensor_Baud, SERIAL_8N1, 21, 19);            // UART1 - Sensor Master node | Rx = GPIO21 Tx = GPIO19
    gpsSerial.begin(GPS_Baud);                                      // UART2 - GNSS module

    // GPIO init
    pinMode(SensorReadySignal, INPUT_PULLUP);
    attachInterrupt(SensorReadySignal, isr, FALLING);

    pinMode(RTSpin, OUTPUT);


    // Wifi init and debug
    Serial.printf("Connect wifi : %s\r\n", SSID);
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

    Serial.print(F("\nCONNECT WIFI COMPLEAT\r\nSYSTEM READY\r\n"));

    pulseRTS();
} // END Void setup


void loop(void) {

    while(sys.processFlag == 1) {
        mainprocess();

        sys.gpsDataReady = 0;
        sys.sensorDataReady = 0;
        sys.reqNode = 0;
        sys.processFlag = 0;
        
        pulseRTS();                 // END
    }

    

} // END VOID LOOP
