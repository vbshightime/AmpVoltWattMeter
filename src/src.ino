#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "hardwareDefs.h"
#include "sensorRead.h"
#include "utils.h"
#include <Wire.h>
#include "WiFiOTA.h"
#include <rom/rtc.h>
#include <esp_wifi.h>
#include <driver/adc.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include "uTimerLib.h"
#include "FS.h"
#include "SPIFFS.h"
#include <esp_task_wdt.h>
#define WDT_TIMEOUT 1000

WiFiUDP ntpUDP;

Ticker logTimer;

DeviceState state;
DeviceState &deviceState = state;
ESPCaptivePortal captivePortal(deviceState);
CloudTalk cloudTalk;
String payloadToSend = "";

NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);


/*void oneMinCallback()
{ if (!RSTATE.isPortalActive && !RSTATE.isAPActive) {
    DEBUG_PRINTLN("in one min callback");
    static int logCount = 0;
    unsigned long timeStamp = 0;
    if (logCount == 0) {
      DEBUG_PRINTLN("logClount is 0 get the updated time stamp");
      timeStamp = timeClient.getEpochTime();
      DEBUG_PRINTF("Timestamp:%lu\n", timeStamp);
      RSTATE.timeToGet = timeStamp;
      payloadToSend = START;
    }
    DEBUG_PRINTLN("creating payload");
    String payload = cloudTalk.createPayload(timeClient);
#ifndef IS_DATA_LOGGING_TO_SPIFF
    payloadToSend += (payload + COMMA);
#else
    String spiffLoggerPath = logDirectoryFromTimeStamp(RSTATE.timeToGet);
    DEBUG_PRINTF("SPIFF LOG PATH: %s\n", spiffLoggerPath);
    appendFile(SPIFFS, spiffLoggerPath.c_str(), payload.c_str());
#endif
    if (logCount >= PSTATE.dataSendFreq) {
      TimerLib.clearTimer();
      logTimer.detach();
      payload = cloudTalk.createPayload(timeClient);
#ifndef IS_DATA_LOGGING_TO_SPIFF
      payloadToSend += (payload + END);
      if (RSTATE.isGSMSelected){
          cloudTalk.sendPayloadPlaneJsonLTE(payloadToSend);
      }
      if (RSTATE.isWiFiSelected){
          cloudTalk.sendPayloadPlaneJsonWiFi(payloadToSend);
      }
      payloadToSend = "";
#else
      readFile(SPIFFS, spiffLoggerPath.c_str());
      if (RSTATE.isGSMSelected){
          cloudTalk.sendPayloadFileLTE(spiffLoggerPath)
      }
      if (RSTATE.isWiFiSelected){
          cloudTalk.sendPayloadPlaneJsonLTE(spiffLoggerPath);
      }
      SPIFFS.remove(spiffLoggerPath.c_str());
#endif
      RSTATE.isTimerDetached = true;
      logCount = 0;
    }
    logCount++;
  }
}*/

std::vector<std::string> readFile(fs::FS &fs, const char * path) {
  DEBUG_PRINTF("Reading file: %s\r\n", path);
  std::vector<std::string> payloadVector;
  File file = fs.open(path);
  DEBUG_PRINTLN("- read from file:");
  while (file.available()) {
    payloadVector.push_back(std::string((file.readStringUntil('\n')).c_str()));
  }
  file.close();
  return payloadVector;
}

