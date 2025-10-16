#pragma once

#include <Arduino.h>
#include "ControlPanel.h"

// Timer duration for push button (1 hour in milliseconds)
#define HEATER_TIMER_DURATION  (60UL * 60UL * 1000UL)

class PanelLogic {
private:
    ControlPanel* panel;
    
    // System state
    bool heatingActiv;
    bool eventBlockChange;
    bool pushButton;
    int activeMode;
    int activePotiValue;
    
    // Heater timer
    unsigned long heaterTimerEnd;
    bool heaterTimerRunning;

    uint8_t dmxBuffer[512];
    // External DMX function pointer
    void setDMXchannel(int ch, int val){
        dmxBuffer[ch-1] = val;
    }

    void setHeater(bool state) {
        int value = state ? 255 : 0;
        setDMXchannel(3, value);
        setDMXchannel(4, value);
    }

    void setWorkLight(bool state) {
        int value = state ? 255 : 0;
        setDMXchannel(6, value);
    }

    void setDekoLight_1(bool state) {
        int value = state ? 255 : 0;
        setDMXchannel(1, value);
        setDMXchannel(2, value);
    }

    void setDekoLight_2(bool state) {
        int value = state ? 255 : 0;
        setDMXchannel(5, value);
    }

    void setCameoLED(int start_ch, int poti, int val){
        setDMXchannel(start_ch + 0, val); // Intensiti
        setDMXchannel(start_ch + 1, 0); // Strobe
        setDMXchannel(start_ch + 2, 255); // R
        setDMXchannel(start_ch + 3, 200); // G
        setDMXchannel(start_ch + 4, 0); // B
        setDMXchannel(start_ch + 5, 100); // W
        // setDMXchannel(start_ch + 6, poti); // Mode
        
        if(poti < 15){
            setDMXchannel(start_ch + 6, 0);
        }
        else{
            setDMXchannel(start_ch + 6, map(poti, 15, 255, 152, 210));
        }
    }

    void setAllCameoLED(int poti, int val){
        int start_ch = 20;
        int device_mode = 7;
        int device_count = 3;
        int max_chanel = start_ch + device_mode * device_count;

        for(start_ch; start_ch <= max_chanel; start_ch +=device_mode){
            setCameoLED(start_ch, poti, val);
        }
    }

    void updateLED() {
        if(eventBlockChange) {
            panel->setLED(true, false);  // Red when Event Mode
        }
        else if (heatingActiv) {
            panel->setLED(true, true);  // Yellow when Community Mode
        }
        else {
            panel->setLED(false, true); // Green when inactive, but Ready
        }
    }

    void applyModeSettings(int mode, int potiValue) {
        // Define DMX values for each mode
        // Adjust these values to your needs

        Serial.println("Mode: " + String(mode) + "\t potiValue: " + String(potiValue));

        switch(mode) {
            case 1:
                setAllCameoLED(0, 0);
                setDekoLight_1(false);
                setDekoLight_2(false);
                setWorkLight(false);
                break;
            case 2:
                setAllCameoLED(0, potiValue);
                setDekoLight_1(false);
                setDekoLight_2(false);
                setWorkLight(false);
                break;
            case 3:
                setAllCameoLED(potiValue, 128);
                setDekoLight_1(true);
                setDekoLight_2(false);
                setWorkLight(false);
                break;
            case 4:
                setAllCameoLED(potiValue, 128);
                setDekoLight_1(false);
                setDekoLight_2(true);
                setWorkLight(false);
                break;
            case 5:
                setAllCameoLED(0, 255);
                setDekoLight_1(false);
                setDekoLight_2(false);
                setWorkLight(true);
                break;
        }
    }

    void community_mode()
    {   
        
    }

    void deactivateHeating() {
        heatingActiv = false;
        heaterTimerRunning = false;
        setHeater(false);
    }

public:
    PanelLogic(ControlPanel* controlPanel) {
        panel = controlPanel;
        
        heatingActiv = false;
        eventBlockChange = false;
        pushButton = false;
        
        heaterTimerEnd = 0;
        heaterTimerRunning = false;
    }

    
    void update(){
        pushButton = panel->getPushButton();
        
        if(panel->getChangedBtnFlag())
        {    
            // Key switch logic
            if(panel->getKeySwitch()){
                eventBlockChange = !eventBlockChange;
                heaterTimerRunning = !eventBlockChange;
                Serial.println("EventBlockChange: " + String(eventBlockChange));
            }

            if (pushButton && !eventBlockChange) {
                heatingActiv = !heatingActiv;
                Serial.println("Heating Active: " + String(heatingActiv));
                heaterTimerRunning = true;
                heaterTimerEnd = millis() + HEATER_TIMER_DURATION;
                setHeater(heatingActiv);
            }
        }    
        
        if(panel->getChangedModeFlag())
        {
            if(!eventBlockChange)
            {
                applyModeSettings(
                    panel->getRotarySwitchMode(), 
                    panel->getPotentiometer());
            }
        }

        panel->resetChangedFlags();

        if (heaterTimerRunning && millis() >= heaterTimerEnd) {
            deactivateHeating();
        }
        
        updateLED();
    }

    uint8_t* getDMXbuffer(){
        return dmxBuffer;
    }

    // Query functions
    bool isheatingActiv() { return heatingActiv; }
    bool isTimerRunning() { return heaterTimerRunning; }
    unsigned long getTimerRemaining() {
        if (!heaterTimerRunning) return 0;
        unsigned long now = millis();
        if (now >= heaterTimerEnd) return 0;
        return heaterTimerEnd - now;
    }
};