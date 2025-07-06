#pragma once
// https://github.com/egnor/wt32-eth01

#include <ETH.h>
#include <WiFiUdp.h>

#define ETH_PHY_ADDR 1
#define ETH_PHY_POWER 16 // Important: GPIO 16 (active HIGH)
#define ETH_PHY_MDC 23
#define ETH_PHY_MDIO 18
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN // WT32-ETH01 has crystal

const IPAddress localIP(2, 0, 0, 32);
const IPAddress gateway(2, 0, 0, 1);
const IPAddress subnet(255, 0, 0, 0);

WiFiUDP udp;
const uint16_t ARTNET_PORT = 6454;

// Aligned buffer for better performance
__attribute__((aligned(4))) static uint8_t packetBuffer[530];

void WiFiEvent(WiFiEvent_t event)
{
  if (event == SYSTEM_EVENT_ETH_GOT_IP)
  {
    Serial.print("Ethernet IP: ");
    Serial.println(ETH.localIP());
  }
}

void setup_artnet()
{
  pinMode(ETH_PHY_POWER, OUTPUT); // Enable power
  digitalWrite(ETH_PHY_POWER, HIGH);

  WiFi.onEvent(WiFiEvent);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ETH.begin(ETH_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
#else
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_TYPE, ETH_CLK_MODE);
#endif

  // Wait for link & IP
  Serial.println("Waiting for Ethernet connection...");
  while (!ETH.linkUp() || ETH.localIP()[0] == 0)
  {
    delay(500);
    Serial.print(".");
  }

  ETH.config(localIP, gateway, subnet);
  udp.begin(ARTNET_PORT);
  Serial.println("\nUDP for ArtNet started.");
}

bool get_artnet(uint8_t recv_universe, uint8_t *dmxData, uint16_t dataLen)
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