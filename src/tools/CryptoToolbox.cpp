// CryptoToolbox demo：实现都在 mytools::CryptoToolbox。
//   g++ -std=c++20 -I include src/tools/CryptoToolbox.cpp \
//       $(pkg-config --cflags --libs --static mytools) -o bin/CryptoToolbox
#include <iostream>
#include <string>
#include <vector>

#include "mytools/crypto.hpp"

using mytools::CryptoToolbox;

int main() {
    std::string aesKey = "thisisaverysecret";
    std::string plainText = "Hello, World!";

    std::string cipherText = CryptoToolbox::AESEncrypt(plainText, aesKey);
    std::string decryptedText = CryptoToolbox::AESDecrypt(cipherText, aesKey);
    std::cout << "AES Encrypted: " << cipherText << std::endl;
    std::cout << "AES Decrypted: " << decryptedText << std::endl;

    std::string publicKey, privateKey;
    CryptoToolbox::GenerateRSAKeyPair(publicKey, privateKey);
    cipherText = CryptoToolbox::RSAEncrypt(plainText, publicKey);
    decryptedText = CryptoToolbox::RSADecrypt(cipherText, privateKey);
    std::cout << "RSA Encrypted: " << cipherText << std::endl;
    std::cout << "RSA Decrypted: " << decryptedText << std::endl;

    std::string desKey = "deskey12";
    std::string desPlainText = "Hello, DES!";
    std::string desCipherText = CryptoToolbox::DESEncrypt(desPlainText, desKey);
    std::string desDecryptedText = CryptoToolbox::DESDecrypt(desCipherText, desKey);
    std::cout << "DES Encrypted: " << desCipherText << std::endl;
    std::cout << "DES Decrypted: " << desDecryptedText << std::endl;

    std::string streamKey = "streamkey";
    std::string streamPlainText = "Hello, Stream!";
    std::string streamCipherText = CryptoToolbox::StreamEncrypt(streamPlainText, streamKey);
    std::string streamDecryptedText = CryptoToolbox::StreamDecrypt(streamCipherText, streamKey);
    std::cout << "Stream Encrypted: " << streamCipherText << std::endl;
    std::cout << "Stream Decrypted: " << streamDecryptedText << std::endl;

    std::string caesarPlainText = "Hello, Caesar!";
    int shift = 3;
    std::string caesarCipherText = CryptoToolbox::CaesarEncrypt(caesarPlainText, shift);
    std::string caesarDecryptedText = CryptoToolbox::CaesarDecrypt(caesarCipherText, shift);
    std::cout << "Caesar Encrypted: " << caesarCipherText << std::endl;
    std::cout << "Caesar Decrypted: " << caesarDecryptedText << std::endl;

    std::string hashInput = "Hello, Hash!";
    std::cout << "MD5 Hash: " << CryptoToolbox::MD5Hash(hashInput) << std::endl;
    std::cout << "SHA1 Hash: " << CryptoToolbox::SHA1Hash(hashInput) << std::endl;
    std::cout << "SHA256 Hash: " << CryptoToolbox::SHA256Hash(hashInput) << std::endl;

    std::vector<std::vector<int>> keyMatrix = {{6, 24, 1}, {13, 16, 10}, {20, 17, 15}};
    std::vector<std::vector<int>> inverseKeyMatrix = {{8, 5, 10}, {21, 8, 21}, {21, 12, 8}};
    std::string hillPlainText = "HELLOHILL";
    std::string hillCipherText = CryptoToolbox::HillEncrypt(hillPlainText, keyMatrix);
    std::string hillDecryptedText = CryptoToolbox::HillDecrypt(hillCipherText, inverseKeyMatrix);
    std::cout << "Hill Encrypted: " << hillCipherText << std::endl;
    std::cout << "Hill Decrypted: " << hillDecryptedText << std::endl;

    std::string base64Input = "Hello, Base64!";
    std::string base64Encoded = CryptoToolbox::Base64Encode(base64Input);
    std::string base64Decoded = CryptoToolbox::Base64Decode(base64Encoded);
    std::cout << "Base64 Encoded: " << base64Encoded << std::endl;
    std::cout << "Base64 Decoded: " << base64Decoded << std::endl;

    return 0;
}
