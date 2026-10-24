#ifndef NETWORK_H
#define NETWORK_H

#include "Config.h"

extern WebServer server;

void setupWiFi();
void uploadLiveEMG();
void sendDataLog(String exercise, int durationSec, int emgPeak, int repetitions, String status);

#endif