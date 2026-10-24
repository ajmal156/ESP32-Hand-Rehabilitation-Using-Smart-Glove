#ifndef HARDWARE_H
#define HARDWARE_H

#include "Config.h"

void setupHardware();
void setupLCD();
void setupKeypad();
void updateLcd(String line1, String line2);
void setValves(bool inflate, bool deflate);
void readKeypad();
bool safeDelay(unsigned long ms);
void stopSystem();
void runCustomExercise(String name, int inflateS, int deflateS, int repetitions);

#endif