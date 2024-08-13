#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <stdint.h> 
#include <Arduino.h>
#include <ArduinoJson.h>

#define ATT_PACKED __attribute__((packed))

typedef struct {
    uint8_t h1;
    uint8_t h2;
    char deviceId[32];
    uint16_t dataLen : 10;
} ATT_PACKED HeadFrame_t;

typedef struct {
    uint8_t t1;
    uint8_t t2;
    uint16_t crc;
} ATT_PACKED TrailFrame_t;

typedef struct {
    char deviceId[32];
    union {
        uint8_t dataLenArr[2];
        uint16_t dataLen : 10;
    };
    uint8_t *data;  // Adjust the size as needed
} ATT_PACKED DataPacket_t;

typedef struct {
    uint8_t h1;
    uint8_t h2;
    DataPacket_t dataPacket;
    uint8_t t1;
    uint8_t t2;
    union {
        uint16_t crc;
        uint8_t crcArr[2];
    };
} ATT_PACKED Frame_t;

// Function to compute CRC16
uint16_t Compute_CRC16(uint8_t *bytes, const int32_t BYTES_LEN);
String frameToJson(const Frame_t &frame);
// Function to parse the received frame and extract sensor data
Frame_t parseFrame(uint8_t *buffer, size_t len, char* deviceId, uint8_t &heart_rate, uint8_t &spo2, uint8_t &temperature);
uint8_t *allocateData(uint16_t dataLen);
#endif // FRAMEPARSER_H