#ifndef CLOUDTALK_H
#define CLOUDTALK_H


#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "FS.h"
#include "SPIFFS.h"
#include "WiFiOTA.h"
#include "deviceState.h"
#include "hardwareDefs.h"
#include "utils.h"
#include "UDHttp.h"
#include "Ticker.h"
#define TINY_GSM_MODEM_SIM800
#include "TinyGsmClient.h"

String remoteFWRev = "0";


#define JSON_MSG_MAX_LEN          256

DynamicJsonDocument jsonDocument(1024);

char urlmessageSend[] = "http://ftp-server.visualai.io/data/";

const char urlGSMServer[] = "ftp-server.visualai.io";

const char urlGSMPathParameter[] = "/data/";

// payload structure for OTA result update to cloud
const char OTA_JSON_PAYLOAD_BODY[] = "{\"deviceId\":\"%s\",\"firmwareVersion\":\"%s\"}";
char otaPayload[JSON_MSG_MAX_LEN];

const char OtaUrl[] = "http://ftp-server.visualai.io/data/fw-updates/%s"; //URL for ota file download


TinyGsm         modem(SerialAT);
TinyGsmClient   client(modem);        

File fileToRead;

int responsef(uint8_t *buffer, int len) {
  Serial.printf("response: %s\n", buffer);
  Serial.printf("length: %d\n", len);
  return 0;
}

int rdataf(uint8_t *buffer, int len) {
  //read file to upload
  if (fileToRead.available()) {
    return fileToRead.read(buffer, len);
  }
  return 0;
}

int wdataf(uint8_t *buffer, int len) {
  //write downloaded data to file
  return fileToRead.write(buffer, len);
}

void progressf(int percent) {
  Serial.printf("percent:%d\n", percent);
}

class CloudTalk {

  public:
    /**
       @brief: Create http link and send the device form Data file to cloud using WiFi
       @param: SPIFF Logger Path
       @return: true when everything works right
    */
    bool sendPayloadFileWiFi(String loggerPath)
    {
      fileToRead = SPIFFS.open(loggerPath.c_str());
      if (!fileToRead) {
        Serial.println("can not open file!");
        return false;
      }
      UDHttp udh;
      char spiffLogPathCpyFromString[30];
      strcpy(spiffLogPathCpyFromString, loggerPath.c_str());
      char *keyName = getBase64Encode(loggerPath);
      int returnCode = udh.upload(urlmessageSend, (char*)spiffLogPathCpyFromString, fileToRead.size(), rdataf, progressf, responsef, keyName);
      DEBUG_PRINTF("return http code %d:", returnCode);
      fileToRead.close();
      return true;
    }


    /**
       @brief: Create http link and send the device payload JSON to cloud using WiFi
       @param: Payload to  send
       @return: true when everything works right
    */

