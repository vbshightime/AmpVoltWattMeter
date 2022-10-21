#ifndef SENSORPAYLOAD_H
#define SENSORPAYLOAD_H

#include "hardwareDefs.h"
#include "assert.h"



struct SensorPayload {
  public:
    SensorPayload():  
    temp_env(INVALID_TEMP_ENV_READING),
    temp_solar(INVALID_TEMP_SOLAR_READING),
    power(INVALID_WATT_READING),
    amp(INVALID_AMP_READING),
    vol(INVALID_VOL_READING),
    hwRev(HW_REV),
    fwRev(FW_REV){}
    float temp_env;
    float temp_solar;
    float power;
    float amp;
    float vol;
    uint8_t hwRev;                  // identifies hw revision
    uint8_t fwRev;
    unsigned long epoch;
} __attribute__ ((packed));



SensorPayload* copyPayloadObject(const SensorPayload *payload) {
    if (!payload) {
        return nullptr;
    }
    SensorPayload *newPayload = new SensorPayload;
    *newPayload = *payload;
    return newPayload;
}


void freePayloadObject(SensorPayload *payload) {
    if (!payload) {
        return;
    }
    delete(payload);
}

size_t sizeofPaylodObject(const SensorPayload *payload)
{
    return sizeof (SensorPayload);
}


#endif // SENSORPAYLOAD_H
