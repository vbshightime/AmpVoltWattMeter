#ifndef CAPTIVE_H_
#define CAPTIVE_H_

#include <pgmspace.h>
#include "deviceState.h"
#include "utils.h"
#include "hardwareDefs.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#define  RESPONSE_LENGTH 200
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);

//Response payload for WiFi Credentials html submission
char credResponsePayload[RESPONSE_LENGTH];

//Response payload for call html Submission
char callResponsePayload[RESPONSE_LENGTH];

//Response payload for apn html submission
char apnResponsePayload[RESPONSE_LENGTH];

//Response payload for clearing EEPROM html submission
char clearResponsePayload[RESPONSE_LENGTH];

/****HTML Page for configuring WiFi and LTE****/

const char HTTP_FORM_WIFISET[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>Futr</title>
	<style>body { background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style></head>
<body><center>
	<h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">Futr Energy</h1>
	<br><label style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;font-size: 24px;padding-top: 5px;padding-bottom: 10px;">Configure Device Settings</label><br><br>
	<FORM action="/cred" method= "get">
		<P><label style="font-family:Times New Roman">WiFi SSID</label><br><input maxlength="30px" type = "text" name="ssid" id="ssid" placeholder= "SSID" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;>
		<br><label style="font-family:Times New Roman">WiFi Password</label><br><input maxlength="30px" type = "text" name="pass" id="pass" placeholder= "Password" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;><br>
		</P>
		<INPUT type="submit"><style>input[type="submit"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style><br><br>
	</FORM><br>
    <h3 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 18px;font-style: oblique">For more info visit https://futr.energy/</label>
</center></body></html> )rawliteral";


/****HTML Page for configuring Data Sending Frequency****/

const char HTTP_FORM_SET_CALLIBRATION_FACTOR[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset="utf-8" />
    <style>body{ background-color: #0067B3  ; font-family: Arial, Helvetica, Sans-Serif }</style>
    </head><title>Futr</title><body><div class="container" align="center" ><center><br>
      <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">Futr Energy</h1>
      <FORM action="/call" method= "get">
        <h3 style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;padding-bottom: 20px;text-align: center;font-size: 20px">Set Upload Time</h3>
            <select name="tCall" id="tCall_id" style="border:2px;  padding: 5px 100px; display: inline-block; margin-top:5px;border: 2px solid #3498DB; border-radius: 4px;box-sizing: border-box;">
              <option value="30">30 mins </option>
              <option value="20">20 mins </option>
              <option value="10">10 mins </option>
              <option value="5">5 mins </option>
              <option value="2">2 mins </option>
            </select><br><br>
            <INPUT type="submit" > <style>input[type="submit"]{background-color: #3498DB;border: none;color: white;padding:10px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 12px;}</style></div>
        </div>
        </FORM>
        <h3 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 18px;font-style: oblique">For more info visit https://futr.energy/</label>
        </center></body></html>)rawliteral";


/****HTML Page for configuring APN Configuration****/
const char HTTP_FORM_SET_APN[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>Futr</title>
    <style>body { background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style></head>
    <body><center>
        <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">Futr Energy</h1>
        <br><label style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;font-size: 24px;padding-top: 5px;padding-bottom: 10px;">Configure APN for GSM</label><br><br>
        <FORM action="/apn" method= "get">
            <P>
                <input type="radio" name="apn_id" value="apn" id="apn" onclick="ShowHideIsAPN(this)" style="color:blue;" > Configure device to operate as GSM &nbsp&nbsp
                <input type="radio" name="apn_id" value="wifi" id="wifi" style="color:blue;" > Configure device to operate as WiFi <br>
                <div id="isAPN" style="display: none">
                    <input type="checkbox" id="gprs_required" name="gprs_required" value="gprs">GPRS Credentials Required <br><br>
                    <label style="font-family:Times New Roman">APN</label><br><input maxlength="30px" type = "text" name="apn" id="apn" placeholder= "Vendor APN" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;><br>
                    <br><label style="font-family:Times New Roman">GPRS Username</label><br><input maxlength="30px" type = "text" name="gprs_user" id="gprs_user" placeholder= "GPRS User" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box"><br>
                    <br><label style="font-family:Times New Roman">GPRS Password</label><br><input maxlength="30px" type = "text" name="gprs_pass" id="gprs_pass" placeholder= "GPRS PassKey" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box"><br>
                  </div>
            </P>
            <INPUT type="submit"><style>input[type="submit"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style><br><br>
        </FORM>
        <h3 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 18px;font-style: oblique">For more info visit https://futr.energy/</label>
    </center>
</body></html>
<script>
  function ShowHideIsAPN() {
      var x = document.getElementById("isAPN");
    if (x.style.display === "none") {
      x.style.display = "block";
    } else {
      x.style.display = "none";
    }
  }
  </script>
    )rawliteral";


/****HTML Page for configuring APN Configuration****/
const char HTTP_FORM_CLEAR_EEPROM[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>Futr</title>
    <style>body { background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style></head>
    <body><center>
        <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">Futr Energy</h1>
        <br><label style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;font-size: 24px;padding-top: 5px;padding-bottom: 10px;">Clear memory</label><br><br>
        <FORM action="/clear" method= "get">
            <P><input type="checkbox" name="clear_name" value="clear" id="clear_id" style="color:blue;">Confirm to clear memory<br>
            </P>
            <INPUT type="submit"><style>input[type="submit"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style><br><br>
        </FORM>
        <h3 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 18px;font-style: oblique">For more info visit https://futr.energy/</label>
    </center></body></html>
    )rawliteral";


DeviceState _devState;
class ESPCaptivePortal
{ 
  public:
    // Warning:: requies device state to  be global and longer living
    // than this
    ESPCaptivePortal(DeviceState& devState) {
          _devState = devState;
    }


    /**
       @brief:
       begins the Async WebServer and AsyncOTA
    */

    void beginServer() {
      AsyncElegantOTA.begin(&server);    // Start ElegantOTA
      DEBUG_PRINTLN("Starting the captive portal. You can configure ESp32 values using portal");
      server.begin();
    }

    /**
       @brief:
       Kills the Async WebServer
    */
    void endPortal() {
      DEBUG_PRINTLN("Ending the captive portal");
      server.reset();
    }

    /**
       @brief:
       Serves the portal
       @param:
       AP active flag
    */
    void servePortal ( bool isAPActive ) {

      /*** Serves WiFi Credential Portal ***/

      server.on("/cred", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("ssid") && request->hasParam("pass")) {

          PSTATE.staSSID = request->getParam("ssid")->value();
          DEBUG_PRINTF("ssid stored %s\t\n", PSTATE.staSSID.c_str());
          PSTATE.staPass = request->getParam("pass")->value();
          DEBUG_PRINTF("Pass Stored %s\t\n", PSTATE.staPass.c_str());
          snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"staSSID\":\"%s\",\"staPass\":\"%s\"}", (PSTATE.staSSID).c_str(), (PSTATE.staPass).c_str());
          request->send(200, "application/json", credResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_WIFISET);
        }
      });

      /*** Serves data sending Portal ***/

      server.on("/call", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("tCall")) {
          PSTATE.dataSendFreq = (request->getParam("tCall")->value()).toInt();
          DEBUG_PRINTF("dataSendFrequency %d\t\n", PSTATE.dataSendFreq);
          snprintf(callResponsePayload, RESPONSE_LENGTH, "{\"dataSendFreq\":%d}", PSTATE.dataSendFreq);
          request->send(200, "application/json", callResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_SET_CALLIBRATION_FACTOR);
        }
      });

      
      /*** Serves apn Portal ***/

      server.on("/apn", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("apn_id")) {
            String isApnOrWifi = request->getParam("apn_id")->value();
          if (isApnOrWifi == "wifi"){
              DEBUG_PRINTLN("WiFi Selected");
              PSTATE.isGSM = 0;
              
              snprintf(apnResponsePayload, RESPONSE_LENGTH, "{\"response\":\"%s\"}", "Configured as wifi");
              request->send(200, "application/json", apnResponsePayload);
          }
           
          if(request->hasParam("apn")){
              DEBUG_PRINTLN("GSM Selected");
              PSTATE.isGSM = 1;
              PSTATE.isgprsCredRequired = 0;
              if(request->hasParam("gprs_required")){
                  PSTATE.isgprsCredRequired = 1;
                  PSTATE.gprsAPN = request->getParam("apn")->value();
                  PSTATE.gprsUser = request->getParam("gprs_user")->value();
                  snprintf(apnResponsePayload, RESPONSE_LENGTH, "{\"apn\":\"%s\",\"gprsUser\":\"%s\",\"gprsPass\":\"%s\"}", (PSTATE.gprsAPN).c_str(),(PSTATE.gprsUser).c_str(),(PSTATE.gprsPass).c_str());
                  request->send(200, "application/json", apnResponsePayload);  
              }
              PSTATE.gprsAPN = request->getParam("apn")->value();
              snprintf(apnResponsePayload, RESPONSE_LENGTH, "{\"apn\":\"%s\"}", (PSTATE.gprsAPN).c_str());
              request->send(200, "application/json", apnResponsePayload);
            }
        }
  else {
          request->send_P(200, "text/html", HTTP_FORM_SET_APN);
        }
      });

        server.on("/clear", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("clear_name")) {
              PSTATE.isGSM = 0;
              PSTATE.isgprsCredRequired = 0;
              PSTATE.gprsAPN = "";
              PSTATE.gprsUser = "";
              PSTATE.gprsPass = "";
              PSTATE.dataSendFreq = 0;
              PSTATE.staPass = "";
              PSTATE.staSSID = "";
              PSTATE.isOtaAvailable = 0;
              PSTATE.newfWVersion = 0;
            snprintf(callResponsePayload, RESPONSE_LENGTH, "{\"clear\":\"%s\"}", "Cleared EEPROM");
            if(_devState.store()){
                snprintf(callResponsePayload, RESPONSE_LENGTH, "{\"clear\":\"%s\"}", "Cleared EEPROM, Restarting the device");
                ESP.restart();
            }

            request->send(200, "application/json", callResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_CLEAR_EEPROM);
        }
      });
      /*** Serves URL not found sending Portal ***/
      server.onNotFound(_handleNotFound);
      yield();
    }

    /**
       @brief:
       Helper funtion for unexpected error
       @param:
       AsyncWebServerRequest
    */
    static void _handleNotFound(AsyncWebServerRequest *request)
    {
      String message = "File Not Found\n\n";
      request->send(404, "text/plain", message);
    }

};


#endif