    bool sendPayloadPlaneJsonWiFi(String payload) {
      String retJson;
      int httpCode;
      HTTPClient http;
      http.begin(urlmessageSend);//zx     //test
      http.addHeader("Content-Type" , "application/json");  //Specify content-type header
      DEBUG_PRINTLN(payload);
      httpCode = http.POST(payload);     //Send the request
      yield();
      if (httpCode == HTTP_CODE_OK && httpCode > 0) {
        retJson = http.getString();   //Get the response payload
        DEBUG_PRINTLN(retJson);
      } else {
        DEBUG_PRINTF("[HTTP] GET... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
        return false;
      }
      http.end();
      if(!extractFWVersion(retJson)){
          return true;
      }else{
        String remoteFileName = createRemoteFileName(remoteFWRev);
          size_t filesize = fetchOTAFile(remoteFileName,SPIFF_FW_PATH);
          ESP.restart();
      }
      
    }

    /**
      @brief: Create http link and send the device payload JSON to cloud using LTE
      @param: Payload to  send
      @return: true when everything works right
    */
    bool sendPayloadFileLTE(String loggerPath) {
        fileToRead = SPIFFS.open(loggerPath.c_str());
      if (!fileToRead) {
        Serial.println("can not open file!");
        return false;
      }
      UDHttp udh;
      char spiffLogPathCpyFromString[30];
      strcpy(spiffLogPathCpyFromString, loggerPath.c_str());
      char *keyName = getBase64Encode(loggerPath);
      int returnCode = udh.upload(urlmessageSend, (char*)spiffLogPathCpyFromString, fileToRead.size(), rdataf, progressf, responsef, keyName);
      DEBUG_PRINTF("return http code %d:", returnCode);
      fileToRead.close();
      return true;
    }
    /**
       @brief: Create http link and send the device form Data file to cloud using LTE
       @param: Payload to  send
       @return: true when everything works right
    */
    void sendPayloadPlaneJsonLTE(String payload) {
      String retJson;
      int httpCode;
      unsigned long httpTimeout = millis();
      DEBUG_PRINTLN(payload);
      client.print(String("POST ") + urlGSMPathParameter + " HTTP/1.1\r\n");
      Serial.println(String("POST ") + urlGSMPathParameter + " HTTP/1.1\r\n");
      client.print(String("Host: ") + urlGSMServer + "\r\n");
      Serial.println(String("Host: ") + urlGSMServer + "\r\n");
      client.println("Connection: close");
      Serial.println("Connection: close");
      client.println("Content-Type: application/json");
      Serial.println("Content-Type: application/json");
      client.print("Content-Length: ");
      Serial.println("Content-Length: ");
      client.println(payload.length());
      Serial.println(payload.length());
      client.println();
      client.println(payload);
      DEBUG_PRINTLN(payload);
      yield();
      while(client.connected() && millis()- httpTimeout < HTTP_CONNEC_TIMEOUT_IN_MS){
            while(client.available()){
                  char clientRead = client.read();
                  DEBUG_PRINTF("Client Resposne: %c\n",clientRead);
                  httpTimeout = millis();
              }  
        }

        client.stop();


      /*if (httpCode == HTTP_CODE_OK && httpCode > 0) {
        retJson = http.getString();   //Get the response payload
        DEBUG_PRINTLN(retJson);
      } else {
        DEBUG_PRINTF("[HTTP] GET... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
        return false;
      }*/
    }

    /**
       @brief: Create message payload
       @return: message payload array
    */

    String createPayload(PayloadQueueElement *telemeteryElement) const
    {
      char messageCreatePayload[JSON_MSG_MAX_LEN];
      String macStr = String(telemeteryElement->_mac);
      SensorPayload *payload = telemeteryElement->_payload;
      DEBUG_PRINTF("%s", macStr);
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp_env\": \"%.1f\",\"temp_solar\": \"%.1f\",\"vol\": \"%.2f\",\"amp\": \"%.2f\",\"watt\": \"%.2f\",\"timestamp\":\"%lu\"}",
               macStr.c_str(),
               payload->temp_env,
               payload->temp_solar,
               payload->vol,
               payload->amp,
               payload->power,
               payload->epoch
              );

      return String(messageCreatePayload);
    }


    String preparePayload(DeviceState &devState) const
    {
        SensorPayload *payload = nullptr;
        String preparedPayload = START;
        while(devState.hasUnprocessedTelemetry()){
              DEBUG_PRINTLN("hasUnprocessedTelemetry");
              PayloadQueueElement *telemeteryElement = devState.telemetryQueue.pop();
              preparedPayload += createPayload(telemeteryElement) + COMMA;
        }
        int lastIndex = preparedPayload.lastIndexOf(COMMA);
        preparedPayload[lastIndex] = ']';
        return preparedPayload;
    }

    void tinyGSMInit(){
        DEBUG_PRINTLN("Initializing modem");
        modem.restart();
        delay(10000);
        String modemInfo = modem.getModemInfo();
        DEBUG_PRINTF("ModemInfo: %s\n",modemInfo);
        
    }

    bool isSimReady(){
      int simStatus = modem.getSimStatus();
      switch (simStatus){
      
      case 0:
        DEBUG_PRINTLN("SIM_ERROR");
        return simStatus;
        break;
      case 1:
        DEBUG_PRINTLN("SIM_READY");
        return simStatus;  
      case 2:
        DEBUG_PRINTLN("SIM_LOCKED");
        return 0;
      case 3:
        DEBUG_PRINTLN("SIM_ANTITHEFT_LOCKED");
        return 0;  
      default:
        break;
    }
  }
  
