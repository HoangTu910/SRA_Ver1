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

uint8_t transitionFrame(Frame_t frame, Encrypt_Frame_t *en_frame)
{
    // if (en_frame == NULL) {
    //     Serial.println("En_Frame NULL");
    //     return 1;  
    // }
    en_frame->h1 = frame.h1;
    en_frame->h2 = frame.h2;
    en_frame->crc = frame.crc;
    en_frame->t1 = frame.t1;
    en_frame->t2 = frame.t2;
    return 0;
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
    reconstructDecryptedData(decryptedtext);
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
        Serial.println("Construct");
        //reconstructDecryptedData(decryptedtext);
        } else {
            Serial.println("Decryption failed!");
        }
    return 0;
}

uint8_t reconstructDecryptedData(unsigned char *decryptedtext) {
    if (decryptedtext == NULL) {
        return 1; // Return error if the input pointer is NULL
    }

    // Print the deviceId (first 32 bytes)
    Serial.print("Device ID: ");
    for (int i = 0; i < 32; i++) {
        if (decryptedtext[i] == '\0') break;
        Serial.print((char)decryptedtext[i]);
    }
    Serial.println();

    // Extract and print the deviceLen (10 bits, starting from byte 32)
    uint16_t deviceLen = decryptedtext[32] | ((decryptedtext[33] & 0x03) << 8);
    Serial.print("Device Length: ");
    Serial.println(deviceLen);

    // Extract and print the dataLen (10 bits, overlapping with deviceLen)
    uint16_t dataLen = ((decryptedtext[33] & 0xFC) >> 2) | (decryptedtext[34] << 6);
    Serial.print("Data Length: ");
    Serial.println(dataLen);

    // Print the remaining data (next 64 bytes)
    if (dataLen >= 3) {  
        uint8_t heartRate = decryptedtext[36];
        uint8_t spO2 = decryptedtext[37];
        uint8_t temperature = decryptedtext[38];
        Serial.print("Heart Rate: ");
        Serial.println(heartRate);
        Serial.print("SpO2: ");
        Serial.println(spO2);
        Serial.print("Temperature: ");
        Serial.println(temperature);
    } else {
        Serial.println("Insufficient data length to extract health metrics.");
    }

    return 0; 
}

uint8_t *hexToBinary(const char *hex, size_t hexLen, size_t *binaryLen)
{
    if (hexLen % 2 != 0) {
        // Invalid hex string length
        return NULL;
    }

    *binaryLen = hexLen / 2;
    uint8_t *binary = (uint8_t *)malloc(*binaryLen);
    if (binary == NULL) {
        return NULL;
    }

    for (size_t i = 0, j = 0; i < hexLen; i += 2, j++) {
        char hexByte[3] = { hex[i], hex[i + 1], '\0' };
        binary[j] = (uint8_t)strtol(hexByte, NULL, 16);
    }

    return binary;
}
