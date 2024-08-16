#include "core.h"
#include "encrypt.h"
#include "decrypt.h"
#include "constants.h"
#include "handshake.h"
#include "frameparsing.h"
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

ThreeWayHandshake handshake(Serial1, 5000);

void setup() {
    Serial.begin(9600);  
    Serial1.begin(115200, SERIAL_8N1, 16, 17); // Initialize UART1 with TX=16, RX=17
    Serial.println("ESP32 UART Receiver Initialized");
    
    // test_ascon_encryption_decryption();
}

void loop() {
    Frame_t received_frame;
    Encrypt_Frame_t encrypted_frame;
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
            }
        }

        transitionFrame(&received_frame, &encrypted_frame);

        int result = encryptDataPacket(&received_frame, &encrypted_frame);
        if (result != 1) {
            Serial.println("Encryption failed");
        } else {
            Serial.println("Encryption successful");
        }

        // Serial.print("Log Check: ");
        // for(int i = 0; i < 116; i++){
        //     Serial.print(encrypted_frame.dataEncrypted[i], HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();

    } else {
        Serial.println("Handshake failed.");
    }
}

