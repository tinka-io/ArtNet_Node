#include "Arduino.h"
#include "artnet.h"
#include "dmx.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Configuration
const u8 UNIVERSE = 8;
const u16 DMX_DATA_LEN = 512;
const u32 DMX_RATE_MS = 33; // ~30 Hz
// 40 Hz -> 25ms
// 30 Hz -> 33.33ms
// 25 Hz -> 40ms
// 20 Hz -> 50ms

// Watchdog

void setup()
{
  Serial.begin(115200);
  Serial.printf("Artnet Node IP: 2.0.0.32\n");
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());

  setup_artnet();
  if (!setup_dmx())
  {
    while (true)
    {
      sleep(1000);
    }
  };
  
  Serial.println("Setup complete");
}

void mapLuminixMiniBeam(u8 *data, int index)
{
  // Bounds checking
  if (index >= DMX_DATA_LEN - 1){
    return;
  }
  
  u16 value = (data[index] << 8) | data[index + 1];
  value = min(value + 128, 65535);
  
  data[index] = value >> 8;
  data[index + 1] = value & 0xFF;
}

void data_corretion(u8 *data)
{
  // Bounds checking for the loop
  for (int i = 399; i < 496 && i < DMX_DATA_LEN - 3; i += 12)
  {
    mapLuminixMiniBeam(data, i);
    mapLuminixMiniBeam(data, i + 2);
  }
}

void loop()
{
  static unsigned long lastArtNetTime = 0;
  static unsigned long lastDmxTime = 0;
  
  // Aligned DMX buffer for better performance
  __attribute__((aligned(4))) static u8 dmxData[DMX_DATA_LEN] = {0};
  
  unsigned long now = millis();
  
  // Receive Art-Net data
  bool new_data = get_artnet(UNIVERSE, dmxData, DMX_DATA_LEN);
  if (new_data)
  {
    lastArtNetTime = now;
    data_corretion(dmxData);
    
    // Debug output
    Serial.printf("Art-Net: Ch[399]=%d, Ch[400]=%d\n", dmxData[399], dmxData[400]);
  }
  
  // Send DMX continuously
  if (now - lastDmxTime >= DMX_RATE_MS)
    {
      lastDmxTime = now;
      send_dmx(dmxData, DMX_DATA_LEN);
    
    // Warning for old Art-Net data
    if (now - lastArtNetTime > 5000)
    {
      lastArtNetTime = now;
      Serial.printf("Warning: No Art-Net data for %lu ms\n", now - lastArtNetTime);
    }
  }
  
  // Watchdog reset
  yield();
}