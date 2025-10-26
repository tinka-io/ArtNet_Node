#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "NetworkConfig.h"
#include "HtmlPages.h"

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
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:Arial;text-align:center;margin:50px;} h1{color:#333;} .info{background:#e8f4f8;padding:20px;border-radius:10px;margin:20px 0;} ";
    html += ".version{color:#666;font-size:0.9em;margin-top:10px;} ";
    html += "a{display:inline-block;background:#0066cc;color:white;padding:15px 30px;margin:10px;text-decoration:none;border-radius:5px;} a:hover{background:#0052a3;}</style></head>";
    html += "<body><h1>Tinkas ArtNet Node</h1>";
    html += "<div class='version'>Firmware: " + String(FIRMWARE_VERSION) + "</div>";
    html += "<div class='info'><h2>Network Information</h2>";
    html += "<p><strong>Connection:</strong> " + String(usingWiFi ? "WiFi" : "Ethernet") + "</p>";
    html += "<p><strong>Mode:</strong> " + String(useDHCP ? "DHCP" : "Static IP") + "</p>";
    html += "<p><strong>IP Address:</strong> " + getCurrentIP().toString() + "</p>";
    html += "<p><strong>Gateway:</strong> " + getCurrentGateway().toString() + "</p>";
    html += "<p><strong>Subnet Mask:</strong> " + getCurrentSubnet().toString() + "</p></div>";
    html += "<a href='/config'>Network Configuration</a>";
    html += "<a href='/update'>OTA Update</a>";
    html += "</body></html>";
    server.send(200, "text/html", html);
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
  Serial.println("HTTP server started");
}

// Handle web server requests (call in loop)
void handleWebServer() {
  server.handleClient();
}
