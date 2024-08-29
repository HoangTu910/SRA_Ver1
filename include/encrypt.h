#ifndef ENCRYPT_H_
#include "core.h"
#include "constants.h"
#include <stdlib.h>
#include <time.h>

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k);
void generate_nonce(unsigned char nonce[ASCON_128_KEYBYTES]);
#endif