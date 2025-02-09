#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>
#include "asconDecrypt.hpp"

std::vector<unsigned char> hexToBytes(const std::string &hex)
{
    std::vector<unsigned char> bytes;
    if (hex.length() % 2 != 0)
    {
        std::cerr << "Invalid hex string (must have even length)." << std::endl;
        exit(1);
    }
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

int main(int argc, char *argv[])
{
    // Expecting 3 command-line arguments:
    // 1. ciphertext (hex)
    // 2. nonce (hex)
    // 3. key (hex)
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <ciphertextHex> <nonceHex> <keyHex>" << std::endl;
        return 1;
    }

    std::string ciphertextHex(argv[1]);
    std::string nonceHex(argv[2]);
    std::string keyHex(argv[3]);

    // Convert hex strings to byte vectors.
    std::vector<unsigned char> ciphertext = hexToBytes(ciphertextHex);
    std::vector<unsigned char> nonce = hexToBytes(nonceHex);
    std::vector<unsigned char> key = hexToBytes(keyHex);

    std::vector<unsigned char> plaintext(ciphertext.size());
    unsigned long long plaintextLen = 0;

    // Allocate nsec buffer (if required; often unused).
    unsigned char nsec[16] = {0};

    // Call the decryption function.
    const unsigned char *associatedData = (const unsigned char *)"DeslabAIoT0x910";

    int ret = Ascon::crypto_aead_decrypt(
        plaintext.data(),  // m: pointer to plaintext output
        &plaintextLen,     // mlen: pointer to plaintext length
        nullptr,           // nsec: optional secret data (often unused)
        ciphertext.data(), // c: ciphertext
        ciphertext.size(), // clen: ciphertext length
        associatedData,    // ad: associated data pointer (not needed here)
        16,                // adlen: associated data length (0)
        nonce.data(),      // npub: nonce
        key.data()         // k: key
    );

    if (ret != 0)
    {
        std::cerr << "Decryption failed" << std::endl;
        return 1;
    }

    // Convert plaintext to a hex string for output.
    std::ostringstream oss;
    for (unsigned long long i = 0; i < plaintextLen; i++)
    {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)plaintext[i];
    }

    std::cout << oss.str() << std::endl;
    return 0;
}
