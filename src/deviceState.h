#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <EEPROM.h>
#include "LinkedList.h"
#include "hardwareDefs.h"
#include "sensorPayload.h"


//advance declaration
class PersistantStateStorageFormat;


/**
   @brief:
   Class for keeps the copy of the senor payload to add it to the linked list
*/
class PayloadQueueElement
{
  public:
    // this will keep a copy of sensor payload, the ptr doens't have to be valid after the call
    PayloadQueueElement(const char *mac, SensorPayload *payload)
    {
      _mac = strdup(mac);
      _payload = copyPayloadObject(payload);
    }
    ~PayloadQueueElement()
    {
      freePayloadObject(_payload);
      _payload = nullptr;
      free(_mac);
      _mac = nullptr;
    }

    char * _mac;
    SensorPayload* _payload;
};


/**
   @brief:
   Class for runtime Device status
*/
class RunTimeState {
  public:
    RunTimeState():
      isWiFiConnected(false),
      isAPActive(false),
      isPortalActive(false),
      startPortal(0),
      macAddr(DEVICE_ID_DEFAULT),
      temp_env(INVALID_TEMP_ENV_READING),
      temp_solar(INVALID_TEMP_SOLAR_READING),
      power(INVALID_WATT_READING),
      amp(INVALID_AMP_READING),
      vol(INVALID_VOL_READING),
      timeToGet(0),
      isTimerDetached(false),
      isGSMSelected(0),
      isWiFiSelected(1),
      missedDataPointCounter(0)
    {

    }
    bool isWiFiConnected;
    bool isAPActive;
    bool isPortalActive;
    unsigned long startPortal;
    String macAddr;
    int batteryPercentage;
    float temp_env;
    float temp_solar;
    float power;
    float amp;
    float vol;
    unsigned long timeToGet;
    bool isTimerDetached;
    bool isGSMSelected;
    bool isWiFiSelected;
    int missedDataPointCounter;
};

/**
   @brief:
   Class EEPROM device format
*/

class PersistantState {
  public:
    PersistantState() : staSSID(WAN_WIFI_SSID_DEFAULT),
      staPass(WAN_WIFI_PASS_DEFAULT),
      dataSendFreq(DATA_SEND_FREQ_DEFAULT),
      isOtaAvailable(0),
      newfWVersion(0),
      isGSM(0),
      gprsAPN(WAN_GPRS_APN_DEFAULT),
      gprsUser(WAN_GPRS_USER_DEFAULT),
      gprsPass(WAN_GPRS_PASS_DEFAULT),
      isgprsCredRequired(0)
    {

    }

    PersistantState(const PersistantStateStorageFormat& persistantStore);

    bool operator==(const PersistantState& rhs) {
      return ((staSSID == rhs.staSSID) &&
              (staPass == rhs.staPass) &&
              (deviceId == rhs.deviceId) &&
              (isOtaAvailable == rhs.isOtaAvailable) &&
              (newfWVersion == rhs.newfWVersion) &&
              (dataSendFreq == rhs.dataSendFreq) &&
              (isGSM == rhs.isGSM)&&
              (gprsAPN == rhs.gprsAPN)&&
              (gprsUser == rhs.gprsUser)&&
              (gprsPass == rhs.gprsPass)&&
              (isgprsCredRequired == rhs.isgprsCredRequired));
    }
    // public data members
    String staSSID;
    String staPass;
    String deviceId;
    int dataSendFreq;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
    uint8_t isGSM;
    String gprsAPN;
    String gprsUser;
    String gprsPass;
    uint8_t isgprsCredRequired;


};

/**
   @brief:
   Structure EEPROM Storage format
   this shadwos persistnat state structure in every way except that
   it replaces complex data types with POD types, complex data can't be directly stored and
   read back as is. It was required because we don't want to deal with c strings in rest of the code.
*/

struct PersistantStateStorageFormat {
  public:
    PersistantStateStorageFormat() {}
    PersistantStateStorageFormat(const PersistantState &persistantState);
    char version[8];
    char staSSID[30];
    char staPass[30];
    char gprsAPN[30];
    char gprsUser[30];
    char gprsPass[30];
    int dataSendFreq;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
    uint8_t isGSM;
    uint8_t isgprsCredRequired; 
} __attribute__ ((packed));

