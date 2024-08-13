#include <Arduino.h>
#include <ArduinoJson.h>
#include "connectionConfig.h"
#include "FrameParser.h"

#define UART Serial1

int readHeartRate() {
    return random(60, 100); 
}

int readSpO2() {
    return random(95, 100); 
}

int readTemperature() {
    return random(20, 30) + random(0, 100) / 100; 
}


void setup() {
    Serial.begin(9600);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    UART.begin(115200, SERIAL_8N1, 16, 17);
    Serial.println("ESP32 UART Receiver Initialized");
}


void loop() {
    unsigned long now = millis();

    // Buffer to hold incoming UART data
    static uint8_t buffer[2048];
    static size_t buffer_len = 0;

    // Variables to hold extracted sensor data
    static uint8_t heart_rate = 0;
    static uint8_t spo2 = 0;
    static uint8_t temperature = 0;
    char device_id[32] = "";

    // Read incoming data from UART
    if (!client.connected()) {
        Serial.println("MQTT client not connected, attempting to reconnect...");
        reconnect();
    } else {
        client.loop();  
    }

    while (UART.available()) {
        buffer[buffer_len++] = UART.read();

        // If the buffer length exceeds the frame size, reset
        if (buffer_len > sizeof(buffer)) {
            Serial.println("Buffer overflow, resetting buffer");
            buffer_len = 0;
            continue;
        }

        // Parse the frame if we have enough data
        if (buffer_len >= sizeof(Frame_t)) {
            Frame_t newFrame = parseFrame(buffer, buffer_len, device_id, heart_rate, spo2, temperature);
            buffer_len = 0;  // Reset buffer after parsing

            String jsonPayload = frameToJson(newFrame); //Construct frame to JSON
            // Convert the frame to a binary payload
            uint8_t *framePtr = (uint8_t *)&newFrame;  // Pointer to the start of the frame
            size_t frameSize = sizeof(newFrame);

            // Ensure the binary payload fits within the buffer
            if (frameSize > sizeof(buffer)) {
                Serial.println("Frame size exceeds buffer limit, not publishing");
                return;
            }

            // Publish the binary frame
            String topic = "sensors/data";
            if (client.publish(topic.c_str(), jsonPayload.c_str(), true)) {
                Serial.print("Published JSON frame: ");
                Serial.println("Success");
                Serial.println("JSON Payload:");
                Serial.println(jsonPayload);
                Serial.print("Frame size: ");
                Serial.println(sizeof(newFrame));
            } else {
                Serial.println("Publish failed.");
                Serial.print("Frame size: ");
                Serial.println(sizeof(newFrame));
                Serial.println("JSON Payload:");
                Serial.println(jsonPayload);
            }
        }
    }
}



    // if (!client.connected()) {
    //     reconnect();
    // }
    // client.loop();

    // static unsigned long lastPublishTime = 0;
    // unsigned long now = millis();
    // if (now - lastPublishTime > 5000) {
    //     lastPublishTime = now;

    //     //Sensor Data Demo
    //     int heartRate = readHeartRate();
    //     int spO2 = readSpO2();
    //     int temperature = readTemperature();

    //     // Create a JSON object
    //     StaticJsonDocument<200> jsonDoc;
    //     jsonDoc["id_device"] = device_id;
    //     jsonDoc["heart_rate"] = heartRate;
    //     jsonDoc["spO2"] = spO2;
    //     jsonDoc["temperature"] = temperature;
    //     jsonDoc["timestamp"] = now;
    //     // Serialize JSON to string
    //     String payload;
    //     serializeJson(jsonDoc, payload);

    //     String topic = "sensors/" + String(device_id) + "/data";

    //     if (client.publish(topic.c_str(), payload.c_str(), true)) {
    //         Serial.print("Published: ");
    //         Serial.println(payload);
    //     } else {
    //         Serial.println("Publish failed");
    //     }
    // }
