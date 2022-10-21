#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT31.h"
#include "deviceState.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "utils.h"
#include "LTC2946.h"

//We will follow Multibus protocol as there are two temperature sensors
OneWire ds18x20[] = { TEMP_ENV_GPIO, TEMP_SOLAR_GPIO };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];


LTC2946    ltc(&Wire);
const float resistor = .02;         //!< resistor value on demo board

const float LTC2946_ADIN_lsb = 5.001221E-04;                      //!< Typical ADIN lsb weight in volts
const float LTC2946_DELTA_SENSE_lsb = 2.5006105E-05;              //25uV/LSB/Rsense=25/0.02=1.25mA/LSB!< Typical Delta lsb weight in volts
const float LTC2946_VIN_lsb = 2.5006105E-02;                      //25mV/LSB!< Typical VIN lsb weight in volts
const float LTC2946_Power_lsb = 6.25305E-07;                      //1.25mA/LSB*25mV/LSB=31.25uW/LSB*Rsense(because we are deviding weight by Rsense in power calculation)!< Typical POWER lsb weight in V^2 VIN_lsb * DELTA_SENSE_lsb
const float LTC2946_ADIN_DELTA_SENSE_lsb = 1.25061E-08;           //!< Typical sense lsb weight in V^2  *ADIN_lsb * DELTA_SENSE_lsb
const float LTC2946_INTERNAL_TIME_lsb = 4101.00 / 250000.00;      //!< Internal TimeBase lsb. Use LTC2946_TIME_lsb if an external CLK is used. See Settings menu for how to calculate Time LSB.

static uint8_t CTRLA = LTC2946_CHANNEL_CONFIG_V_C_3 | LTC2946_SENSE_PLUS | LTC2946_OFFSET_CAL_EVERY | LTC2946_ADIN_GND; // Set Control A register to default value.
/**
 * @ToDo:we will use this later 
 */
//static uint8_t VOLTAGE_SEL = LTC2946_SENSE_PLUS;                                                //! Set Voltage selection to default value.


bool LTCInit() {
  uint8_t ack = 0;
  uint8_t LTC2946_mode;
  ltc.init(LTC_I2C_ADDRESS);
  LTC2946_mode = CTRLA;                                                         //! Set the configuration of the CTRLA Register.
  Serial.println();
  ack |= ltc.LTC2946_write(LTC2946_CTRLA_REG, LTC2946_mode);   //! Sets the LTC2946 to continuous mode
  return ack;
}

bool LTCPowerCalculation() {
  //if (ack) {
  RSTATE.power = RSTATE.amp*RSTATE.vol;
  //}
  DEBUG_PRINTF("Watt calculation: %.2f\n", RSTATE.power);
  return true;
}

bool LTCAmpCalculation() {
  uint16_t current_code;
  uint8_t ack = 0;
  ack |= ltc.LTC2946_read_12_bits(LTC2946_DELTA_SENSE_MSB_REG, &current_code);
  //if (ack) {
  RSTATE.amp = ltc.LTC2946_code_to_current(current_code, resistor, LTC2946_DELTA_SENSE_lsb);
  DEBUG_PRINTF("Amp calculation: %.2f\n", RSTATE.amp);
  //}
  return ack;
}

bool LTCVolCalculation() {
  uint16_t voltage_code;
  uint8_t ack = 0;
  ack |= ltc.LTC2946_read_12_bits(LTC2946_VIN_MSB_REG, &voltage_code);
  //if (ack) {
  RSTATE.vol = ltc.LTC2946_VIN_code_to_voltage(voltage_code , LTC2946_VIN_lsb);
  //}
  DEBUG_PRINTF("Vol calculation: %.2f\n", RSTATE.vol);
  return ack;
}

void DSB112Init()
{
  sensor[TEMP_ENV_INDEX].setOneWire(&ds18x20[TEMP_ENV_INDEX]);
  sensor[TEMP_ENV_INDEX].begin();
  sensor[TEMP_SOLAR_INDEX].setOneWire(&ds18x20[TEMP_SOLAR_INDEX]);
  sensor[TEMP_SOLAR_INDEX].begin();
}


bool readDSB112()
{
  sensor[TEMP_ENV_INDEX].requestTemperatures();
  float tempEnv = sensor[TEMP_ENV_INDEX].getTempCByIndex(0);
  DEBUG_PRINTF("DSB temprature for environment %.1f", tempEnv);
  if (isnan(tempEnv) || (int)tempEnv < -50) {
    DEBUG_PRINTLN("failed to read DSB Env temperature");
    return false;
  }
  sensor[TEMP_SOLAR_INDEX].requestTemperatures();
  float tempSolar = sensor[TEMP_SOLAR_INDEX].getTempCByIndex(0);
  DEBUG_PRINTF("DSB temprature for solar %.1f", tempSolar);
  if (isnan(tempSolar) || (int)tempSolar < -50) {
    DEBUG_PRINTLN("failed to read DSB Solar temperature");
    return false;
  }

  RSTATE.temp_env = tempEnv;
  RSTATE.temp_solar = tempSolar;
  return true;
}

#endif