PersistantState::PersistantState(const PersistantStateStorageFormat& persistantStore)
{
  staSSID = String(persistantStore.staSSID);
  staPass = String(persistantStore.staPass);
  gprsAPN = String(persistantStore.gprsAPN);
  gprsUser = String(persistantStore.gprsUser);
  gprsPass = String(persistantStore.gprsPass);
  dataSendFreq = persistantStore.dataSendFreq;
  isOtaAvailable = persistantStore.isOtaAvailable;
  newfWVersion = persistantStore.newfWVersion;
  isgprsCredRequired = persistantStore.isgprsCredRequired;
  isGSM = persistantStore.isGSM;
}

PersistantStateStorageFormat::PersistantStateStorageFormat(const PersistantState &persistantState)
{
  strcpy(version, EEPROM_STORAGE_FORMAT_VERSION);
  strcpy(staSSID, persistantState.staSSID.c_str());
  strcpy(staPass, persistantState.staPass.c_str());
  strcpy(gprsAPN, persistantState.gprsAPN.c_str());
  strcpy(gprsUser, persistantState.gprsUser.c_str());
  strcpy(gprsPass, persistantState.gprsPass.c_str());
  dataSendFreq = persistantState.dataSendFreq;
  isOtaAvailable = persistantState.isOtaAvailable;
  newfWVersion = persistantState.newfWVersion;
  isGSM = persistantState.isGSM;
  isgprsCredRequired = persistantState.isgprsCredRequired;
}


class DeviceState
{
  public:
    // public data members
    RunTimeState        runTimeState;
    PersistantState     persistantState;
    SensorPayload       telemetryPayload;


    DeviceState() {
      /**
         @todo:There was a problem in begining it here
      */
      //EEPROM.begin(EEPROM_STORE_SIZE);
    }

    //DeviceState destructor
    ~DeviceState() {
      EEPROM.end();
    }

    /**
       @brief:add recent data to queue
       @param: mac address, Sensor payload, NTPClient 
    */
    
    void addSensorPayloadToQueue(const char *mac, SensorPayload *payload)
    { 
      
      if (!mac || !payload) {
        DEBUG_PRINTLN("invalid data in payload");
        return;
      }

      DEBUG_PRINTLN("Allocating new queue element");
      PayloadQueueElement * telemetryElement = new PayloadQueueElement(mac, payload);
      
      if (!telemetryElement->_payload) {
        DEBUG_PRINTF("copy of payload had failed, not queuing\n");
        delete telemetryElement;
        return;
      }
      
      DEBUG_PRINTLN("adding new telemetry data to queue");
      telemetryQueue.add(telemetryElement);
    }

    /**
       @brief:check whether there is payload data in queue
    */
    
    bool hasUnprocessedTelemetry() {
      return ( telemetryQueue.size() != 0 );
    }


    /**
       @brief:Load and Store helper functions
    */
    bool store()
    {
      bool retValue = false;
      retValue = storeEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem Storing to EEPROM");
        return false;
      }
      return retValue;
    }

    bool load()
    {
      bool retValue = false;
      retValue = loadEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem loading from EEPROM");
        return false;
      }
      return retValue;
    }

    bool clear()
    {
     
      bool retValue = false;
      retValue = clearEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem loading from EEPROM");
        return false;
      }
      return retValue; 
     }

  LinkedLists<PayloadQueueElement*> telemetryQueue;

  private:
    PersistantState eepromRealState;

    bool storeEEPROM()
    {
      if (persistantState == eepromRealState) {
        DEBUG_PRINTLN("nothing to write, state hasn't changed since last read/write");
        return true;
      }

      DEBUG_PRINTLN("Writing EEPROM, in memory structure is dirty");
      PersistantStateStorageFormat persistantStore(persistantState);
      EEPROM.put(0, persistantStore);
      EEPROM.commit();
      eepromRealState = persistantState;
      return true;
    }

    bool loadEEPROM() {
      PersistantStateStorageFormat persistantStore;
      EEPROM.get(0, persistantStore);
      if (strcmp(persistantStore.version, EEPROM_STORAGE_FORMAT_VERSION) != 0) {
        DEBUG_PRINTLN("storage format doens't match, let defaults load, will become proper in next write.");
        return true;
      }
      persistantState = PersistantState(persistantStore);
      eepromRealState = persistantState;
      return true;
    }

    bool clearEEPROM() {
        for (int i=EEPROM_STARTING_ADDRESS; i<=EEPROM_STORE_SIZE; i++){
             EEPROM.write(i,0);
          }
          EEPROM.commit();
          return true;
      }

    bool storeSPIFF()
    {
      return true;
    }
};

extern DeviceState& deviceState;

// just shortening macros
#define RSTATE   deviceState.runTimeState
#define PSTATE   deviceState.persistantState

#endif // DEVICESTATE_H
