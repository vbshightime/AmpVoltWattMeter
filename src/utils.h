#ifndef UTILS_H
#define UTILS_H

#include "hardwareDefs.h"
#include <SPIFFS.h>
#include "base64.h"
#include "WiFiOTA.h"

// return battery voltage in "V" units

/**
   @brief:
   Helper function to get the RSSI percentage from the RSSI in db of available Networks
   @param:
   RSSI in db
   @return:
   rssi in percentage
*/

int getRSSIasQuality(int RSSI) {
  int quality = 0;
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

/**
   @brief:
   Helper function to get if there are multiple faliure.
   @todo:
   Do we need this we have another helper funtion for the same purpose
   @param:
   Device State enum
   @return:
   true for multiple device event
*/

bool isMultiDeviceEvent(int n) {
  if (n == 0) {
    return false;
  }
  while (n != 1)
  {
    n = n / 2;
    if (n % 2 != 0 && n != 1) {
      return false;
    }
  }
  return true;
}

/**
   @brief:
   Checks for the deviceStateEvent set bits.
   @param:
   deviceEvent      bit position
   @return:
   true if bit is set
*/

bool checkSetBit(int state, int setBit) {
  return (state & (1 << setBit));
}


/**
   @brief:
   Checks for the deviceStateEvent set bits.
   @param:
   deviceEvent      bit position
   @return:
   true if bit is set
*/

bool testBit(uint &bits, int bit)
{
  return (bits & bit);
}

/**
   @brief:
   sets the bit.
   @param:
   deviceEvent      bit position
*/
void setBit(uint &bits, int bit)
{
  bits |= bit;
}

/**
   @brief:
   clears the bit.
   @param:
   deviceEvent      bit position
*/

void clearBit(uint &bits, int bit)
{
  bits &= (~bit);
}

/**
   @brief:
   base 64 encoded key from spiff file name
   @param:
   String Spiff file name
   @return:
   char* keyName
*/
char *getBase64Encode(String fileName) {
  String encoded = base64::encode(fileName);
  char fileNameKey[15];
  Serial.println(encoded);
  strcpy(fileNameKey, encoded.c_str());
  return fileNameKey;
}

int isWiFiOrGSM(){
  if (PSTATE.isGSM == 1){
      RSTATE.isWiFiSelected = 1;
      return 1;
  }
  return 0;
}

String createRemoteFileName(String remoteFWVersion){
  uint8_t mac[6];
  String remoteFileName = getLast3ByteMac(mac,true)+remoteFWVersion+".bin";
  DEBUG_PRINTF("remoteFilename is %s:  \n",remoteFileName.c_str());
  return remoteFileName;
}
#endif
