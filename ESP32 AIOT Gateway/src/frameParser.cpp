#include "FrameParser.h"
#include "rfc7539.h"
#include "chacha20poly1305.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ENV_AAD 0xa8, 0x54, 0x2a, 0x95, 0x4a, 0xa5, 0xb6, 0x5b, 0x2d, 0x96, 0x4b, 0x25
#define ENV_KEY 0xe1, 0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0xe1, 0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0xe1, 0xf0, 0x78, 0x3c, \
                0x1e, 0x0f, 0xa6, 0x53, 0x29, 0x94, 0x4a, 0x25, 0x4c, 0x26, 0x93, 0xc9, 0x64, 0x32, 0xbf, 0x5f
#define ENV_NONCE 0x18, 0x0c, 0x86, 0x43, \
                  0x21, 0x90, 0x1c, 0x0e, 0x07, 0x83, 0x41, 0x20

#define DESIOT_ENCRYPT_TAG_SIZE 16

void encrypt_and_authenticate(const uint8_t *plaintext, size_t plaintext_len, const uint8_t *aad, size_t aad_len, uint8_t *ciphertext, uint8_t *tag, const uint8_t *key, const uint8_t *nonce) {
    memmove(ciphertext + DESIOT_ENCRYPT_TAG_SIZE, plaintext, plaintext_len);
    chacha20poly1305_ctx ctx;
    rfc7539_init(&ctx, key, nonce);
    chacha20poly1305_encrypt(&ctx, plaintext, ciphertext + DESIOT_ENCRYPT_TAG_SIZE, plaintext_len);
    rfc7539_auth(&ctx, aad, aad_len);
    rfc7539_finish(&ctx, aad_len, plaintext_len, tag);
}

void decrypt_and_verify(const uint8_t *ciphertext, size_t ciphertext_len, const uint8_t *aad, size_t aad_len, uint8_t *plaintext, const uint8_t *tag, const uint8_t *key, const uint8_t *nonce) {
    chacha20poly1305_ctx ctx;
    uint8_t computed_tag[DESIOT_ENCRYPT_TAG_SIZE];
    rfc7539_init(&ctx, key, nonce);
    rfc7539_auth(&ctx, aad, aad_len);
    size_t plaintext_len = ciphertext_len - DESIOT_ENCRYPT_TAG_SIZE;
    chacha20poly1305_decrypt(&ctx, ciphertext + DESIOT_ENCRYPT_TAG_SIZE, plaintext, plaintext_len);
    rfc7539_finish(&ctx, aad_len, plaintext_len, computed_tag);
    if (memcmp(computed_tag, tag, DESIOT_ENCRYPT_TAG_SIZE) != 0) {
        // printf("Authentication failed!\n");
    } else {
        printf("Authentication successful!\n");
    }

    printf("Computed Tag: ");
    for (size_t i = 0; i < sizeof(computed_tag); i++) {
        printf("%02x ", computed_tag[i]);
    }
    printf("\n");
}

