#pragma once

#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>
#include <Preferences.h>
#include "WebLogger.h"

// Firmware version - automatically generated from compile time
#define FIRMWARE_VERSION __DATE__ " " __TIME__

// WiFi fallback credentials
const char* wifi_ssid = "KAOS";
const char* wifi_password = "KAOSboot77!";

// Network Configuration
Preferences preferences;
bool useDHCP = true;
bool usingWiFi = false;  // Track if we're using WiFi fallback
IPAddress staticIP(2, 0, 0, 99);
IPAddress gateway(2, 0, 0, 1);
IPAddress subnet(255, 0, 0, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);

// Operation mode: false = ArtNet Node, true = Control Panel
bool useControlPanelMode = false;

// Function to load network configuration from preferences
void loadNetworkConfig() {
  preferences.begin("network", false);
  useDHCP = preferences.getBool("useDHCP", true);
  useControlPanelMode = preferences.getBool("ctrlPanel", false);

  if (!useDHCP) {
    uint32_t ip = preferences.getUInt("staticIP", 0);
    uint32_t gw = preferences.getUInt("gateway", 0);
    uint32_t sn = preferences.getUInt("subnet", 0);
    uint32_t d1 = preferences.getUInt("dns1", 0);
    uint32_t d2 = preferences.getUInt("dns2", 0);

    if (ip != 0) staticIP = IPAddress(ip);
    if (gw != 0) gateway = IPAddress(gw);
    if (sn != 0) subnet = IPAddress(sn);
    if (d1 != 0) dns1 = IPAddress(d1);
    if (d2 != 0) dns2 = IPAddress(d2);
  }
  preferences.end();
}

// Function to save network configuration to preferences
void saveNetworkConfig() {
  preferences.begin("network", false);
  preferences.putBool("useDHCP", useDHCP);
  preferences.putBool("ctrlPanel", useControlPanelMode);
  preferences.putUInt("staticIP", static_cast<uint32_t>(staticIP));
  preferences.putUInt("gateway", static_cast<uint32_t>(gateway));
  preferences.putUInt("subnet", static_cast<uint32_t>(subnet));
  preferences.putUInt("dns1", static_cast<uint32_t>(dns1));
  preferences.putUInt("dns2", static_cast<uint32_t>(dns2));
  preferences.end();
}

// Parse IP address from string
bool parseIP(const String& str, IPAddress& ip) {
  return ip.fromString(str);
}

// Get current IP address (WiFi or Ethernet)
IPAddress getCurrentIP() {
  return usingWiFi ? WiFi.localIP() : ETH.localIP();
}

IPAddress getCurrentGateway() {
  return usingWiFi ? WiFi.gatewayIP() : ETH.gatewayIP();
}

IPAddress getCurrentSubnet() {
  return usingWiFi ? WiFi.subnetMask() : ETH.subnetMask();
}

// WiFi fallback management
void startWiFiFallback() {
  if (usingWiFi) return; // Already using WiFi

  LOG_PRINTLN("Starting WiFi fallback...");
  WiFi.mode(WIFI_STA);

  if (!useDHCP) {
    LOG_PRINTLN("Configuring Static IP for WiFi...");
    WiFi.config(staticIP, gateway, subnet, dns1, dns2);
  }

  WiFi.begin(wifi_ssid, wifi_password);
  usingWiFi = true;
}

void stopWiFiFallback() {
  if (!usingWiFi) return; // Not using WiFi

  LOG_PRINTLN("Stopping WiFi fallback...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  usingWiFi = false;
}

// WiFi/Ethernet event handler
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      LOG_PRINTLN("ETH Started");
      ETH.setHostname("WT32-ETH01");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      LOG_PRINTLN("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      LOG_PRINTF("ETH IP: %s, Gateway: %s, Subnet: %s\n",
                 ETH.localIP().toString().c_str(),
                 ETH.gatewayIP().toString().c_str(),
                 ETH.subnetMask().toString().c_str());
      // Ethernet is connected, stop WiFi if it's running
      stopWiFiFallback();
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      LOG_PRINTLN("ETH Disconnected - Switching to WiFi");
      // Ethernet disconnected, start WiFi fallback
      startWiFiFallback();
      break;
    case ARDUINO_EVENT_ETH_STOP:
      LOG_PRINTLN("ETH Stopped");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      LOG_PRINTLN("WiFi Connected (Fallback)");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      LOG_PRINTF("WiFi IP: %s, Gateway: %s, Subnet: %s\n",
                 WiFi.localIP().toString().c_str(),
                 WiFi.gatewayIP().toString().c_str(),
                 WiFi.subnetMask().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      LOG_PRINTLN("WiFi Disconnected");
      // Try to reconnect WiFi if Ethernet is still down
      if (usingWiFi && !ETH.linkUp()) {
        LOG_PRINTLN("Attempting to reconnect WiFi...");
        WiFi.reconnect();
      }
      break;
    default:
      break;
  }
}

// Initialize network (Ethernet with WiFi fallback)
void initializeNetwork() {
  // Load network configuration
  loadNetworkConfig();

  // Initialize Ethernet
  WiFi.onEvent(WiFiEvent);

  // Must call ETH.begin() BEFORE ETH.config()
  // WT32-ETH01 uses LAN8720 PHY
  // ETH_PHY_ADDR = 1, ETH_PHY_POWER = 16, ETH_PHY_MDC = 23, ETH_PHY_MDIO = 18, ETH_PHY_TYPE = ETH_PHY_LAN8720, ETH_CLK_MODE = ETH_CLOCK_GPIO0_IN
  ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);

  if (!useDHCP) {
    LOG_PRINTLN("Configuring Static IP...");
    if (!ETH.config(staticIP, gateway, subnet, dns1, dns2)) {
      LOG_PRINTLN("Static IP configuration failed!");
    }
  } else {
    LOG_PRINTLN("Using DHCP...");
  }

  // Wait for Ethernet connection
  LOG_PRINTLN("Waiting for Ethernet Connection");
  int timeout = 0;
  while (!ETH.linkUp() && timeout < 5) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (ETH.linkUp()) {
    LOG_PRINTLN("\nEthernet connected!");
    LOG_PRINTF("IP address: %s\n", ETH.localIP().toString().c_str());
    usingWiFi = false;
  } else {
    LOG_PRINTLN("\nEthernet failed! Falling back to WiFi...");

    // Fallback to WiFi
    WiFi.mode(WIFI_STA);

    if (!useDHCP) {
      LOG_PRINTLN("Configuring Static IP for WiFi...");
      if (!WiFi.config(staticIP, gateway, subnet, dns1, dns2)) {
        LOG_PRINTLN("WiFi Static IP configuration failed!");
      }
    }

    WiFi.begin(wifi_ssid, wifi_password);

    LOG_PRINTLN("Connecting to WiFi...");
    timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) {
      delay(500);
      Serial.print(".");
      timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      LOG_PRINTLN("\nWiFi connected!");
      LOG_PRINTF("SSID: %s\n", wifi_ssid);
      LOG_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
      usingWiFi = true;
    } else {
      LOG_PRINTLN("\nFailed to connect to WiFi! No network available.");
    }
  }
}
