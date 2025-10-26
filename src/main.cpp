#include "Arduino.h"
#include "esp_system.h"
#include <WiFiUdp.h>

#include "web/WebServer.h"
#include "OTA.h"

#include "Artnet.h"
#include "Dmx.h"
#include "ControlPanel.h"
#include "PanelLogic.h"

ControlPanel cP;
PanelLogic pL(&cP);

// UDP for ArtNet
WiFiUDP udp;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

const u8 LED_PIN = 32;// 15; 15 Is for the Panno LED

// Configuration
const u8 UNIVERSE = 8;
const u16 DMX_DATA_LEN = 512;
const u32 DMX_RATE_MS = 33; // ~30 Hz
// 40 Hz -> 25ms
// 30 Hz -> 33.33ms
// 25 Hz -> 40ms
// 20 Hz -> 50ms

// Watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule()
{
  ESP.restart();
}

void setup_watchdog()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &resetModule, true);
  timerAlarmWrite(timer, 20000000, false); // 20 seconds
  timerAlarmEnable(timer);
}

void blink_led(u32 dt){
  static u32 last = 0;

  if(millis() - last > dt){
    last = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void print_status()
{
  // Print status every 30 seconds
  static unsigned long last_status = 0;
  if (millis() - last_status > 30000)
  {
    last_status = millis();
    Serial.println("\n[STATUS] Firmware: " + String(FIRMWARE_VERSION) + " | Uptime: " + String(millis() / 1000) + "s");
  }
}

void setup()
{
  setup_watchdog();
  
  // Disable unused features
  btStop();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);

  delay(100); // Give serial time to initialize
  Serial.println("\n\n========================================");
  Serial.println("Tinkas ArtNet Node");
  Serial.println("Firmware Version: " + String(FIRMWARE_VERSION));
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
  Serial.println("========================================\n");

  // Initialize network (Ethernet with WiFi fallback)
  initializeNetwork();

  // Setup web server and OTA
  setupWebServer();
  OTA_setup(&server);
  startWebServer();

  cP.begin();
  
  setup_artnet(udp);
  if (!setup_dmx())
  {
    while (true)
    {
      sleep(1000);
    }
  };

  Serial.println("Setup complete");
  digitalWrite(LED_PIN, LOW);
}

void data_corretion(u8 *data)
{
  // Bounds checking for the loop
  for (int i = 399; i < 496 && i < 503; i += 13)
  {
    data[i + 1] = (uint8_t)(data[i + 1] / 2.5);
    data[i + 3] = data[i + 3] >> 3;
  }
}

void loop_artnet_node()
{
  static unsigned long lastArtNetTime = 0;
  static unsigned long lastDmxTime = 0;
  static u32 led_dt = 1000;
  // Aligned DMX buffer for better performance
  __attribute__((aligned(4))) static u8 dmxData[DMX_DATA_LEN] = {0};

  unsigned long now = millis();

  // Receive Art-Net data
  bool new_data = get_artnet(udp, UNIVERSE, dmxData, DMX_DATA_LEN);
  if (new_data)
  {
    if(led_dt != 250) led_dt = 250;
    
    lastArtNetTime = now;
    // Serial.printf("Art-Net: %d, %d -> ", dmxData[438], dmxData[439]);
    data_corretion(dmxData);
    // Serial.printf("%d, %d\n", dmxData[438], dmxData[439]);
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
      if(led_dt != 1000) led_dt = 1000;
    }
  }

  blink_led(led_dt);
  // Watchdog reset
  timerWrite(timer, 0);
}

void loop_control_panael()
{
  cP.update();
  pL.update();

  static unsigned long lastDmxTime = 0;
  // Send DMX continuously
  unsigned long now = millis();
  if (now - lastDmxTime >= DMX_RATE_MS)
  {
    lastDmxTime = now;
    send_dmx(pL.getDMXbuffer(), 512);
  }

  blink_led(500);


  // Watchdog reset
  timerWrite(timer, 0);
}

void loop()
{
  handleWebServer();
  OTA_loop();

  //loop_artnet_node();
  loop_control_panael();
}