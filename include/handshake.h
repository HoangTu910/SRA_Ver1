#ifndef _HANDSHAKE_H_
#include <stdint.h>
#include <Arduino.h>
#include "gateway.h"

#ifdef DEFAULT
#undef DEFAULT
#endif

#define COMMAND_SYN 0x01
#define COMMAND_SYN_ACK 0x02
#define COMMAND_ACK 0x03

#define SERVER_COMMAND_SYN 0xA1
#define SERVER_SYN_ACK 0xA2
#define SERVER_COMMAND_ACK 0xA3

#define TIME_OUT 2000
#define UART Serial1

uint8_t receivePacket(uint8_t expectedData, uint32_t timeout);
uint8_t sendPacket(); // later
uint8_t receiveThreeWayHandshake();

class ThreeWayHandshake {
public:
    ThreeWayHandshake(HardwareSerial &serial, uint32_t timeout);
    bool performHandshake(uint8_t commandSyn, uint8_t commandSynAck, uint8_t commandAck);
    bool handshakeWithServer(uint8_t commandSyn, uint8_t commandSynAck, uint8_t commandAck);
private:
    HardwareSerial &serial;
    uint32_t timeout = 2000;
    String synTopic = "handshake/syn";
    String synAckTopic = "handshake/syn-ack";
    String ackTopic = "handshake/ack";

    bool receivePacket(uint8_t expectedData);
    void clearUartBuffer();
    void mqttCallback(char* topic, byte* payload, unsigned int length);
    bool waitForSynAck();
    bool receivedAckSyn2 = false;
    int serverPublicCheck(DH_KEY serverPublicKey);
    void generateSecretKey(DH_KEY clientSecret);
};

#endif