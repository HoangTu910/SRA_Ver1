#ifndef _HANDSHAKE_H_
#include <stdint.h>
#include <Arduino.h>

#define COMMAND_SYN 0x01
#define COMMAND_SYN_ACK 0x02
#define COMMAND_ACK 0x03
#define TIME_OUT 2000
#define UART Serial1

uint8_t receivePacket(uint8_t expectedData, uint32_t timeout);
uint8_t sendPacket(); // later
uint8_t receiveThreeWayHandshake();

class ThreeWayHandshake {
public:
    ThreeWayHandshake(HardwareSerial &serial, uint32_t timeout);
    bool performHandshake(uint8_t commandSyn, uint8_t commandSynAck, uint8_t commandAck);

private:
    HardwareSerial &serial;
    uint32_t timeout;

    bool receivePacket(uint8_t expectedData);
    void clearUartBuffer();
};

#endif