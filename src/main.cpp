#include "core.h"
#include "encrypt.h"
#include "decrypt.h"
#include "constants.h"
#include "mbedtls/aes.h"
#include "rfc7539.h"
#include "chacha20poly1305.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>

#define MAX_BLOCK_SIZE 128
#define NUM_ITERATIONS 1000
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

void benchmark_chacha20_poly1305() {
  const unsigned char plaintext[] = "Encrypt Data 9999";
  const unsigned char aad[] = {ENV_AAD};  // Associated Data
  const unsigned char key[] = {ENV_KEY};  // 256-bit key
  const unsigned char nonce[] = {ENV_NONCE}; // Nonce
  unsigned char ciphertext[MAX_BLOCK_SIZE];
  unsigned char tag[16];
  
  for (int block_size = 16; block_size <= MAX_BLOCK_SIZE; block_size *= 2) { // Test block sizes: 16, 32, 64, 128 bytes
      // Measure memory usage before
      uint32_t memory_before = esp_get_free_heap_size();
      
      // Measure clock cycles start
      uint64_t clock_cycles_start = esp_cpu_get_ccount();
      
      // Measure time start
      clock_t start = clock();

      for (int i = 0; i < NUM_ITERATIONS; i++) {
          encrypt_and_authenticate(plaintext, strlen((char *)plaintext), aad, sizeof(aad), ciphertext, tag, key, nonce);
      }

      // Measure clock cycles end
      uint64_t clock_cycles_end = esp_cpu_get_ccount();
      // Measure time end
      clock_t end = clock();

      // Calculate metrics
      uint64_t clock_cycles = clock_cycles_end - clock_cycles_start;
      double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

      // Throughput calculations
      double throughput = (block_size * NUM_ITERATIONS) / cpu_time_used;
      double cycles_per_byte = (double)clock_cycles / (block_size * NUM_ITERATIONS);
      double microseconds_per_byte = (cpu_time_used * 1e6) / (block_size * NUM_ITERATIONS);
      
      // Measure memory usage after
      uint32_t memory_after = esp_get_free_heap_size();

      Serial.println("Results for block size " + String(block_size) + " bytes:");
      Serial.println("Time taken for Chacha20-Poly1305: " + String(cpu_time_used) + " seconds");
      Serial.println("Clock cycles used: " + String(clock_cycles));
      Serial.println("Throughput: " + String(throughput) + " bytes/sec");
      Serial.println("Cycles per byte: " + String(cycles_per_byte));
      Serial.println("Microseconds per byte: " + String(microseconds_per_byte));
      Serial.println("Memory used: " + String(memory_before - memory_after) + " bytes");
      Serial.println();  // New line for better readability
  }
}

void res(){
  const unsigned char plaintext[] = "Encrypt Data 9999";
  const unsigned char aad[] = {ENV_AAD};  // Associated Data
  const unsigned char key[] = {ENV_KEY};  // 256-bit key
  const unsigned char nonce[] = {ENV_NONCE}; // Nonce
  unsigned char ciphertext[MAX_BLOCK_SIZE];
  unsigned char tag[16];
  
  for (int block_size = 16; block_size <= MAX_BLOCK_SIZE; block_size *= 2) { // Test block sizes: 16, 32, 64, 128 bytes
      // Measure memory usage before
      uint32_t memory_before = esp_get_free_heap_size();
      
      // Measure clock cycles start
      uint64_t clock_cycles_start = esp_cpu_get_ccount();
      
      // Measure time start
      clock_t start = clock();

      // Measure clock cycles end
      uint64_t clock_cycles_end = esp_cpu_get_ccount();
      // Measure time end
      clock_t end = clock();

      // Calculate metrics
      uint64_t clock_cycles = clock_cycles_end - clock_cycles_start;
      double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

      // Throughput calculations
      double throughput = (block_size * NUM_ITERATIONS) / cpu_time_used;
      double cycles_per_byte = (double)clock_cycles / (block_size * NUM_ITERATIONS);
      double microseconds_per_byte = (cpu_time_used * 1e6) / (block_size * NUM_ITERATIONS);
      
      // Measure memory usage after
      uint32_t memory_after = esp_get_free_heap_size();
  }
}

