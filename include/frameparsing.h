#ifndef _FRAME_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdint.h>
#include "encrypt.h"
#include "decrypt.h"

#define H1_INDEX 0xAA
#define H2_INDEX 0xBB
#define T1_INDEX 0xCC
#define T2_INDEX 0xDD

typedef struct {
    char deviceId[32];
    uint16_t deviceLen : 10;   
    union {
        uint8_t dataLenArr[2];
        uint16_t dataLen : 10; 
    };
    uint8_t data[64];
} __attribute__((packed)) DataPacket_t;

typedef struct {
    uint8_t h1;
    uint8_t h2;
    DataPacket_t dataPacket;
    uint8_t t1;
    uint8_t t2;
    union {
        uint16_t crc;
        uint8_t crcArr[2];  // CRC as array of 2 bytes
    };
} __attribute__((packed)) Frame_t;

typedef struct {
    uint8_t h1;
    uint8_t h2;
    unsigned char dataEncrypted[255];
    uint8_t t1;
    uint8_t t2;
    unsigned char nonce[16];
    union {
        uint16_t crc;
        uint8_t crcArr[2];  // CRC as array of 2 bytes
    };
} __attribute__((packed)) Encrypt_Frame_t;

uint16_t Compute_CRC16(uint8_t* data, size_t length);
void log_error(const char* error_message);
int parse_frame(uint8_t* received_data, size_t length);
uint8_t transitionFrame(Frame_t frame, Encrypt_Frame_t *en_frame);
int encryptDataPacket(Frame_t *frame, Encrypt_Frame_t *en_frame, unsigned long long* getLen);
int decryptDataPacket(Encrypt_Frame_t *en_frame);
uint8_t reconstructDecryptedData(unsigned char *decryptedtext);
uint8_t *hexToBinary(const char *hex, size_t hexLen, size_t *binaryLen);
String serializeToJSON(Encrypt_Frame_t& frame, unsigned long long clen);
#endif