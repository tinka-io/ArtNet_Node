#pragma once

#include <Arduino.h>
#include <ElegantOTA.h>
#include "web/WebLogger.h"

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  LOG_PRINTLN("OTA update started!");
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    LOG_PRINTF("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  if (success) {
    LOG_PRINTLN("OTA update finished successfully!");
  } else {
    LOG_PRINTLN("There was an error during OTA update!");
  }
}

void OTA_setup(WebServer* server) {
  ElegantOTA.begin(server);

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
}

void OTA_loop(){
    ElegantOTA.loop();
}