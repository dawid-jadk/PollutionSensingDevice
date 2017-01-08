/*
  GasSensor.h - Library for getting parts per million (PPM) from a gas sensor
  Created by Roberto Burgos, August 15, 2014.
  Released into the public domain.
*/

#ifndef GasSensor_h
#define GasSensor_h

#include "Arduino.h"

/************************Hardware Related Macros************************************/
#define RL_VALUE            5      // Define the load resistance on the board, in kilo ohms
#define RO_CLEAN_AIR_FACTOR 9.83    // Sensor resistance in clean air divided by RO,
                                    // which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define CALIBARAION_SAMPLE_TIMES     50     // Define how many samples you are going to take in the calibration phase
#define CALIBRATION_SAMPLE_INTERVAL  500    // Define the time interal(in milisecond) between each samples in the
                                            // cablibration phase.
#define READ_SAMPLE_INTERVAL         50     // Define how many samples you are going to take in normal operation
#define READ_SAMPLE_TIMES            5      // Define the time interal(in milisecond) between each samples in 
                                            // normal operation.
 
/**********************Application Related Macros**********************************/

#define GAS_CO      0    // Carbon Monoxide
#define GAS_CH4     1    // Methane/Natural gase
#define GAS_LPG     2    // Liquefied petroleum gas


class GasSensor {
  public:
    GasSensor(unsigned int);
    int getPPM(unsigned int);
    float calibrate();
  private:
    unsigned int _sensorPin;
    float _Ro = 10;    // Ro is initialized to 10 kilo ohms
    float _CH4Curve[2]     = {4483.093364, -2.618023789};
    float _COCurve[2]      = {579.9367181, -1.971028561};
    float _LPGCurve[2]     = {1015.709651, -2.074506054};
    float calculateSensorResistance(unsigned int );
    float readSensor(unsigned int);
    int getPercentage(float, float[]);
};

#endif