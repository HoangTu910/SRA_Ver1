#include "core.h"
#include "encrypt.h"
#include "decrypt.h"
#include "constants.h"
#include "handshake.h"
#include "gateway.h"
#include "TFLiteModel.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <PubSubClient.h>

#ifdef DEFAULT
#undef DEFAULT
#endif

int clenForTrans;
TFLiteModel tfliteModel;

typedef struct{
    uint16_t dataSum = 0;
    uint16_t dataMin = UINT16_MAX;
    uint16_t dataMax = 0;
} aggregateData;

unsigned long previousMillis = 0;
const long aggregationInterval = 30000; 
uint16_t dataCount = 0;

aggregateData heartRate, temperature, accelerator, spO2;

void test_ascon_encryption_decryption() {
    const unsigned char plaintext[] = "Encrypt Data 9999";
    const unsigned char key[16] = ASCON_KEY;  // 128-bit key
    unsigned char nonce[16]; generate_nonce(nonce);
    const unsigned char assoc_data[] = "Metadata"; // Optional
    unsigned char ciphertext[64];
    unsigned char decryptedtext[64];
    unsigned long long clen = 0;
    unsigned long long decrypted_len = 0;

    // Encrypt the message
    crypto_aead_encrypt(ciphertext, &clen, plaintext, strlen((char *)plaintext), 
                        assoc_data, strlen((char *)assoc_data), 
                        NULL, nonce, key);

    // Output the ciphertext
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

ThreeWayHandshake handshake(Serial1, 5000);

void setup() {
    Serial.begin(115200);  
    tfliteModel.Initialize();
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    Serial1.begin(115200, SERIAL_8N1, 16, 17); // Initialize UART1 with TX=16, RX=17
    Serial.println("ESP32 UART Receiver Initialized");
    // test_ascon_encryption_decryption();
    tfliteModel.Initialize();
}

void loop() {
    mqtt_setup();
    Frame_t received_frame;
    Encrypt_Frame_t encrypted_frame;
    unsigned long long clen;
    if (!client.connected()) {
        Serial.println("MQTT client not connected, attempting to reconnect...");
        reconnect();
    } else {
        client.loop();  
    }
    Serial.println("----------Three way handshake---------");
    if (handshake.performHandshake(COMMAND_SYN, COMMAND_SYN_ACK, COMMAND_ACK)) {
        Serial.println("Handshake completed successfully!");
        Serial.println("Starting parsing frame...");
        Serial.println("----------Received Frame---------");
        if (Serial1.available() >= sizeof(Frame_t)) {
            // Read frame data from UART
            Serial1.readBytes((uint8_t*)&received_frame, sizeof(received_frame));

            int result = parse_frame((uint8_t*)&received_frame, sizeof(received_frame));

            if (result != 0) {
                Serial.print("Failed to parse frame, error code: ");
                Serial.println(result);
                return;
            }
        }
        bool isAnomaly = false;
        //tfliteModel.Cleanup();
        if (tfliteModel.interpreter == nullptr) {
            Serial.println("Re-initializing interpreter...");
            if (!tfliteModel.Initialize()) {
                Serial.println("Failed to re-setup interpreter.");
                return;
            }
        }
        tfliteModel.PerformInference(float(received_frame.dataPacket.data[0]), float(received_frame.dataPacket.data[3]), float(received_frame.dataPacket.data[2]), &isAnomaly);
        if(!isAnomaly){
            Serial.println("Normal! Aggregating...");
            heartRate.dataSum += received_frame.dataPacket.data[0];
            spO2.dataSum += received_frame.dataPacket.data[1];
            temperature.dataSum += received_frame.dataPacket.data[2];
            accelerator.dataSum += received_frame.dataPacket.data[3];
            dataCount++;
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= aggregationInterval) {
                Serial.println("Aggregate completed! Sending...");
                previousMillis = currentMillis;
                received_frame.dataPacket.data[0] = heartRate.dataSum / dataCount;
                received_frame.dataPacket.data[1] = spO2.dataSum / dataCount;
                received_frame.dataPacket.data[2] = temperature.dataSum / dataCount;
                received_frame.dataPacket.data[3] = accelerator.dataSum / dataCount;

                resetData(&heartRate.dataSum);
                resetData(&spO2.dataSum);
                resetData(&temperature.dataSum);
                resetData(&accelerator.dataSum);
                resetData(&dataCount);

                transitionFrame(received_frame, &encrypted_frame);
                int result = encryptDataPacket(&received_frame, &encrypted_frame, &clen);
                switch(result){
                    case 1: Serial.println("Encryption successful"); break;
                    default: Serial.println("Encryption failed"); break;
                }
                if(handshake.handshakeWithServer(SERVER_COMMAND_SYN, SERVER_SYN_ACK, SERVER_COMMAND_ACK)){
                    publishFrame(encrypted_frame, dataTopic, clen);
                }
            }
        }
        else{
            Serial.println("Anomaly Detected! Encrypt and update");
            transitionFrame(received_frame, &encrypted_frame);
            int result = encryptDataPacket(&received_frame, &encrypted_frame, &clen);
            switch(result){
                case 1: Serial.println("Encryption successful"); break;
                default: Serial.println("Encryption failed"); break;
            }
            resetData(&heartRate.dataSum);
            resetData(&spO2.dataSum);
            resetData(&temperature.dataSum);
            resetData(&accelerator.dataSum);
            resetData(&dataCount);
            if(handshake.handshakeWithServer(SERVER_COMMAND_SYN, SERVER_SYN_ACK, SERVER_COMMAND_ACK)){
                publishFrame(encrypted_frame, dataTopic, clen);
            }
        }
    } else {
        Serial.println("Handshake failed.");
        return;
    }
}

