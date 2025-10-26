#pragma once

#include <Arduino.h>
#include <WebServer.h>

#define LOG_BUFFER_SIZE 50
#define LOG_MAX_LENGTH 128

// Circular buffer for log messages
class LogBuffer {
private:
    String messages[LOG_BUFFER_SIZE];
    int head = 0;
    int count = 0;

public:
    void addMessage(const String& msg) {
        messages[head] = msg;
        head = (head + 1) % LOG_BUFFER_SIZE;
        if (count < LOG_BUFFER_SIZE) count++;
    }

    int getCount() const { return count; }

    String getMessage(int index) const {
        if (index >= count) return "";
        int actualIndex = (head - count + index + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;
        return messages[actualIndex];
    }

    void clear() {
        count = 0;
        head = 0;
    }
};

// Simple logger that captures messages
class WebLogger {
private:
    LogBuffer buffer;

public:
    WebLogger() {}

    void log(const String& message) {
        String timestamp = String(millis() / 1000) + "s";
        String logEntry = "[" + timestamp + "] " + message;
        buffer.addMessage(logEntry);
    }

    // Get all messages as JSON array
    String getMessagesJSON() {
        String json = "[";
        int count = buffer.getCount();
        for (int i = 0; i < count; i++) {
            if (i > 0) json += ",";
            String msg = buffer.getMessage(i);
            msg.replace("\\", "\\\\");
            msg.replace("\"", "\\\"");
            msg.replace("\n", "\\n");
            msg.replace("\r", "\\r");
            json += "\"" + msg + "\"";
        }
        json += "]";
        return json;
    }

    void clearBuffer() {
        buffer.clear();
    }
};

// Global logger instance
WebLogger* webLogger = nullptr;

void setupWebLogger() {
    if (!webLogger) {
        webLogger = new WebLogger();
    }
}

// Helper macros for logging that go to both Serial AND web log
#define LOG_PRINT(x) do { Serial.print(x); if(webLogger) webLogger->log(String(x)); } while(0)
#define LOG_PRINTLN(x) do { Serial.println(x); if(webLogger) webLogger->log(String(x)); } while(0)
#define LOG_PRINTF(...) do { Serial.printf(__VA_ARGS__); if(webLogger) { char buf[256]; snprintf(buf, sizeof(buf), __VA_ARGS__); webLogger->log(String(buf)); } } while(0)

// API to get logs as JSON
void handleGetLogs(WebServer* server) {
    if (webLogger) {
        String json = "{\"logs\":" + webLogger->getMessagesJSON() + "}";
        server->send(200, "application/json", json);
    } else {
        server->send(500, "application/json", "{\"error\":\"Logger not initialized\"}");
    }
}

// API to clear logs
void handleClearLogs(WebServer* server) {
    if (webLogger) {
        webLogger->clearBuffer();
        server->send(200, "application/json", "{\"success\":true}");
    } else {
        server->send(500, "application/json", "{\"error\":\"Logger not initialized\"}");
    }
}
