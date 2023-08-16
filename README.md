# ESP32 Teacking module

| **Description** | **Device** |
| --- | --- |
| Processor | ESP32 WROOM32 |
| GNSS Module | Ublox Neo 7M |

| Requirement | PlatformIO |
|---|---|
| Code Rev. | 1.0A |
| Schematic Rev. | 1.0A |

# Feature
### Released ✅
- read data from GNSS Module
- read data from sensor via UART
- put all data to Google sheet
- Deep sleep  

### in test *️⃣  
- _None_  

### in progress ❓  
- LTE/GNSS module
    - SIMCOM SIM7600E-H1C
    - SIMCOM SIM7020E
    - QUECTEL UC20G
    - QUECTEL EC25
    - QUECTEL BG96
- RS232 Transceiver module  
    - MAX3232
    - ST3232


# Informations
config WiFi SSID and password at
```c
#define SSID        ""
#define PASS        ""

```

### UART 0 
used for program and debug  
_debug parametor_  
| **Description** | **Value** |
| --- | --- |
| Buad rate | 115200 |
| data bit | 8 |
| Parity | None |
| Stop bit | 1 |

### UART 1
used for communicated with Sensor module

| **Description** | **Value** |
| --- | --- |
| Buad rate | 9600 |
| data bit | 8 |
| Parity | None |
| Stop bit | 1 |

### UART 2
used for communicated with GNSS module

| **Description** | **Value** |
| --- | --- |
| Buad rate | 9600 |
| data bit | 8 |
| Parity | None |
| Stop bit | 1 |


# Flowchart
![flowchart](/DOC/flowchart.svg)