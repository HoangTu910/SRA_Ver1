#include "decrypt.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <cstddef>

std::vector<unsigned char> hexStringToByteArray(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  if (clen < CRYPTO_ABYTES) {
    *mlen = 0;
    return -1;
  }

  state s;
  u32_4 tmp;
  (void)nsec;

  // set plaintext size
  *mlen = clen - CRYPTO_ABYTES;

  ascon_core(&s, m, c, *mlen, ad, adlen, npub, k, ASCON_DEC);

  tmp.words[0].l = ((u32*)(c + *mlen))[0];
  tmp.words[0].h = ((u32*)(c + *mlen))[1];
  tmp.words[1].l = ((u32*)(c + *mlen))[2];
  tmp.words[1].h = ((u32*)(c + *mlen))[3];
  tmp = ascon_rev8(tmp);
  u32_2 t0 = tmp.words[0];
  u32_2 t1 = tmp.words[1];

  // verify should be constant time, check compiler output
  if (((s.x3.h ^ t0.h) | (s.x3.l ^ t0.l) | (s.x4.h ^ t1.h) | (s.x4.l ^ t1.l)) !=
      0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <encryptedDataHex> <nonceHex> <keyHex>" << std::endl;
        return 1;
    }

    // Convert hex strings to byte arrays
    std::vector<unsigned char> encryptedData = hexStringToByteArray(argv[1]);
    std::vector<unsigned char> nonce = hexStringToByteArray(argv[2]);
    std::vector<unsigned char> key = hexStringToByteArray(argv[3]);
    const unsigned char assoc_data[] = "Metadata";
    unsigned long long mlen = 0;
    unsigned char decrypted[1024]; // Adjust the size as needed
    int result = crypto_aead_decrypt(decrypted, &mlen, nullptr, encryptedData.data(), encryptedData.size(), assoc_data, strlen((char *)assoc_data), nonce.data(), key.data());

    // if (result == 0) {
    //     std::cout << "Decrypted data: ";
    //     for (unsigned long long i = 0; i < mlen; ++i) {
    //         std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)decrypted[i];
    //     }
    //     std::cout << std::endl;
    // } else {
    //     std::cerr << "Decryption failed!" << std::endl;
    //     return 1;
    // }

    if (result == 0) {
        std::cout << "Decryption completed check: " << mlen << std::endl;
        for (unsigned long long i = 0; i < mlen; ++i) {
            std::cout << std::hex << static_cast<int>(decrypted[i]) << " ";
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Decryption failed" << std::endl;
    }

    return 0;
}