  bool waitFormodemNetwork(){
    unsigned long start_millis = millis();
    unsigned long modem_timeout = 20000;
    static int modem_retries = 0;
    if(modem.isNetworkConnected()){
      DEBUG_PRINTLN("Network Connected");
      return true;
    }
    if(modem_retries>1){
      DEBUG_PRINTLN("reached maximum retries restarting");
      ESP.restart();
      return false;
    }
    while(millis() <(start_millis + modem_timeout) && !modem.waitForNetwork() && (modem_retries<2)){
        delay(5000);
    }

    if(modem.isNetworkConnected()){
      DEBUG_PRINTLN("Network Connected");
      return true;
    }
    
    modem_retries++;
    return false;
  }

  bool waitForgprsConnection(){
    unsigned long start_millis = millis();
    unsigned long gprs_timeout = 20000;
    static int gprs_retries = 0;
    if(modem.isGprsConnected()){
      DEBUG_PRINTLN("GPRS Connected");
      return true;
    }
    if(gprs_retries>1){
      DEBUG_PRINTLN("reached maximum retries for gprs connect");
       ESP.restart();
      return false;
    }
    const char *gprs_user = PSTATE.gprsUser.c_str();
    const char *gprs_pass = PSTATE.gprsPass.c_str();
    const char *apn = PSTATE.gprsAPN.c_str();
    if(PSTATE.isgprsCredRequired == 1){
        DEBUG_PRINTF("Connecting to APN: %s, gprs_user: %s, gprs_pass: %s\n",apn,gprs_user,gprs_pass);
        while(millis() <(start_millis + gprs_timeout) && !modem.gprsConnect(apn,gprs_user,gprs_pass) && (gprs_retries<2)){
        delay(5000);
    }
    }else{
        DEBUG_PRINTF("Connecting to APN: %s\n",apn);
        while(millis() <(start_millis + gprs_timeout) && !modem.gprsConnect(apn,"","") && (gprs_retries<2)){
        delay(5000);
    }
    }
    if(modem.isGprsConnected()){
      DEBUG_PRINTLN("GPRS Connected");
      return true;
    }
    gprs_retries++;
    return false;
  }


  bool isNetworkConnected(){
        return modem.isNetworkConnected() && modem.isGprsConnected();
    }

  

  bool extractFWVersion(String load) const
    {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, load);
      if (error) {
        DEBUG_PRINTLN("failed to deserialize");
        return false;
      }
      if(doc.containsKey("isUpdateRequired")){
          int otaInitState = doc["isUpdateRequired"];
          int fwRev = doc["FW_REV"];
          remoteFWRev = String(fwRev);
          DEBUG_PRINTF("isUpdateRequired status is %d: \n",otaInitState);
          if(otaInitState == 1){
             PSTATE.isOtaAvailable = otaInitState;
             deviceState.store();
             return true;
          }
          return false;
      }
      DEBUG_PRINTLN("Does not contain isUpdateRequired");
      return false;
  }
  
   // fetch ota file and store in spiffs
    size_t fetchOTAFile(const String &remoteFilename, const String &localFilename) const
    {

      size_t otaFileSize = 0;

      File f = SPIFFS.open(localFilename.c_str(), "w");

      if (!f)
      {
        return 0;
      }

      HTTPClient http;
      char urlWithFilename[JSON_MSG_MAX_LEN];
      snprintf(urlWithFilename, JSON_MSG_MAX_LEN, OtaUrl, remoteFilename.c_str());
      DEBUG_PRINTLN(urlWithFilename);
      int connected = http.begin(urlWithFilename);

      if (!connected)
      {
        DEBUG_PRINTLN("being failed to connect");
      }

      int httpCode = http.GET();
      if (httpCode > 0 && httpCode == HTTP_CODE_OK)
      {
        otaFileSize = http.writeToStream(&f);
        if (otaFileSize == 0)
        {
          DEBUG_PRINTF("incorrect file written, actual size %u\n", otaFileSize);
          f.close();
          SPIFFS.remove(localFilename.c_str());
          return 0;
        }
      }
      else
      {
        DEBUG_PRINTF("[HTTP] GET... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
        f.close();
        SPIFFS.remove(localFilename.c_str());
        return 0;
      }

      DEBUG_PRINTLN("File Written");
      http.end();
      f.close();
      DEBUG_PRINTLN(otaFileSize);
      return otaFileSize;
    }
};

#endif