void oneMinCallback(){
    DEBUG_PRINTLN("in one min callback");
    if (!RSTATE.isPortalActive && (!RSTATE.isAPActive || RSTATE.isGSMSelected)) {
    uint8_t mac[6];
    unsigned long timeStamp = timeClient.getEpochTime();
    String macStr = getLast3ByteMac(mac, true);
    deviceState.telemetryPayload.amp = RSTATE.amp;
    deviceState.telemetryPayload.vol = RSTATE.vol;
    deviceState.telemetryPayload.power = RSTATE.power;
    deviceState.telemetryPayload.temp_env = RSTATE.temp_env;
    deviceState.telemetryPayload.temp_solar = RSTATE.temp_solar;
    deviceState.telemetryPayload.epoch = timeStamp;
    deviceState.addSensorPayloadToQueue(macStr.c_str(),&deviceState.telemetryPayload);
    }
    if (!deviceState.hasUnprocessedTelemetry()) {
        DEBUG_PRINTLN("NO data to process Try Again");
    }
    if (deviceState.telemetryQueue.size() > PSTATE.dataSendFreq){
        TimerLib.clearTimer();
        logTimer.detach();
        String payload = cloudTalk.preparePayload(deviceState);
         if (RSTATE.isGSMSelected){
             cloudTalk.sendPayloadPlaneJsonLTE(payload);
          }
         if (RSTATE.isWiFiSelected){
             if(!cloudTalk.sendPayloadPlaneJsonWiFi(payload)){
                 DEBUG_PRINTF("SPIFF LOG PATH: %s\n", SPIFF_LOG_PATH);
                 String payloadToAppend = payload+"\n";
                 appendFile(SPIFFS, String(SPIFF_LOG_PATH).c_str(), payload.c_str());
                 RSTATE.missedDataPointCounter++;
             }else{
                  deleteFile(SPIFFS, String(SPIFF_LOG_PATH).c_str());
             }
             if(isFileExists(SPIFFS,String(SPIFF_LOG_PATH).c_str()) && RSTATE.missedDataPointCounter > 0){
                int count = RSTATE.missedDataPointCounter;
                for (int i = 0; i <= count; i++)
                {
                    std::vector<std::string> vectorpayload; 
                    vectorpayload = readFile(SPIFFS,String(SPIFF_LOG_PATH).c_str());
                    if(!cloudTalk.sendPayloadPlaneJsonWiFi(String((vectorpayload[i]).c_str()))) break;
                    RSTATE.missedDataPointCounter--;
                }
                if(RSTATE.missedDataPointCounter == 0){
                    deleteFile(SPIFFS, String(SPIFF_LOG_PATH).c_str());
                }
             }
          }
         RSTATE.isTimerDetached = true;
    }
}

