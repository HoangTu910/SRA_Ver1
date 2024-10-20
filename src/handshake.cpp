#ifndef _HANDSHAKE_H
#define _HANDSHAKE_H
#include "handshake.h"

ThreeWayHandshake::ThreeWayHandshake(HardwareSerial &serialPort, uint32_t timeoutDuration)
    : serial(serialPort), timeout(timeoutDuration) {}

bool ThreeWayHandshake::receivePacket(uint8_t expectedData) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
        if (serial.available() > 0) {
            uint8_t receivedData = serial.read();
            if(receivedData == 255){
                Serial.print("Error transmit: ");
                Serial.println(receivedData);
            }
            else if (receivedData == expectedData) {
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

void ThreeWayHandshake::mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    
    // Print the payload as a string
    String messageTemp;
    for (unsigned int i = 0; i < length; i++) {
        messageTemp += (char)payload[i];
    }
    Serial.println(messageTemp);

    // Print the payload in HEX format for debugging
    String mesHex = bytesToHexString(payload, length);
    Serial.print("Message in HEX: ");
    Serial.println(mesHex);

    // Check if the topic matches and if the message matches expected SYN-ACK
    if (String(topic) == "handshake/syn-ack") {
        if (mesHex == String(SERVER_SYN_ACK, HEX)) {
            Serial.println("SYN-ACK match found. Received SYN-ACK.");
            this->receivedAckSyn2 = true;
        } else {
            Serial.println("SYN-ACK mismatch. Expected: " + String(SERVER_SYN_ACK, HEX) + ", Received: " + mesHex);
        }
    } else {
        Serial.println("Topic did not match 'handshake/syn-ack'.");
    }
}

bool ThreeWayHandshake::waitForSynAck() {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
        client.loop(); 
        if (receivedSynAck) {
            return true;
        }
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
        Serial.println("Three-way handshake [PASSED]");
        return true;
    } else {
        Serial.println("Unable to receive final packet!");
        return false;
    }
}

bool ThreeWayHandshake::handshakeWithServer(uint8_t commandSyn, uint8_t commandSynAck, uint8_t commandAck) {
    if (!client.connected()) {
        Serial.println("MQTT client not connected.");
        return false;
    }
    receivedAckSyn2 = false;
    client.subscribe(synAckTopic.c_str());
    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->mqttCallback(topic, payload, length);
    }); 

    // Publish SYN command to server
    byte payloadSyn[] = { commandSyn };
    if (client.publish(synTopic.c_str(), payloadSyn, sizeof(payloadSyn))) {
        Serial.print("Sent command SYN [");
        Serial.print(commandSyn, HEX);
        Serial.println("]");
    } else {
        Serial.println("Failed to send SYN command.");
        return false;
    }

    unsigned long startTime = millis();

    Serial.println("Waiting for SYN-ACK response...");
    while (millis() - startTime < timeout) {
        client.loop(); 
        if (receivedAckSyn2) {
            Serial.println("SYN-ACK received.");
            break;
        }
    }

    if (!receivedAckSyn2) {
        Serial.println("Timeout waiting for SYN-ACK");
        return false; 
    }
    receivedAckSyn2 = false;
    byte payloadAck[] = { commandAck };
    if (client.publish(ackTopic.c_str(), payloadAck, sizeof(payloadAck))) {
        Serial.print("Sent command ACK [");
        Serial.print(commandAck, HEX);
        Serial.println("]");
    } else {
        Serial.println("Failed to send ACK command.");
        return false;
    }

    return true; // Handshake completed successfully
}
#endif