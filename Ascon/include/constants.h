#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdint.h>

#define ASCON_128_KEYBYTES 16
#define ASCON_128A_KEYBYTES 16
#define ASCON_80PQ_KEYBYTES 20

#define ASCON_NONCE_SIZE 16

#define ASCON_KEY { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, \
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }
#define ASCON_NONCE = random()

#define ASCON_128_RATE 8
#define ASCON_128A_RATE 16
#define ASCON_HASH_RATE 8
#define ASCON_PRF_IN_RATE 32
#define ASCON_PRFA_IN_RATE 40
#define ASCON_PRF_OUT_RATE 16

#define ASCON_128_PA_ROUNDS 12
#define ASCON_128_PB_ROUNDS 6
#define ASCON_128A_PA_ROUNDS 12
#define ASCON_128A_PB_ROUNDS 8

#define ASCON_HASH_PA_ROUNDS 12
#define ASCON_HASH_PB_ROUNDS 12
#define ASCON_HASHA_PA_ROUNDS 12
#define ASCON_HASHA_PB_ROUNDS 8

#define ASCON_PRF_PA_ROUNDS 12
#define ASCON_PRF_PB_ROUNDS 12
#define ASCON_PRFA_PA_ROUNDS 12
#define ASCON_PRFA_PB_ROUNDS 8

#define ASCON_128_IV 0x80400c0600000000ull
#define ASCON_128A_IV 0x80800c0800000000ull
#define ASCON_80PQ_IV 0xa0400c0600000000ull

#define ASCON_HASH_IV 0x00400c0000000100ull
#define ASCON_HASHA_IV 0x00400c0400000100ull
#define ASCON_XOF_IV 0x00400c0000000000ull
#define ASCON_XOFA_IV 0x00400c0400000000ull

#define ASCON_HASH_IV0 0xee9398aadb67f03dull
#define ASCON_HASH_IV1 0x8bb21831c60f1002ull
#define ASCON_HASH_IV2 0xb48a92db98d5da62ull
#define ASCON_HASH_IV3 0x43189921b8f8e3e8ull
#define ASCON_HASH_IV4 0x348fa5c9d525e140ull

#define ASCON_HASHA_IV0 0x01470194fc6528a6ull
#define ASCON_HASHA_IV1 0x738ec38ac0adffa7ull
#define ASCON_HASHA_IV2 0x2ec8e3296c76384cull
#define ASCON_HASHA_IV3 0xd6f6a54d7f52377dull
#define ASCON_HASHA_IV4 0xa13c42a223be8d87ull

#define ASCON_XOF_IV0 0xb57e273b814cd416ull
#define ASCON_XOF_IV1 0x2b51042562ae2420ull
#define ASCON_XOF_IV2 0x66a3a7768ddf2218ull
#define ASCON_XOF_IV3 0x5aad0a7a8153650cull
#define ASCON_XOF_IV4 0x4f3e0e32539493b6ull

#define ASCON_XOFA_IV0 0x44906568b77b9832ull
#define ASCON_XOFA_IV1 0xcd8d6cae53455532ull
#define ASCON_XOFA_IV2 0xf7b5212756422129ull
#define ASCON_XOFA_IV3 0x246885e1de0d225bull
#define ASCON_XOFA_IV4 0xa8cb5ce33449973full

#define ASCON_MAC_IV 0x80808c0000000080ull
#define ASCON_MACA_IV 0x80808c0400000080ull
#define ASCON_PRF_IV 0x80808c0000000000ull
#define ASCON_PRFA_IV 0x80808c0400000000ull
#define ASCON_PRFS_IV 0x80004c8000000000ull

#define RC0 0xf0
#define RC1 0xe1
#define RC2 0xd2
#define RC3 0xc3
#define RC4 0xb4
#define RC5 0xa5
#define RC6 0x96
#define RC7 0x87
#define RC8 0x78
#define RC9 0x69
#define RCa 0x5a
#define RCb 0x4b

#define RC(i) (i)

#define START(n) ((3 + (n)) << 4 | (12 - (n)))
#define INC -0x0f
#define END 0x3c

#endif /* CONSTANTS_H_ */