void readSensorCallback() {
  if (!LTCPowerCalculation()) {
    //DEBUG_PRINTLN("Error reading power");
  }
  if (!LTCAmpCalculation()) {
    //DEBUG_PRINTLN("Error reading amp");
  }
  if (LTCVolCalculation()) {
    //DEBUG_PRINTLN("Error reading vol");
  }
  if (!readDSB112()) {
      RSTATE.temp_env = 28.0;
      RSTATE.temp_solar = 29.0;
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  DEBUG_PRINTF("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    DEBUG_PRINTLN("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    DEBUG_PRINTLN("- message appended");
  } else {
    DEBUG_PRINTLN("- append failed");
  }
  file.close();
}

bool isFileExists(fs::FS &fs, const char * path) {
  DEBUG_PRINTF("is %s file exists\r\n", path);
  bool isExists = fs.exists(path);
  return isExists;
}


void deleteFile(fs::FS &fs, const char * path){
    DEBUG_PRINTF("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        DEBUG_PRINTLN("- file deleted");
    } else {
        DEBUG_PRINTLN("- delete failed");
    }
}


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  SerialAT.begin(115200,SERIAL_8N1,SERIAL2_RXD,SERIAL2_TXD,false);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  Wire.begin();
  pinMode(TEMP_SOLAR_GPIO, INPUT);
  pinMode(TEMP_ENV_GPIO, INPUT);
  pinMode(GPIO_MODEM_RESET,OUTPUT);
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(GPIO_MODEM_RESET,HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  uint8_t mac[6];
  String remoteFileName = getLast3ByteMac(mac,true);
  Serial.println(remoteFileName);
  DEBUG_PRINTLN("This is Futr Solar Wireless Sensor");
  
  if (!EEPROM.begin(EEPROM_STORE_SIZE))
  {
    DEBUG_PRINTLN("Problem loading EEPROM");
  }

  deviceState.load() ? DEBUG_PRINTLN("Values Loaded") : DEBUG_PRINTLN("EEPROM Values not loaded"); 
  if (PSTATE.isOtaAvailable == 1) {
    PSTATE.isOtaAvailable = 0;
    deviceState.store();
    // TODO:: check if everything is written before reset
    //performOTA calls reset if successful
    bool rc = performOTA();
    if (!rc) {
      DEBUG_PRINTLN("could not do an ota update");
    }
  }
  
  isWiFiOrGSM() ? RSTATE.isGSMSelected=1 : RSTATE.isWiFiSelected=0;

  DEBUG_PRINTF("The reset reason is %d\n", (int)rtc_get_reset_reason(0));
  if ( ((int)rtc_get_reset_reason(0) == 12) || ((int)rtc_get_reset_reason(0) == 1))  { // =  SW_CPU_RESET
    RSTATE.isPortalActive  = true;
    if (!APConnection(AP_MODE_SSID)) {
      DEBUG_PRINTLN("Error Setting Up AP Connection");
      return;
    }
    delay(100);
    captivePortal.servePortal(true);
    captivePortal.beginServer();
    delay(100);
    timeClient.begin();
    timeClient.update();
  }
  DSB112Init();
  LTCInit();

  logTimer.attach_ms(SECS_TO_LOG * MILLI_SECS_MULTIPLIER, oneMinCallback);
  analogSetAttenuation(ADC_0db);
  TimerLib.setInterval_s(readSensorCallback, READ_SENSOR_CALLBACK_TIME_IN_S);

}

void loop()
{
  if (!RSTATE.isPortalActive && RSTATE.isWiFiSelected) {
    reconnectWiFi(PSTATE.staSSID, PSTATE.staPass, 300, timeClient);
  }
  
  if (!RSTATE.isPortalActive && RSTATE.isGSMSelected && !cloudTalk.isNetworkConnected()){
      TimerLib.clearTimer();
      logTimer.detach();
      if(cloudTalk.isSimReady()){
         DEBUG_PRINTLN("SIM_READY");}else{cloudTalk.tinyGSMInit();} 
      DEBUG_PRINTLN("Wait for network");
      for (int i=0; i<=MODEM_RETRY; i++){
          DEBUG_PRINTLN("...");    
          if (cloudTalk.waitFormodemNetwork()) break;
          cloudTalk.tinyGSMInit();
      }
      DEBUG_PRINTLN("Wait for GPRS Connection");
      for (int i=0; i<=MODEM_RETRY; i++){
          DEBUG_PRINTLN("...");
          if (cloudTalk.waitForgprsConnection()) break;
          cloudTalk.tinyGSMInit();
    }
    RSTATE.isTimerDetached = true;
  }
  
  if (RSTATE.isTimerDetached) {
    logTimer.attach_ms(SECS_TO_LOG * MILLI_SECS_MULTIPLIER, oneMinCallback);
    TimerLib.setInterval_s(readSensorCallback, READ_SENSOR_CALLBACK_TIME_IN_S);
    RSTATE.isTimerDetached = false;
  }
  if (millis() - RSTATE.startPortal >= SECS_PORTAL_WAIT * MILLI_SECS_MULTIPLIER && RSTATE.isPortalActive)
  {
    DEBUG_PRINTLN("Portal is over");
    RSTATE.isPortalActive = false;
    deviceState.store();
    if(isWiFiOrGSM()){
        RSTATE.isGSMSelected=0;
        DEBUG_PRINTLN("GSM Selected");
    }else{
        RSTATE.isWiFiSelected=1;
        DEBUG_PRINTLN("WiFi Selected");
    }  
  }
}

bool performOTA()
{

  DEBUG_PRINTF("isOtaAvailable in isOtaAvailable is %d", PSTATE.isOtaAvailable);
  
  DEBUG_PRINTF("full Local File name in %s (inPerformOta())", String(SPIFF_FW_PATH).c_str());
  bool rc = writeOTA(SPIFF_FW_PATH);
  if (!rc) {
    DEBUG_PRINTF("writing file: %s to flash failed\n", String(SPIFF_FW_PATH).c_str());
    return false;
  }

  delay(4000); // NOTE :: make sure that flash has settled anyway we use this only on powered gateway
  //storeAndRestart();
  ESP.restart();
  return true;
}
