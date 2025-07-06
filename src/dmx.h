#pragma once

#include "Arduino.h"
#include <esp_dmx.h>

dmx_port_t dmxPort = 1;
const uint8_t transmitPin = 17;
const uint8_t receivePin = 5;
const uint8_t enablePin = 33;

// byte dmx_data[DMX_PACKET_SIZE];

void setup_dmx()
{
    dmx_config_t config = DMX_CONFIG_DEFAULT;

    // dmx_personality_t personalities[] = {};
    // int personality_count = 0;
    // dmx_driver_install(dmxPort, &config, personalities, personality_count);


    dmx_driver_install(dmxPort, &config, NULL, 0);
    if(!dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin))
    {
        Serial.printf("ERORR: DMX Setup\n");
        return;
    }

    Serial.printf("DMX Setup complete\n");
}

void send_dmx(uint8_t *data, uint16_t dataLen)
{
    dmx_write(dmxPort, data, dataLen);
    dmx_send_num(dmxPort, dataLen);
    // dmx_wait_sent(dmxPort, portMAX_DELAY); 
}