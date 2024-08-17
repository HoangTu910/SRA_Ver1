#include "frameparsing.h"

uint16_t Compute_CRC16(uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (int32_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
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

void log_error(const char *error_message)
{
    Serial.println(error_message);
}

int parse_frame(uint8_t *received_data, size_t length)
{
    if (length != sizeof(Frame_t)) {
        log_error("Invalid frame size");
        return -1;  
    }

    Frame_t* frame = (Frame_t*)received_data;

    // Validate headers
    if (frame->h1 != H1_INDEX || frame->h2 != H2_INDEX) {
        Serial.println(frame->h1);
        Serial.println(frame->h2);
        log_error("Invalid frame headers");
        return -2; 
    }

    // Validate device length 
    if (frame->dataPacket.deviceLen != 32) {  
        log_error("Invalid device length");
        return -3;  
    }

    // Validate T1 and T2 fields
    if (frame->t1 != 0xCC || frame->t2 != 0xDD) {
        log_error("Invalid trailer values");
        return -4;  
    }

    // Validate CRC
    uint16_t received_crc = frame->crc;
    uint16_t computed_crc = Compute_CRC16(received_data, length - sizeof(frame->crc));
    if (received_crc != computed_crc) {
        Serial.print("Received CRC: ");
        Serial.println(received_crc);
        Serial.print("Computed CRC: ");
        Serial.println(computed_crc);
        log_error("CRC mismatch");
        return -5;  
    }
    else{
        Serial.print("Received CRC: ");
        Serial.println(received_crc);
        Serial.print("Computed CRC: ");
        Serial.println(computed_crc);
        Serial.println("CRC match");
    }

    //process the data
    Serial.println("Frame parsed successfully");
    Serial.print("H1: ");
    Serial.println(frame->h1);
    Serial.print("H2: ");
    Serial.println(frame->h2);
    Serial.print("Device ID Len: ");
    Serial.println(frame->dataPacket.deviceLen);
    Serial.print("Device ID: ");
    Serial.println(frame->dataPacket.deviceId);
    Serial.print("Data Len: ");
    Serial.println(frame->dataPacket.dataLen);
    Serial.print("Heart Rate: ");
    Serial.println(frame->dataPacket.data[0]);
    Serial.print("SpO2: ");
    Serial.println(frame->dataPacket.data[1]);
    Serial.print("Temperature: ");
    Serial.println(frame->dataPacket.data[2]);
    Serial.print("T1: ");
    Serial.println(frame->t1);
    Serial.print("T2: ");
    Serial.println(frame->t2);
    return 0;  // Success
}

uint8_t transitionFrame(Frame_t *frame, Encrypt_Frame_t *en_frame)
{
    frame->h1 = en_frame->h1;
    frame->h2 = en_frame->h2;
    frame->crc = en_frame->crc;
    frame->t1 = en_frame->t1;
    frame->t2 = en_frame->t2;
}

int encryptDataPacket(Frame_t *frame, Encrypt_Frame_t *en_frame)
{
    unsigned char *dataPacket = (unsigned char *)&frame->dataPacket;
    Serial.print("Data Log: ");
    for (size_t i = 0; i < sizeof(frame->dataPacket); i++) {
        Serial.print(dataPacket[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    size_t dataPacketLen = sizeof(frame->dataPacket);
    unsigned char ciphertext[dataPacketLen + CRYPTO_ABYTES];  
    unsigned long long ciphertextLen;
    const unsigned char key[16] = ASCON_KEY; 
    unsigned char nonce[16]; 
    generate_nonce(nonce);  
    const unsigned char assoc_data[] = "Metadata";

    // Encrypt the data
    int ret = crypto_aead_encrypt(ciphertext, &ciphertextLen, dataPacket, dataPacketLen,
                                 assoc_data, strlen((char *)assoc_data),
                                 NULL, nonce, key);
    if (ret != 0) {
        Serial.println("Encryption failed!");
        return ret;
    }

    Serial.print("Encrypted Data: ");
    for (size_t i = 0; i < ciphertextLen; i++) {
        Serial.print(ciphertext[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    memcpy(en_frame->dataEncrypted, ciphertext, ciphertextLen);

    unsigned char decryptedtext[255];  
    unsigned long long decrypted_len = 0;

    int decrypt_ret = crypto_aead_decrypt(decryptedtext, &decrypted_len, NULL, en_frame->dataEncrypted, ciphertextLen, 
                                          assoc_data, strlen((char *)assoc_data), 
                                          nonce, key);
    if (decrypt_ret == 0) {
        if (decrypted_len < sizeof(decryptedtext)) {
            decryptedtext[decrypted_len] = '\0';
        }
        
        Serial.print("Decrypted text: ");
        for (unsigned long long i = 0; i < decrypted_len; i++) {
            Serial.print(decryptedtext[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Decryption failed!");
    }

    return 1;  
}


int decryptDataPacket(Encrypt_Frame_t* en_frame){
    const unsigned char key[16] = ASCON_KEY;  // 128-bit key
    unsigned char nonce[16]; generate_nonce(nonce);
    const unsigned char assoc_data[] = "Metadata"; // Optional
    unsigned long long clen = 116;
    unsigned long long decrypted_len = 0;
    unsigned char decryptedtext[100];
    if (crypto_aead_decrypt(decryptedtext, &decrypted_len, NULL, en_frame->dataEncrypted, clen, 
                            assoc_data, strlen((char *)assoc_data), 
                            nonce, key) == 0) {
        if (decrypted_len < sizeof(decryptedtext)) {
            decryptedtext[decrypted_len] = '\0';
        }
        
        Serial.print("Decrypted text: ");
        for (unsigned long long i = 0; i < decrypted_len; i++) {
            Serial.print(decryptedtext[i], HEX);
        }
        Serial.println();
    } else {
        Serial.println("Decryption failed!");
    }
    return 0;
}
