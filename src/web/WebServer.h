#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "NetworkConfig.h"
#include "HtmlPages.h"
#include "WebLogger.h"
#include "../settings/AppSettings.h"

WebServer server(80);

// Replace placeholders in HTML
String processor(const String& var) {
  if (var == "MODE") return useDHCP ? "DHCP" : "Static IP";
  if (var == "IP") return getCurrentIP().toString();
  if (var == "GATEWAY") return getCurrentGateway().toString();
  if (var == "SUBNET") return getCurrentSubnet().toString();
  if (var == "DHCP_CHECKED") return useDHCP ? "checked" : "";
  if (var == "STATIC_CHECKED") return useDHCP ? "" : "checked";
  if (var == "STATIC_IP") return staticIP.toString();
  if (var == "GATEWAY_IP") return gateway.toString();
  if (var == "SUBNET_MASK") return subnet.toString();
  if (var == "DNS1") return dns1.toString();
  if (var == "DNS2") return dns2.toString();
  return String();
}

// Setup web server routes
void setupWebServer() {
  // Home page
  server.on("/", []() {
    String html = String(homePage);
    html.replace("%FIRMWARE_VERSION%", FIRMWARE_VERSION);
    html.replace("%MODE%", AppSettings::getModeString());
    html.replace("%MODE_CHECKED%", AppSettings::app.useControlPanelMode ? "checked" : "");
    html.replace("%CONNECTION%", String(usingWiFi ? "WiFi" : "Ethernet"));
    html.replace("%IP_MODE%", String(useDHCP ? "DHCP" : "Static IP"));
    html.replace("%IP%", getCurrentIP().toString());
    html.replace("%GATEWAY%", getCurrentGateway().toString());
    html.replace("%SUBNET%", getCurrentSubnet().toString());
    server.send(200, "text/html", html);
  });

  // API: Set operation mode
  server.on("/api/setMode", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, body);

      if (error) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        return;
      }

      bool newMode = doc["useControlPanel"] | false;
      AppSettings::app.useControlPanelMode = newMode;
      AppSettings::save();

      String modeName = newMode ? "Control Panel" : "ArtNet Node";
      LOG_PRINTF("Mode changed to: %s\n", modeName.c_str());

      server.send(200, "application/json", "{\"success\":true,\"message\":\"Mode changed to " + modeName + "\"}");
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"No data received\"}");
    }
  });

  // Configuration page
  server.on("/config", []() {
    String html = String(configPage);
    html.replace("%MODE%", String(usingWiFi ? "WiFi (" : "Ethernet (") + (useDHCP ? "DHCP)" : "Static IP)"));
    html.replace("%IP%", getCurrentIP().toString());
    html.replace("%GATEWAY%", getCurrentGateway().toString());
    html.replace("%SUBNET%", getCurrentSubnet().toString());
    html.replace("%DHCP_CHECKED%", useDHCP ? "checked" : "");
    html.replace("%STATIC_CHECKED%", useDHCP ? "" : "checked");
    html.replace("%STATIC_IP%", staticIP.toString());
    html.replace("%GATEWAY_IP%", gateway.toString());
    html.replace("%SUBNET_MASK%", subnet.toString());
    html.replace("%DNS1%", dns1.toString());
    html.replace("%DNS2%", dns2.toString());
    server.send(200, "text/html", html);
  });

  // Logs page
  server.on("/logs", []() {
    String html = String(logsPage);
    server.send(200, "text/html", html);
  });

  // API: Get logs
  server.on("/api/logs", []() {
    handleGetLogs(&server);
  });

  // API: Clear logs
  server.on("/api/logs/clear", HTTP_POST, []() {
    handleClearLogs(&server);
  });

  // Save configuration endpoint
  server.on("/saveConfig", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, body);

      if (error) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        return;
      }

      String ipMode = doc["ipMode"] | "dhcp";
      useDHCP = (ipMode == "dhcp");

      if (!useDHCP) {
        String ipStr = doc["staticIP"] | "";
        String gwStr = doc["gateway"] | "";
        String snStr = doc["subnet"] | "";
        String d1Str = doc["dns1"] | "";
        String d2Str = doc["dns2"] | "";

        if (!parseIP(ipStr, staticIP) || !parseIP(gwStr, gateway) || !parseIP(snStr, subnet)) {
          server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid IP address format\"}");
          return;
        }

        parseIP(d1Str, dns1);
        parseIP(d2Str, dns2);
      }

      saveNetworkConfig();
      server.send(200, "application/json", "{\"success\":true,\"message\":\"Configuration saved! Restarting...\"}");

      delay(1000);
      ESP.restart();
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"No data received\"}");
    }
  });
}

// Start web server
void startWebServer() {
  server.begin();
  LOG_PRINTLN("HTTP server started");
}

// Handle web server requests (call in loop)
void handleWebServer() {
  server.handleClient();
}
