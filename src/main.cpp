#include "core.h"
#include "encrypt.h"
#include "decrypt.h"
#include "constants.h"
#include "handshake.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>

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
}

void setup() {
    Serial.begin(9600);  // Initialize UART for debugging
    Serial1.begin(115200, SERIAL_8N1, 16, 17); // Initialize UART1 with TX=16, RX=17
    Serial.println("ESP32 UART Receiver Initialized");
    // Uncomment to test encryption/decryption if needed
    // test_ascon_encryption_decryption();
}

void loop() {
    // Run the three-way handshake continuously
    if (receiveThreeWayHandshake()) {
        Serial.println("Handshake succeeded!");
    } else {
        Serial.println("Handshake failed. Retrying...");
    }
}

uint8_t receivePacket(uint8_t expectedData, uint32_t timeout) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
        if (Serial1.available() > 0) {
            uint8_t receivedData = Serial1.read();
            if (receivedData == expectedData) {
                Serial.print("Receive Correct Packet: ");
                Serial.println(receivedData);
                return true; 
            }
            else{
                Serial.print("Receive Wrong Packet: ");
                Serial.println(receivedData);
            }
        }
        delay(10);
    }
    return false; 
}

void clearUartBuffer() {
    while (Serial1.available() > 0) {
        Serial1.read(); // Read and discard data
    }
}

uint8_t receiveThreeWayHandshake() {
    clearUartBuffer(); 
    if (!receivePacket(COMMAND_SYN, TIME_OUT)) {
        Serial.println("Failed to receive (Handshake step 1)");
        return 0; 
    }
    Serial.print("Received packet [");
    Serial.print(COMMAND_SYN);
    Serial.println("]");

    Serial1.write(COMMAND_SYN_ACK); // Send acknowledgment
    Serial1.flush();
    Serial.print("Sent acknowledgment [");
    Serial.print(COMMAND_SYN_ACK);
    Serial.println("]");

    clearUartBuffer(); 
    unsigned long startTime = millis();
    uint8_t state;
    Serial.println("Receiving final packet...");
    while (millis() - startTime < TIME_OUT){
        state = receivePacket(COMMAND_ACK, TIME_OUT);
        if(state) break;
    }

    if(state) {
        Serial.println("Three-way handshake success!");
        return 1;    
    }
    else {
        Serial.println("Unable to receive final packet!");
        return 0;
    }
    return 1; 
}

