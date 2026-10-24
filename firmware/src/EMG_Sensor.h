#ifndef EMG_SENSOR_H
#define EMG_SENSOR_H

#include "Config.h"

void calibrateEmgBaseline();
int calculateEmgPercent(int adc);
int readSingleEmgSensor();

#endif