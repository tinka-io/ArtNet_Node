#pragma once

#include <Arduino.h>

// Pin definitions - adjust to your wiring
#define PIN_PUSH_BUTTON    2
#define PIN_KEY_SWITCH     4
#define PIN_POTENTIOMETER  36
#define PIN_ROTARY_SWITCH  35
#define PIN_LED_RED        14
#define PIN_LED_GREEN      15

// Debounce time in milliseconds
#define DEBOUNCE_TIME 250

// Thresholds for rotary switch (5 positions)
// Adjust based on actual measured values
#define ROTARY_THRESHOLD_1  820
#define ROTARY_THRESHOLD_2  1640
#define ROTARY_THRESHOLD_3  2460
#define ROTARY_THRESHOLD_4  3280
#define ROTARY_THRESHOLD_5  4000
class ControlPanel {
private:
    // States
    bool pushButtonState;
    bool keySwitchState;
    int potentiometerValue;
    int rotarySwitchMode;

    bool changedBtnFlag = false;
    bool changedModeFlag = false;

    // Debounce blocking
    unsigned long pushButtonBlockUntil;
    unsigned long keySwitchBlockUntil;
    unsigned long rotarySwitchBlockUntil;

    // Lowpass filter for potentiometer (4 taps)
    static const int FilterLen = 200;
    int potFilterTaps[FilterLen];
    int potFilterIndex;

    int getRotaryPosition(int value) {
        if (value < ROTARY_THRESHOLD_1) return 1;
        if (value < ROTARY_THRESHOLD_2) return 2;
        if (value < ROTARY_THRESHOLD_3) return 3;
        if (value < ROTARY_THRESHOLD_4) return 4;
        if (value < ROTARY_THRESHOLD_5) return 5;
        return 6;
    }

    int lowPassFilter(int rawValue){
        // Update circular buffer
        potFilterTaps[potFilterIndex] = rawValue;
        potFilterIndex = (potFilterIndex + 1) % FilterLen;

        // Calculate average of FilterLen taps
        int newValue = 0;
        for(int i = 0; i < FilterLen; i ++){
            newValue += potFilterTaps[i];
        }
        return newValue / FilterLen;
    }

public:
    ControlPanel() {
        pushButtonState = false;
        keySwitchState = false;
        potentiometerValue = 0;
        rotarySwitchMode = 1;

        pushButtonBlockUntil = 0;
        keySwitchBlockUntil = 0;
        rotarySwitchBlockUntil = 0;

        // Initialize filter taps
        for (int i = 0; i < 4; i++) {
            potFilterTaps[i] = 0;
        }
        potFilterIndex = 0;
    }

    void begin() {
        pinMode(PIN_PUSH_BUTTON, INPUT_PULLUP);
        pinMode(PIN_KEY_SWITCH, INPUT_PULLUP);
        pinMode(PIN_POTENTIOMETER, INPUT);
        pinMode(PIN_ROTARY_SWITCH, INPUT);
        pinMode(PIN_LED_RED, OUTPUT);
        pinMode(PIN_LED_GREEN, OUTPUT);
        
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_GREEN, LOW);
    }

    void update() {
        unsigned long now = millis();
        
        // Push button - debounced
        if (now >= pushButtonBlockUntil) {
            bool newState = !digitalRead(PIN_PUSH_BUTTON);
            if (newState != pushButtonState) {
                // Serial.println("Push Button: " + String(newState));
                pushButtonState = newState;
                pushButtonBlockUntil = now + DEBOUNCE_TIME;
                changedBtnFlag = true;
            }
        }

        // Key switch - debounced
        bool newState = !digitalRead(PIN_KEY_SWITCH);
        static bool tempState = false;
        if (tempState != newState) {
            keySwitchBlockUntil = now + DEBOUNCE_TIME;
            tempState = newState;
        }
        if ((tempState != keySwitchState) && (now >= keySwitchBlockUntil)) {
            Serial.println("Key: " + String(newState));
            keySwitchState = tempState;
            changedBtnFlag = true;
        }
        

        // Potentiometer - with 4-tap lowpass filter
        int rawValue = analogRead(PIN_POTENTIOMETER);
        int newValue = lowPassFilter(rawValue);
        newValue = map(newValue, 0, 4095, 0, 255);
        if(abs(newValue - potentiometerValue) > 3){
            // Serial.println("Potie: " + String(newValue));
            potentiometerValue = newValue;
            changedModeFlag = true;
        }
        
        // Rotary switch - debounced
        if (now >= rotarySwitchBlockUntil) {
            int newMode = getRotaryPosition(analogRead(PIN_ROTARY_SWITCH));
            if (newMode != rotarySwitchMode && newMode != 6) {
                // Serial.println("Mode: " + String(newMode));
                rotarySwitchMode = newMode;
                rotarySwitchBlockUntil = now + DEBOUNCE_TIME;
                changedModeFlag = true;
            }
        }
    }

    // Get functions
    bool getPushButton() { return pushButtonState; }
    bool getKeySwitch() { return keySwitchState; }
    int getPotentiometer() { return potentiometerValue; }
    int getRotarySwitchMode() { return rotarySwitchMode; }
    bool getChangedBtnFlag() { return changedBtnFlag; }
    bool getChangedModeFlag() { return changedModeFlag; }
    void resetChangedFlags() { 
        changedBtnFlag = false;
        changedModeFlag = false;
    }

    // LED control
    void setLED(bool red, bool green) {
        digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
        digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
    }
};