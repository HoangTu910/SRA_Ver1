#include "core.h"
#include "encrypt.h"
#include "decrypt.h"
#include "constants.h"
#include "handshake.h"
#include "gateway.h"
#include "TFLiteModel.h"
#include "default.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <PubSubClient.h>

#ifdef DEFAULT
#undef DEFAULT
#endif

int clenForTrans;
TFLiteModel tfliteModel;
ThreeWayHandshake handshake(Serial1, 5000);

void test_ascon_encryption_decryption();
void VerifyInterpreterReset();
void AggregateData(dataToProcess* data, Frame_t* received_frame);
void ProcessAverage(Frame_t* received_frame, dataToProcess* data, uint16_t &dataCount);
bool IsFinishedAggregate(unsigned long* currentMillis, unsigned long* previousMillis, const long timeInterval);
void ResetCompleteStruct(dataToProcess* metricsData);
void ExecutePredictionModel(Frame_t &received_frame, bool *isAnomaly);

void setup() {
    Serial.begin(AIOT_BAUD_RATE);  
    tfliteModel.Initialize();
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    Serial1.begin(AIOT_BAUD_RATE, SERIAL_8N1, AIOT_TX, AIOT_RX); // Initialize UART1 with TX=16, RX=17
    Serial.println("ESP32 UART Receiver Initialized");
}

void loop() {
    mqtt_setup();
    Frame_t received_frame;
    Encrypt_Frame_t encrypted_frame;
    bool isAnomaly = false;
    unsigned long long clen;
    Serial.println("----------Three way handshake---------");
    if (handshake.performHandshake(COMMAND_SYN, COMMAND_SYN_ACK, COMMAND_ACK)) {
        if(!ParseFrameProcess(&received_frame)) return;
        //Setup model
        VerifyInterpreterReset();
        ExecutePredictionModel(received_frame, &isAnomaly);

        if(!isAnomaly){
            Serial.println("Normal! Aggregating...");
            AggregateData(&metricsData, &received_frame);
            dataCount++;
            unsigned long currentMillis = millis();
            if (IsFinishedAggregate(&currentMillis, &previousMillis, aggregationInterval)) {
                Serial.println("Aggregate completed! Sending...");

                ProcessAverage(&received_frame, &metricsData, dataCount);
                transitionFrame(received_frame, &encrypted_frame);

                int encryptResult = encryptDataPacket(&received_frame, &encrypted_frame, &clen);
                if(encryptResult && handshake.handshakeWithServer(SERVER_COMMAND_SYN, SERVER_SYN_ACK, SERVER_COMMAND_ACK)){
                    publishFrame(encrypted_frame, dataTopic, clen);
                }
            }
        } else{
            Serial.println("Anomaly Detected! Encrypt and update");
            transitionFrame(received_frame, &encrypted_frame);
            int encryptResult = encryptDataPacket(&received_frame, &encrypted_frame, &clen);
            ResetCompleteStruct(&metricsData);
            resetData(&dataCount);
            if(encryptResult && handshake.handshakeWithServer(SERVER_COMMAND_SYN, SERVER_SYN_ACK, SERVER_COMMAND_ACK)){
                publishFrame(encrypted_frame, dataTopic, clen);
            }
        }
    } else {
        Serial.println("Handshake failed.");
        return;
    }
}

void test_ascon_encryption_decryption() {
    const unsigned char plaintext[] = "Encrypt Data 9999";
    const unsigned char key[16] = ASCON_KEY;  // 128-bit key
    unsigned char nonce[16]; generate_nonce(nonce);
    const unsigned char assoc_data[] = "Metadata"; // Optional
    unsigned char ciphertext[64];
    unsigned char decryptedtext[64];
    unsigned long long clen = 0;
    unsigned long long decrypted_len = 0;

    crypto_aead_encrypt(ciphertext, &clen, plaintext, strlen((char *)plaintext), 
                        assoc_data, strlen((char *)assoc_data), 
                        NULL, nonce, key);

    Serial.print("Ciphertext: ");
    for (unsigned long long i = 0; i < clen; i++) {
        if (ciphertext[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(ciphertext[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    //Tag
    Serial.print("Tag: ");
    for (unsigned long long i = clen - CRYPTO_ABYTES; i < clen; i++) {
        if (ciphertext[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(ciphertext[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    // Decrypt the message
    if (crypto_aead_decrypt(decryptedtext, &decrypted_len, NULL, ciphertext, clen, 
                            assoc_data, strlen((char *)assoc_data), 
                            nonce, key) == 0) {
        // Null terminate the decrypted text if it's supposed to be a string
        if (decrypted_len < sizeof(decryptedtext)) {
            decryptedtext[decrypted_len] = '\0';
        }
        
        Serial.print("Decrypted text: ");
        for (unsigned long long i = 0; i < decrypted_len; i++) {
            Serial.print((char)decryptedtext[i]);
        }
        Serial.println();
    } else {
        Serial.println("Decryption failed!");
    }
    clenForTrans = clen;
}

void VerifyInterpreterReset(){
    if (tfliteModel.interpreter == nullptr) {
        Serial.println("Re-initializing interpreter...");
        if (!tfliteModel.Initialize()) {
            Serial.println("Failed to re-setup interpreter.");
            return;
        }
    }
}

void AggregateData(dataToProcess* data, Frame_t* received_frame){
    data->heartRate.dataSum += received_frame->dataPacket.data[0];
    data->spO2.dataSum += received_frame->dataPacket.data[1];
    data->temperature.dataSum += received_frame->dataPacket.data[2];
    data->accelerator.dataSum += received_frame->dataPacket.data[3];
}

void ProcessAverage(Frame_t* received_frame, dataToProcess* data, uint16_t &dataCount){
    received_frame->dataPacket.data[0] = data->heartRate.dataSum / dataCount;
    received_frame->dataPacket.data[1] = data->spO2.dataSum / dataCount;
    received_frame->dataPacket.data[2] = data->temperature.dataSum / dataCount;
    received_frame->dataPacket.data[3] = data->accelerator.dataSum / dataCount;

    resetData(&data->heartRate.dataSum);
    resetData(&data->spO2.dataSum);
    resetData(&data->temperature.dataSum);
    resetData(&data->accelerator.dataSum);
    resetData(&dataCount);
}

bool IsFinishedAggregate(unsigned long* currentMillis, unsigned long* previousMillis, const long timeInterval){
    if (*currentMillis - *previousMillis >= timeInterval) {
        *previousMillis = *currentMillis;  
        return true;
    } else {
        return false;
    }
}

void ResetCompleteStruct(dataToProcess* metricsData){
    metricsData->accelerator.dataSum = 0;
    metricsData->heartRate.dataSum = 0;
    metricsData->spO2.dataSum = 0;
    metricsData->temperature.dataSum = 0;
}

void ExecutePredictionModel(Frame_t &received_frame, bool* isAnomaly)
{
    tfliteModel.PerformInference(received_frame.dataPacket.data[HEART_RATE], 
                                     received_frame.dataPacket.data[ACCELEROMETER], 
                                     received_frame.dataPacket.data[TEMPERATURE], 
                                     &*isAnomaly); //run model
}
