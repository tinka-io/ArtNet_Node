#pragma once

#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>
#include <Preferences.h>

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

  Serial.println("Starting WiFi fallback...");
  WiFi.mode(WIFI_STA);

  if (!useDHCP) {
    Serial.println("Configuring Static IP for WiFi...");
    WiFi.config(staticIP, gateway, subnet, dns1, dns2);
  }

  WiFi.begin(wifi_ssid, wifi_password);
  usingWiFi = true;
}

void stopWiFiFallback() {
  if (!usingWiFi) return; // Not using WiFi

  Serial.println("Stopping WiFi fallback...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  usingWiFi = false;
}

// WiFi/Ethernet event handler
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("WT32-ETH01");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH IP: ");
      Serial.print(ETH.localIP());
      Serial.print(", Gateway: ");
      Serial.print(ETH.gatewayIP());
      Serial.print(", Subnet: ");
      Serial.println(ETH.subnetMask());
      // Ethernet is connected, stop WiFi if it's running
      stopWiFiFallback();
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected - Switching to WiFi");
      // Ethernet disconnected, start WiFi fallback
      startWiFiFallback();
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("WiFi Connected (Fallback)");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("WiFi IP: ");
      Serial.print(WiFi.localIP());
      Serial.print(", Gateway: ");
      Serial.print(WiFi.gatewayIP());
      Serial.print(", Subnet: ");
      Serial.println(WiFi.subnetMask());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi Disconnected");
      // Try to reconnect WiFi if Ethernet is still down
      if (usingWiFi && !ETH.linkUp()) {
        Serial.println("Attempting to reconnect WiFi...");
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
    Serial.println("Configuring Static IP...");
    if (!ETH.config(staticIP, gateway, subnet, dns1, dns2)) {
      Serial.println("Static IP configuration failed!");
    }
  } else {
    Serial.println("Using DHCP...");
  }

  // Wait for Ethernet connection
  Serial.println("Waiting for Ethernet Connection");
  int timeout = 0;
  while (!ETH.linkUp() && timeout < 5) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (ETH.linkUp()) {
    Serial.println("");
    Serial.println("Ethernet connected!");
    Serial.print("IP address: ");
    Serial.println(ETH.localIP());
    usingWiFi = false;
  } else {
    Serial.println("\nEthernet failed! Falling back to WiFi...");

    // Fallback to WiFi
    WiFi.mode(WIFI_STA);

    if (!useDHCP) {
      Serial.println("Configuring Static IP for WiFi...");
      if (!WiFi.config(staticIP, gateway, subnet, dns1, dns2)) {
        Serial.println("WiFi Static IP configuration failed!");
      }
    }

    WiFi.begin(wifi_ssid, wifi_password);

    Serial.println("Connecting to WiFi...");
    timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) {
      delay(500);
      Serial.print(".");
      timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected!");
      Serial.print("SSID: ");
      Serial.println(wifi_ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      usingWiFi = true;
    } else {
      Serial.println("\nFailed to connect to WiFi! No network available.");
    }
  }
}
