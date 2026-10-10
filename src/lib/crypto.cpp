#include "mytools/crypto.hpp"

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace mytools {

std::string CryptoToolbox::AESEncrypt(const std::string& plainText, const std::string& key) {
    AES_KEY encryptKey;
    AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &encryptKey);
    std::string cipherText(plainText.size(), '\0');
    AES_encrypt(reinterpret_cast<const unsigned char*>(plainText.c_str()),
                reinterpret_cast<unsigned char*>(&cipherText[0]), &encryptKey);
    return cipherText;
}

std::string CryptoToolbox::AESDecrypt(const std::string& cipherText, const std::string& key) {
    AES_KEY decryptKey;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &decryptKey);
    std::string plainText(cipherText.size(), '\0');
    AES_decrypt(reinterpret_cast<const unsigned char*>(cipherText.c_str()),
                reinterpret_cast<unsigned char*>(&plainText[0]), &decryptKey);
    return plainText;
}

std::string CryptoToolbox::DESEncrypt(const std::string& plainText, const std::string& key) {
    DES_cblock keyBlock;
    DES_key_schedule schedule;
    std::memset(keyBlock, 0, sizeof(keyBlock));
    std::memcpy(keyBlock, key.c_str(), key.size() > 8 ? 8 : key.size());
    DES_set_key_unchecked(&keyBlock, &schedule);

    std::string cipherText(plainText.size(), '\0');
    DES_cblock inputText;
    DES_cblock outputText;
    for (size_t i = 0; i < plainText.size(); i += 8) {
        std::memset(inputText, 0, sizeof(inputText));
        std::memcpy(inputText, plainText.c_str() + i,
                    (plainText.size() - i) < 8 ? (plainText.size() - i) : 8);
        DES_ecb_encrypt(&inputText, &outputText, &schedule, DES_ENCRYPT);
        std::memcpy(&cipherText[i], outputText, (plainText.size() - i) < 8 ? (plainText.size() - i) : 8);
    }
    return cipherText;
}

std::string CryptoToolbox::DESDecrypt(const std::string& cipherText, const std::string& key) {
    DES_cblock keyBlock;
    DES_key_schedule schedule;
    std::memset(keyBlock, 0, sizeof(keyBlock));
    std::memcpy(keyBlock, key.c_str(), key.size() > 8 ? 8 : key.size());
    DES_set_key_unchecked(&keyBlock, &schedule);

    std::string plainText(cipherText.size(), '\0');
    DES_cblock inputText;
    DES_cblock outputText;
    for (size_t i = 0; i < cipherText.size(); i += 8) {
        std::memset(inputText, 0, sizeof(inputText));
        std::memcpy(inputText, cipherText.c_str() + i,
                    (cipherText.size() - i) < 8 ? (cipherText.size() - i) : 8);
        DES_ecb_encrypt(&inputText, &outputText, &schedule, DES_DECRYPT);
        std::memcpy(&plainText[i], outputText, (cipherText.size() - i) < 8 ? (cipherText.size() - i) : 8);
    }
    return plainText;
}

void CryptoToolbox::GenerateRSAKeyPair(std::string& publicKey, std::string& privateKey) {
    RSA* rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    BIO* pub = BIO_new(BIO_s_mem());
    BIO* pri = BIO_new(BIO_s_mem());

    PEM_write_bio_RSAPublicKey(pub, rsa);
    PEM_write_bio_RSAPrivateKey(pri, rsa, nullptr, nullptr, 0, nullptr, nullptr);

    size_t pubLen = BIO_pending(pub);
    size_t priLen = BIO_pending(pri);

    publicKey.resize(pubLen);
    privateKey.resize(priLen);

    BIO_read(pub, &publicKey[0], pubLen);
    BIO_read(pri, &privateKey[0], priLen);

    BIO_free_all(pub);
    BIO_free_all(pri);
    RSA_free(rsa);
}

std::string CryptoToolbox::RSAEncrypt(const std::string& plainText, const std::string& publicKey) {
    RSA* rsa = RSA_new();
    BIO* keybio = BIO_new_mem_buf(publicKey.c_str(), -1);
    PEM_read_bio_RSAPublicKey(keybio, &rsa, nullptr, nullptr);

    std::string cipherText(RSA_size(rsa), '\0');
    RSA_public_encrypt(plainText.size(),
                       reinterpret_cast<const unsigned char*>(plainText.c_str()),
                       reinterpret_cast<unsigned char*>(&cipherText[0]), rsa, RSA_PKCS1_PADDING);

    BIO_free_all(keybio);
    RSA_free(rsa);
    return cipherText;
}

