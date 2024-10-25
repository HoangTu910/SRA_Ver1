#ifndef _FRAME_H_
#define _FRAME_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdint.h>
#include "encrypt.h"
#include "decrypt.h"
#include "dhexchange.h"


#define H1_INDEX 0xAA
#define H2_INDEX 0xBB
#define T1_INDEX 0xCC
#define T2_INDEX 0xDD

#define MAX_DATA_SIZE 64
#define DEVICE_ID_LEN 32
#define MAX_DATA_ENCRYPT 255
#define NONCE_SIZE 16

extern unsigned char ASCON_SECRET_KEY[16];

// Ensure DataPacket_t is only defined here
typedef struct {
    char deviceId[DEVICE_ID_LEN];
    uint8_t deviceLen;   
    union {
        uint8_t dataLenArr[2];
        uint8_t dataLen; 
    };
    uint8_t data[MAX_DATA_SIZE];
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
    unsigned char dataEncrypted[MAX_DATA_ENCRYPT];
    uint8_t t1;
    uint8_t t2;
    unsigned char nonce[NONCE_SIZE];
    union {
        uint16_t crc;
        uint8_t crcArr[2];  // CRC as array of 2 bytes
    };
} __attribute__((packed)) Encrypt_Frame_t;

// Function declarations
uint16_t Compute_CRC16(uint8_t* data, size_t length);
void log_error(const char* error_message);
int parse_frame(uint8_t* received_data, size_t length);
uint8_t transitionFrame(Frame_t frame, Encrypt_Frame_t *en_frame);
int encryptDataPacket(Frame_t *frame, Encrypt_Frame_t *en_frame, unsigned long long* getLen);
int decryptDataPacket(Encrypt_Frame_t *en_frame);
uint8_t reconstructDecryptedData(unsigned char *decryptedtext);
uint8_t *hexToBinary(const char *hex, size_t hexLen, size_t *binaryLen);
String serializeToJSON(Encrypt_Frame_t& frame, unsigned long long clen);
template <typename anyInt>
void resetData(anyInt *data) {
    *data = 0;
}
void TFLiteModeProcessAnomaly(Frame_t *frame);
bool ParseFrameProcess(Frame_t *frame);
// unsigned char* performKeyExchange();

#endif // _FRAME_H_