void benchmark_aes() {
  const unsigned char plaintext[] = "Encrypt Data 9999";
  unsigned char key[16] = ASCON_KEY; 
  unsigned char ciphertext[MAX_BLOCK_SIZE];
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 128);
  for (int block_size = 16; block_size <= MAX_BLOCK_SIZE; block_size *= 2) { // Test block sizes: 16, 32, 64, 128 bytes
      // Measure memory usage before
      uint32_t memory_before = esp_get_free_heap_size();
      
      // Measure clock cycles start
      uint64_t clock_cycles_start = esp_cpu_get_ccount();
      
      // Measure time start
      clock_t start = clock();

      for (int i = 0; i < NUM_ITERATIONS; i++) {
          mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plaintext, ciphertext);
      }

      // Measure clock cycles end
      uint64_t clock_cycles_end = esp_cpu_get_ccount();
      // Measure time end
      clock_t end = clock();

      // Calculate metrics
      uint64_t clock_cycles = clock_cycles_end - clock_cycles_start;
      double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

      // Throughput calculations
      double throughput = (block_size * NUM_ITERATIONS) / cpu_time_used;
      double cycles_per_byte = (double)clock_cycles / (block_size * NUM_ITERATIONS);
      double microseconds_per_byte = (cpu_time_used * 1e6) / (block_size * NUM_ITERATIONS);
      
      // Measure memory usage after
      uint32_t memory_after = esp_get_free_heap_size();

      Serial.println("Results for block size " + String(block_size) + " bytes:");
      Serial.println("Time taken for AES: " + String(cpu_time_used) + " seconds");
      Serial.println("Clock cycles used: " + String(clock_cycles));
      Serial.println("Throughput: " + String(throughput) + " bytes/sec");
      Serial.println("Cycles per byte: " + String(cycles_per_byte));
      Serial.println("Microseconds per byte: " + String(microseconds_per_byte));
      Serial.println("Memory used: " + String(memory_before - memory_after) + " bytes");
      Serial.println();  // New line for better readability
  }
}

void benchmark_ascon() {
  const unsigned char plaintext[] = "Encrypt Data 9999";
  const unsigned char key[16] = ASCON_KEY;  // 128-bit key
  unsigned char nonce[16] = {
      0x1A, 0x2B, 0x3C, 0x4D,
      0x5E, 0x6F, 0x70, 0x81,
      0x92, 0xA3, 0xB4, 0xC5,
      0xD6, 0xE7, 0xF8, 0x09
  };
  const unsigned char assoc_data[] = {ENV_AAD}; 
  unsigned char ciphertext[256];
  unsigned long long clen = 0;

  for (int block_size = 16; block_size <= MAX_BLOCK_SIZE; block_size *= 2) { // Test block sizes: 16, 32, 64, 128 bytes
      // Measure memory usage before
      uint32_t memory_before = esp_get_free_heap_size();
      
      // Measure clock cycles start
      uint64_t clock_cycles_start = esp_cpu_get_ccount();
      
      // Measure time start
      clock_t start = clock();

      for (int i = 0; i < NUM_ITERATIONS; i++) {
          crypto_aead_encrypt(ciphertext, &clen, plaintext, block_size, 
                              assoc_data, strlen((char *)assoc_data), 
                              NULL, nonce, key);
      }

      // Measure clock cycles end
      uint64_t clock_cycles_end = esp_cpu_get_ccount();
      // Measure time end
      clock_t end = clock();

      // Calculate metrics
      uint64_t clock_cycles = clock_cycles_end - clock_cycles_start;
      double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

      // Throughput calculations
      double throughput = (block_size * NUM_ITERATIONS) / cpu_time_used;
      double cycles_per_byte = (double)clock_cycles / (block_size * NUM_ITERATIONS);
      double microseconds_per_byte = (cpu_time_used * 1e6) / (block_size * NUM_ITERATIONS);
      
      // Measure memory usage after
      uint32_t memory_after = esp_get_free_heap_size();

      Serial.println("Results for block size " + String(block_size) + " bytes:");
      Serial.println("Time taken for Ascon-128a: " + String(cpu_time_used) + " seconds");
      Serial.println("Clock cycles used: " + String(clock_cycles));
      Serial.println("Throughput: " + String(throughput) + " bytes/sec");
      Serial.println("Cycles per byte: " + String(cycles_per_byte));
      Serial.println("Microseconds per byte: " + String(microseconds_per_byte));
      Serial.println("Memory used: " + String(memory_before - memory_after) + " bytes");
      Serial.println();  // New line for better readability
  }
}

void setup() {
    Serial.begin(9600);  
    res();
    benchmark_chacha20_poly1305();
    Serial.println("---------------------------------------------");
    benchmark_aes();
    Serial.println("---------------------------------------------");
    benchmark_ascon();
}

void loop() {
  
}