std::string CryptoToolbox::RSADecrypt(const std::string& cipherText, const std::string& privateKey) {
    RSA* rsa = RSA_new();
    BIO* keybio = BIO_new_mem_buf(privateKey.c_str(), -1);
    PEM_read_bio_RSAPrivateKey(keybio, &rsa, nullptr, nullptr);

    std::string plainText(RSA_size(rsa), '\0');
    int n = RSA_private_decrypt(cipherText.size(),
                                reinterpret_cast<const unsigned char*>(cipherText.c_str()),
                                reinterpret_cast<unsigned char*>(&plainText[0]), rsa,
                                RSA_PKCS1_PADDING);
    if (n >= 0) plainText.resize(n);

    BIO_free_all(keybio);
    RSA_free(rsa);
    return plainText;
}

std::string CryptoToolbox::StreamEncrypt(const std::string& plainText, const std::string& key) {
    std::string cipherText(plainText.size(), '\0');
    for (size_t i = 0; i < plainText.size(); ++i) {
        cipherText[i] = plainText[i] ^ key[i % key.size()];
    }
    return cipherText;
}

std::string CryptoToolbox::StreamDecrypt(const std::string& cipherText, const std::string& key) {
    std::string plainText(cipherText.size(), '\0');
    for (size_t i = 0; i < cipherText.size(); ++i) {
        plainText[i] = cipherText[i] ^ key[i % key.size()];
    }
    return plainText;
}

std::string CryptoToolbox::CaesarEncrypt(const std::string& plainText, int shift) {
    std::string cipherText = plainText;
    for (char& c : cipherText) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            char base = std::islower(static_cast<unsigned char>(c)) ? 'a' : 'A';
            c = static_cast<char>((c - base + shift) % 26 + base);
        }
    }
    return cipherText;
}

std::string CryptoToolbox::CaesarDecrypt(const std::string& cipherText, int shift) {
    return CaesarEncrypt(cipherText, 26 - shift);
}

std::string CryptoToolbox::MD5Hash(const std::string& input) {
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::string output;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x", hash[i]);
        output.append(buf);
    }
    return output;
}

std::string CryptoToolbox::SHA1Hash(const std::string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::string output;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x", hash[i]);
        output.append(buf);
    }
    return output;
}

std::string CryptoToolbox::SHA256Hash(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::string output;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x", hash[i]);
        output.append(buf);
    }
    return output;
}

std::string CryptoToolbox::HillEncrypt(const std::string& plainText,
                                       const std::vector<std::vector<int>>& keyMatrix) {
    int n = static_cast<int>(keyMatrix.size());
    std::string cipherText;
    for (size_t i = 0; i < plainText.size(); i += n) {
        std::vector<int> block(n, 0);
        for (int j = 0; j < n; ++j) {
            if (i + j < plainText.size()) block[j] = plainText[i + j] - 'A';
        }
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) sum += keyMatrix[j][k] * block[k];
            cipherText += static_cast<char>((sum % 26) + 'A');
        }
    }
    return cipherText;
}

std::string CryptoToolbox::HillDecrypt(const std::string& cipherText,
                                       const std::vector<std::vector<int>>& inverseKeyMatrix) {
    int n = static_cast<int>(inverseKeyMatrix.size());
    std::string plainText;
    for (size_t i = 0; i < cipherText.size(); i += n) {
        std::vector<int> block(n, 0);
        for (int j = 0; j < n; ++j) {
            if (i + j < cipherText.size()) block[j] = cipherText[i + j] - 'A';
        }
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) sum += inverseKeyMatrix[j][k] * block[k];
            plainText += static_cast<char>((sum % 26) + 'A');
        }
    }
    return plainText;
}

std::string CryptoToolbox::Base64Encode(const std::string& input) {
    BIO *bio, *b64;
    BUF_MEM* bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input.c_str(), input.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return encodedData;
}

std::string CryptoToolbox::Base64Decode(const std::string& input) {
    BIO *bio, *b64;
    char* buffer = static_cast<char*>(std::malloc(input.size()));
    std::memset(buffer, 0, input.size());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input.c_str(), input.size());
    bio = BIO_push(b64, bio);
    int decodedLength = BIO_read(bio, buffer, input.size());
    std::string decodedData(buffer, decodedLength > 0 ? decodedLength : 0);
    BIO_free_all(bio);
    std::free(buffer);
    return decodedData;
}

}  // namespace mytools