uint16_t Compute_CRC16(uint8_t *bytes, const int32_t length) {
    uint16_t crc = 0xFFFF;
    for (int32_t i = 0; i < length; i++) {
        crc ^= (uint16_t)bytes[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x8005;  // CRC polynomial
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

String frameToJson(const Frame_t &frame) {
    // Create a JSON document with enough capacity
    StaticJsonDocument<512> doc;

    // Populate the JSON document with frame data
    doc["Header1"] = frame.h1;
    doc["Header2"] = frame.h2;
    doc["DeviceID"] = String(frame.dataPacket.deviceId);
    doc["DataLength"] = frame.dataPacket.dataLen;

    // Add sensor data
    JsonArray dataArray = doc.createNestedArray("Data");
    for (size_t i = 0; i < frame.dataPacket.dataLen; ++i) {
        dataArray.add(frame.dataPacket.data[i]);
    }

    // Add trailers and CRC
    doc["Trailer1"] = frame.t1;
    doc["Trailer2"] = frame.t2;
    doc["CRC"] = frame.crc;

    // Serialize the JSON document to a string
    String jsonString;
    serializeJson(doc, jsonString);

    return jsonString;
}

Frame_t parseFrame(uint8_t *buffer, size_t len, char* deviceId, uint8_t &heart_rate, uint8_t &spo2, uint8_t &temperature) {
    Frame_t newFrame = {0};  // Initialize the new frame to zero

    if (len < sizeof(Frame_t)) {
        Serial.println("Error: Incomplete frame");
        return newFrame;
    }

    Frame_t *frame = (Frame_t *)buffer;

    // Compute the CRC of the received data (excluding the CRC itself)
    uint16_t received_crc = frame->crc;
    frame->crc = 0;  // Set CRC field to 0 for CRC computation
    uint16_t computed_crc = Compute_CRC16((uint8_t*)frame, sizeof(Frame_t) - sizeof(frame->crc));
    frame->crc = received_crc;  // Restore the CRC field

    // Print CRC values for debugging
    Serial.print("Received CRC: ");
    Serial.println(received_crc, HEX);
    Serial.print("Computed CRC: ");
    Serial.println(computed_crc, HEX);

    // Verify CRC
    if (computed_crc != received_crc) {
        Serial.println("Error: CRC mismatch");
        return newFrame;
    }
    Serial.println("Data contents:");
    for (size_t i = 0; i < frame->dataPacket.dataLen; ++i) {
        Serial.print("Data[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(frame->dataPacket.data[i]);
    }

    // Encryption
    uint8_t aad[12] = {ENV_AAD};
    uint8_t key[32] = {ENV_KEY};
    uint8_t nonce[] = {ENV_NONCE};
    uint8_t tag[16] = {0}; // Buffer for Poly1305 tag, initialized to zero
    uint8_t encryptedData[frame->dataPacket.dataLen + DESIOT_ENCRYPT_TAG_SIZE];
    
    Serial.println("Encrypting...");
    encrypt_and_authenticate(frame->dataPacket.data, frame->dataPacket.dataLen, aad, sizeof(aad), encryptedData, tag, key, nonce);
    Serial.println("Encrypted Data:");
    for (size_t i = 0; i < frame->dataPacket.dataLen + DESIOT_ENCRYPT_TAG_SIZE; ++i) {
        if (encryptedData[i] < 0x10) {  // Add leading zero for single digit hex numbers
            Serial.print("0");
        }
        Serial.print(encryptedData[i], HEX);
        if (i < (frame->dataPacket.dataLen + DESIOT_ENCRYPT_TAG_SIZE - 1)) {
            Serial.print(" ");  // Separate bytes with a space
        }
    }
    Serial.println();
    // Decrypt the data for verification
    uint8_t decryptedData[frame->dataPacket.dataLen];
    Serial.println("Decrypting for verification...");
    decrypt_and_verify(encryptedData, frame->dataPacket.dataLen + DESIOT_ENCRYPT_TAG_SIZE, aad, sizeof(aad), decryptedData, tag, key, nonce);

    // Check if the original data matches the decrypted data
    if (memcmp(frame->dataPacket.data, decryptedData, frame->dataPacket.dataLen) == 0) {
        Serial.println("Encryption and decryption verified successfully.");
    } else {
        Serial.println("Error: Decrypted data does not match the original data.");
    }

    // Construct the new frame with encrypted data
    newFrame.h1 = frame->h1;
    newFrame.h2 = frame->h2;
    newFrame.dataPacket.dataLen = frame->dataPacket.dataLen + DESIOT_ENCRYPT_TAG_SIZE;
    newFrame.dataPacket.data = allocateData(newFrame.dataPacket.dataLen);
    memcpy(newFrame.dataPacket.deviceId, frame->dataPacket.deviceId, sizeof(frame->dataPacket.deviceId));
    memcpy(newFrame.dataPacket.data, encryptedData, newFrame.dataPacket.dataLen);
    newFrame.t1 = frame->t1;
    newFrame.t2 = frame->t2;
    newFrame.crc = Compute_CRC16((uint8_t*)&newFrame, sizeof(newFrame) - sizeof(newFrame.crc));

    // Log the frame details
    Serial.print("Header 1: ");
    Serial.println(newFrame.h1, HEX);
    Serial.print("Header 2: ");
    Serial.println(newFrame.h2, HEX);
    Serial.print("Device ID: ");
    Serial.println(newFrame.dataPacket.deviceId);
    Serial.print("Data Length: ");
    Serial.println(newFrame.dataPacket.dataLen);
    // Extract sensor data
    if (frame->dataPacket.dataLen >= 3) {  // Ensure enough data is present
        frame->dataPacket.data[0] = 98;
        heart_rate = frame->dataPacket.data[0];
        spo2 = frame->dataPacket.data[1];
        temperature = frame->dataPacket.data[2];

        // Print sensor data for debugging
        Serial.print("Heart Rates: ");
        Serial.println(heart_rate);
        Serial.print("SpO2: ");
        Serial.println(spo2);
        Serial.print("Temperature: ");
        Serial.println(temperature);

        // Copy device ID
        strncpy(deviceId, frame->dataPacket.deviceId, 32);
    } else {
        Serial.println("Error: Insufficient data length");
    }
    Serial.print("Trailer 1: ");
    Serial.println(newFrame.t1, HEX);
    Serial.print("Trailer 2: ");
    Serial.println(newFrame.t2, HEX);

    return newFrame;  // Return the constructed frame
}

uint8_t *allocateData(uint16_t dataLen)
{
    uint8_t *data = (uint8_t*)malloc(dataLen);
    if(data == NULL){
        Serial.println("Failed to allocate memory");
        return nullptr;
    }
    else{
        return data;
    }
}
