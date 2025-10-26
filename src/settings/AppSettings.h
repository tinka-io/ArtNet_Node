#pragma once

#include <Arduino.h>
#include <Preferences.h>

// Application Settings Structure
struct ApplicationSettings {
  bool useControlPanelMode;   // false = ArtNet Node, true = Control Panel
  uint8_t artnetUniverse;      // ArtNet universe to listen to
  uint32_t dmxRefreshRate;     // DMX refresh rate in milliseconds
};

// Central Settings Manager
class AppSettings {
public:
  // Application settings
  static ApplicationSettings app;

  // Load all settings from persistent storage
  static void load() {
    Preferences prefs;
    prefs.begin("appsettings", true); // Read-only

    // ArtNet Configuration
    app.useControlPanelMode = prefs.getBool("ctrlPanel", false);
    app.artnetUniverse = prefs.getUChar("artnetUniv", 8);
    
    // DMX Configuration
    app.dmxRefreshRate = prefs.getUInt("dmxRefresh", 33);
    // DMX refresh rate examples:
    // 40 Hz -> 25ms
    // 30 Hz -> 33.33ms
    // 25 Hz -> 40ms
    // 20 Hz -> 50ms

    prefs.end();

    Serial.println("=== Application Settings Loaded ===");
    Serial.printf("Mode: %s\n", app.useControlPanelMode ? "Control Panel" : "ArtNet Node");
    Serial.printf("ArtNet Universe: %d\n", app.artnetUniverse);
    Serial.printf("DMX Refresh Rate: %d ms\n", app.dmxRefreshRate);
  }

  // Save all settings to persistent storage
  static void save() {
    Preferences prefs;
    prefs.begin("appsettings", false); // Read-write

    prefs.putBool("ctrlPanel", app.useControlPanelMode);
    prefs.putUChar("artnetUniv", app.artnetUniverse);
    prefs.putUInt("dmxRefresh", app.dmxRefreshRate);

    prefs.end();

    Serial.println("=== Application Settings Saved ===");
  }

  // Reset to factory defaults
  static void reset() {
    app.useControlPanelMode = false;
    app.artnetUniverse = 8;
    app.dmxRefreshRate = 33;
    save();
    Serial.println("=== Application Settings Reset to Defaults ===");
  }

  // Get mode as string for display
  static const char* getModeString() {
    return app.useControlPanelMode ? "Control Panel" : "ArtNet Node";
  }
};

// Static member initialization
ApplicationSettings AppSettings::app = {
  false,  // useControlPanelMode
  8,      // artnetUniverse
  33      // dmxRefreshRate
};
