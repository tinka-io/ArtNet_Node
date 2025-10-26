#pragma once
// ArtNet protocol handler - network setup is handled by NetworkConfig.h

#include <WiFiUdp.h>
#include "web/WebLogger.h"

const uint16_t ARTNET_PORT = 6454;

// Aligned buffer for better performance
__attribute__((aligned(4))) static uint8_t packetBuffer[530];

// Initialize ArtNet UDP listener
void setup_artnet(WiFiUDP& udp)
{
  udp.begin(ARTNET_PORT);
  LOG_PRINTF("UDP for ArtNet started on port %d\n", ARTNET_PORT);
}

// Receive and parse ArtNet DMX packet
bool get_artnet(WiFiUDP& udp, uint8_t recv_universe, uint8_t *dmxData, uint16_t dataLen)
{
  int packetSize = udp.parsePacket();
  if (packetSize <= 0 || packetSize > sizeof(packetBuffer))
    return false;

  // Complete packet read
  int bytesRead = udp.read(packetBuffer, packetSize);
  if (bytesRead != packetSize)
    return false;

  // Art-Net header check
  if (memcmp(packetBuffer, "Art-Net", 7) != 0)
    return false;

  // OpCode check (Little Endian!)
  uint16_t opCode = packetBuffer[8] | (packetBuffer[9] << 8);
  if (opCode != 0x5000) // OpDmx
    return false;

  // Universe check (Little Endian!)
  uint16_t incoming_universe = packetBuffer[14] | (packetBuffer[15] << 8);
  if (incoming_universe != recv_universe)
    return false;

  // Data length check (Big Endian!)
  uint16_t dmxDataLen = (packetBuffer[16] << 8) | packetBuffer[17];
  if (dmxDataLen == 0 || dmxDataLen > dataLen)
    return false;

  // Safer copy: byte-by-byte instead of memcpy
  for (uint16_t i = 0; i < dmxDataLen; i++)
  {
    dmxData[i] = packetBuffer[18 + i];
  }

  // Set remaining bytes to 0 if less data received
  for (uint16_t i = dmxDataLen; i < dataLen; i++)
  {
    dmxData[i] = 0;
  }

  return true;
}