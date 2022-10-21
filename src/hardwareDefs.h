#ifndef HARDWAREDEFS_H
#define HARDWAREDEFS_H


#define AP_MODE_SSID "Futr-"


//SPIFFS Macros
#define SPIFF_LOG_PATH                  "/LOG"
#define SPIFF_FW_PATH                   "/BIN"
#define FORMAT_SPIFFS_IF_FAILED         true


//JSON Macros
#define START   "["
#define END     "]"
#define COMMA   ","


/**
      @brief:
      Create AP name with AP SSID and macAddress
      @param:
      last three bytes macaddress without colon
      @return:
      concated string
*/

String formApSsidName(String deviceId) {
  return String(AP_MODE_SSID + deviceId);
}

String logDirectoryFromTimeStamp(unsigned long time_stamp) {
  char unixTime[30];
  sprintf(unixTime, "%s_%lu.txt", SPIFF_LOG_PATH, time_stamp);
  return String(unixTime);
}


    
//functional Macros
//#define IS_DATA_LOGGING_TO_SPIFF        1
#define DEBUG_SERIAL                    1


//Revesion Macros
#define HW_REV                          1
#define FW_REV                          1

//modem  macros
#define MODEM_RETRY                     2
#define SERIAL2_RXD                     26
#define SERIAL2_TXD                     27
#define SerialAT                        Serial1
#define GPIO_MODEM_RESET                      5
#define MODEM_PWKEY                     4
#define MODEM_POWER_ON                  23                     


//Timing Macros
#define MILLI_SECS_MULTIPLIER                 1000
#define MICRO_SECS_MULITPLIER                 1000000
#define SECS_PORTAL_WAIT                      60 //120
#define SECS_TO_LOG                           60
#define HTTP_CONNEC_TIMEOUT_IN_MS             10000L
#define READ_SENSOR_CALLBACK_TIME_IN_S        20

//EEPROM Macros
#define EEPROM_STORE_SIZE                     512
#define EEPROM_STORAGE_FORMAT_VERSION         "c1"
#define EEPROM_STARTING_ADDRESS               0

//WiFi macros
#define WAN_WIFI_SSID_DEFAULT                 "Sarthak"
#define WAN_WIFI_PASS_DEFAULT                 "wireless18"

//APN Macros
#define WAN_GPRS_APN_DEFAULT                  "airtelgprs.com"
#define WAN_GPRS_USER_DEFAULT                 "admin"
#define WAN_GPRS_PASS_DEFAULT                 "admin"


//Device macros
#define DEVICE_ID_DEFAULT                     "000000000000"

//Device data send freq default
#define DATA_SEND_FREQ_DEFAULT                1

//Invalid reading Macros
#define INVALID_VOL_READING                   -1
#define INVALID_AMP_READING                   -1
#define INVALID_WATT_READING                  -1
#define INVALID_TEMP_ENV_READING              -1
#define INVALID_TEMP_SOLAR_READING            -1



//PIN Config Macros
#define TEMP_ENV_GPIO                           4
#define TEMP_SOLAR_GPIO                         33
#define DEFAULT_I2C_SDA_GPIO                    21
#define DEFAULT_I2C_SCL_GPIO                    22

//GPIO Indexes
#define TEMP_ENV_INDEX                          0
#define TEMP_SOLAR_INDEX                        1


//Count config macros
#define MAX_WAKEUP_COUNT                        2
#define MIN_WAKEUP_COUNT                        0
#define SPIFF_DIR_DATA_COUNT                    30

//LTC address
#define LTC_I2C_ADDRESS                         0x67


#ifdef DEBUG_SERIAL
#define DEBUG_PRINTF(...)                       Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...)                      Serial.println(__VA_ARGS__)
#define DEBUG_PRINT(...)                        Serial.print(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINT(...)
#endif


#endif // HARDWAREDEFS_H
