# Solar Monitoring

### Libraries used:

#### library for ds18b20
1. Dallastemperature 3.8.0 Miles Burton <https://github.com/milesburton/Arduino-Temperature-Control-Library>
2. oneWire 2.3.4 Jim Studt, Tom Pollard, Robin James, Glenn Trewitt, Jason Dangel <https://github.com/PaulStoffregen/OneWire>

#### library for Timer interrupts
1. uTimerLib 1.4.0 Naguissa <https://github.com/Naguissa/uTimerLib>
2. Ticker  <https://github.com/esp8266/Arduino/tree/master/libraries/Ticker>


#### Json
1. Arduino Json 5.5.0 Benoit Blenchon <https://github.com/bblanchon/ArduinoJson>

#### LTC2946 library for energy monitoring
1. LTC2946 by vbshightime https://github.com/vbshightime/LTC2946


#### Linked List 
1. https://github.com/ivanseidel/LinkedList
#### Multipart upload
1. UDHttp by  nhatuan84  https://github.com/nhatuan84/esp32-upload-download-multipart-http
##### Edits done for compatibility:
in UDHttp.h key is added as public variable after line 25

```    
public:
    UDHttp();
    ~UDHttp();
    char *key;
    int upload(char *uploadUrlHandler, char *fileName, int sizeOfFile, DataCb dataCb, ProgressCb progressCb, DataCb responseCb);
    int download(char *downloadUrl, DataCb dataCb, ProgressCb progressCb);
    };
```

#### Spiff logging
1. Effortless-SPIFFS by thebigpotatoe https://thebigpotatoe.github.io/Effortless-SPIFFS/
##### Edits done for compatibility:
in line 202 of Effortless_SPIFFS.h i have added println instead of print to append to new line

```
virtual bool appendFile(const char* _filename, const char* _input) {
    // Open the file in write mode and check if open
    File currentFile = getFile(_filename, "a");
    if (currentFile) {
    // Print the input string to the file
    if (currentFile.println(_input)) {
    currentFile.close();
    return true;
    } else {
        ESPIFFS_DEBUG("[saveFile] - Failed to append any bytes to file: ");
        ESPIFFS_DEBUGLN(_filename);
    }
    }
    return false;
    }
```
## Releases
### Release version  1.1.0
    
| HW_REV  |   FW_REV  | Minor_revesions |
| ------- | --------- | --------------- |
| 1       | 1         | 0               |
 
### Release Info
HW_REV 1
1. ESP32 SoC
2. LTC2946 for Energy monitoring
3. LTE SIM800

FW_REV 1
1. SPIFF logging 
2. sending Multipart form data using WiFi
3. sending Multipart form data using LTE
4. sending JSON Response using WiFi
5. sending Multipart form data using LTE

