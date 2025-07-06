#include "Arduino.h"
#include "artnet.h"
#include "dmx.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

void setup()
{
  Serial.begin(115200);
  Serial.printf("Artnet Node IP: 2.0.0.32\n");
  Serial.println(getCpuFrequencyMhz());

  setup_artnet();

  setup_dmx();
}

void mapLuminixMiniBeam(u8 *dmxData, int i)
{
  u8 high, low;
  u32 value;

  high = dmxData[i];
  low = dmxData[i + 1];
  value = (high << 8) | low;

  value = value + 128;
  if (value > 65535) {
    value = 65535;
  }
  
  high = value / 256;
  low = value % 256;

  dmxData[i] = high;
  dmxData[i + 1] = low;
}

void data_corretion(u8 *dmxData)
{
  u8 high, low;
  u16 value;

  for (int i = 399; i < 496; i += 12)
  {
    mapLuminixMiniBeam(dmxData, i);
    mapLuminixMiniBeam(dmxData, i+2);
  }
}

void loop()
{
  const u8 universe = 8;
  const u16 dataLen = 512;
  static u8 dmxData[dataLen] = {};

  bool new_data = get_artnet(universe, dmxData, dataLen);
  if (new_data)
  {
    // Serial.printf("%3d, %3d", dmxData[399], dmxData[400]);
    data_corretion(dmxData);
    // Serial.printf(" - %3d, %3d\n", dmxData[399], dmxData[400]);
  }

  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  // 40 Hz -> 25ms
  // 30 Hz -> 33.33ms
  // 25 Hz -> 40ms
  // 20 Hz -> 50ms
  if (now - lastUpdate >= 33)
  {
    // Question to AI:
    // If I leave this for loop active, the DMX Output Works fine.
    // If I don't overwrte the dmxData array, the DMX Output stopts working. 
    // What can it be?
    for(int i=0; i < dataLen; i++){
      dmxData[i] = 127;
    }
    lastUpdate = now;
    send_dmx(dmxData, dataLen);
    // Serial.printf("%d\n", dmxData[2]);
  }
}
