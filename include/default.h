#ifndef DEFAULT_H

#include <stdint.h>

#define AIOT_BAUD_RATE 115200
#define AIOT_TX 16
#define AIOT_RX 17

#define ASCON_KEY_SIZE 16
#define ASCON_NONCE_SIZE 16
#define ASCON_CIPHERTEXT_SIZE 64
#define ASCON_DECRYPTED_TEXT_SIZE 64

#define RESET_STATE 0

enum dataIndex{
    HEART_RATE,
    SPO2,
    TEMPERATURE,
    ACCELEROMETER,
    ANOMALY
};

typedef struct{
    float dataSum = 0;
    float dataMin = UINT16_MAX;
    float dataMax = 0;
} aggregateData;

typedef struct {aggregateData heartRate, temperature, accelerator, spO2;} dataToProcess;

dataToProcess metricsData;

unsigned long previousMillis = 0;
const long aggregationInterval = 30000; 
uint16_t dataCount = 0;

#endif