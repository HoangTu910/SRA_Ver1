#include "decrypt.h"
#include <cstddef>

extern "C" int decrypt_data(const unsigned char* ciphertext, unsigned long long ciphertext_len,
                            const unsigned char* nonce, const unsigned char* key, 
                            unsigned char* decrypted) {
    unsigned long long decrypted_len;
    return crypto_aead_decrypt(decrypted, &decrypted_len, NULL, ciphertext, ciphertext_len, NULL, 0, nonce, key);
}
