#include "handshake.h"

ThreeWayHandshake::ThreeWayHandshake(HardwareSerial &serialPort, uint32_t timeoutDuration)
    : serial(serialPort), timeout(timeoutDuration) {}

bool ThreeWayHandshake::receivePacket(uint8_t expectedData) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
        if (serial.available() > 0) {
            uint8_t receivedData = serial.read();
            if (receivedData == expectedData) {
                Serial.print("Received Correct Packet: ");
                Serial.println(receivedData);
                return true;
            } else {
                Serial.print("Received Wrong Packet: ");
                Serial.println(receivedData);
            }
        }
        delay(10);
    }
    return false;
}

void ThreeWayHandshake::clearUartBuffer() {
    while (serial.available() > 0) {
        serial.read(); // Read and discard data
    }
}

bool ThreeWayHandshake::performHandshake(uint8_t commandSyn, uint8_t commandSynAck, uint8_t commandAck) {
    clearUartBuffer();
    if (!receivePacket(commandSyn)) {
        Serial.println("Failed to receive (Handshake step 1)");
        return false;
    }
    Serial.print("Received packet [");
    Serial.print(commandSyn);
    Serial.println("]");

    serial.write(commandSynAck); // Send acknowledgment
    serial.flush();
    Serial.print("Sent acknowledgment [");
    Serial.print(commandSynAck);
    Serial.println("]");

    clearUartBuffer();
    unsigned long startTime = millis();
    bool state = false;
    Serial.println("Receiving final packet...");
    while (millis() - startTime < timeout) {
        state = receivePacket(commandAck);
        if (state) break;
    }

    if (state) {
        Serial.println("Three-way handshake success!");
        return true;
    } else {
        Serial.println("Unable to receive final packet!");
        return false;
    }
}