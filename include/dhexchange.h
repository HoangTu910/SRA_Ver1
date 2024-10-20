#ifndef DIFFIE_HELLMAN_EXCHANGE_H
#define DIFFIE_HELLMAN_EXCHANGE_H

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>

#define DH_KEY_LENGTH	(16)

typedef unsigned char DH_KEY[DH_KEY_LENGTH];

/*--------------------------------------------------------------------------*/
typedef union _uint128_t {
	struct {
		uint64_t low;
		uint64_t high;
	};
	unsigned char byte[DH_KEY_LENGTH];
} uint128_t;

/* P =  2^128-159 = 0xffffffffffffffffffffffffffffff61 (The biggest 64bit prime) */
static const uint128_t Prime = { 0xffffffffffffff61ULL, 0xffffffffffffffffULL };
static const uint128_t INVERT_P = { 159 };
static const uint128_t G = { 5 };

void DH_generate_key_pair(DH_KEY public_key, DH_KEY private_key);
void DH_generate_key_secret(DH_KEY secret_key, const DH_KEY my_private, const DH_KEY another_public);

#endif