#pragma once

#include "Arduino.h"
#include <esp_dmx.h>

// DMX configuration
const dmx_port_t DMX_PORT = 1;
const uint8_t DMX_TRANSMIT_PIN = 17;
const uint8_t DMX_RECEIVE_PIN = 5;
const uint8_t DMX_ENABLE_PIN = 33;

// DMX timing constants
const uint32_t DMX_TIMEOUT_MS = 1000;

// Global DMX port handle
dmx_port_t dmxPort = DMX_PORT;

bool setup_dmx()
{
    // Initialize DMX configuration with default values
    dmx_config_t config = DMX_CONFIG_DEFAULT;
    
    // Install DMX driver without personalities (transmit only)
    
    if (!dmx_driver_install(dmxPort, &config, NULL, 0))
    {
        Serial.printf("ERROR: DMX driver install failed!\n");
        return false;
    }

    // Configure DMX pins
    if (!dmx_set_pin(dmxPort, DMX_TRANSMIT_PIN, DMX_RECEIVE_PIN, DMX_ENABLE_PIN))
    {
        Serial.printf("ERROR: DMX pin configuration failed\n");
        return false;
    }

    Serial.printf("DMX setup complete - Port: %d, TX: %d, RX: %d, EN: %d\n", 
                  dmxPort, DMX_TRANSMIT_PIN, DMX_RECEIVE_PIN, DMX_ENABLE_PIN);

    return true;
}

void send_dmx(uint8_t *data, uint16_t dataLen)
{
    // Validate input parameters
    if (data == NULL || dataLen == 0 || dataLen > DMX_PACKET_SIZE)
    {
        Serial.printf("ERROR: Invalid DMX data parameters\n");
        return;
    }

    // Write data to DMX buffer
    size_t bytes_written = dmx_write(dmxPort, data, dataLen);
    if (bytes_written != dataLen)
    {
        Serial.printf("WARNING: DMX write incomplete - wrote %d/%d bytes\n", 
                      bytes_written, dataLen);
    }

    // Send DMX packet
    size_t bytes_sended = dmx_send_num(dmxPort, dataLen);
    if (bytes_sended != dataLen)
    {
        Serial.printf("ERROR: DMX send failed!\n");
        return;
    }

    // Optional: Wait for transmission to complete
    // Uncomment if you need to ensure transmission is finished before continuing
    // dmx_wait_sent(dmxPort, DMX_TIMEOUT_MS / portTICK_PERIOD_MS);
}