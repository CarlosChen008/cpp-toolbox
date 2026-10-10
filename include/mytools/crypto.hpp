#pragma once

#include <string>
#include <vector>

namespace mytools {

// 常用加解密 / 哈希 / 编码工具聚合。实现依赖 OpenSSL（crypto 库）。
class CryptoToolbox {
public:
    // AES（ECB 单块，128bit key）
    static std::string AESEncrypt(const std::string& plainText, const std::string& key);
    static std::string AESDecrypt(const std::string& cipherText, const std::string& key);

    // DES（ECB）
    static std::string DESEncrypt(const std::string& plainText, const std::string& key);
    static std::string DESDecrypt(const std::string& cipherText, const std::string& key);

    // RSA（PEM），密钥由 GenerateRSAKeyPair 生成
    static void GenerateRSAKeyPair(std::string& publicKey, std::string& privateKey);
    static std::string RSAEncrypt(const std::string& plainText, const std::string& publicKey);
    static std::string RSADecrypt(const std::string& cipherText, const std::string& privateKey);

    // 异或流密码
    static std::string StreamEncrypt(const std::string& plainText, const std::string& key);
    static std::string StreamDecrypt(const std::string& cipherText, const std::string& key);

    // 凯撒密码
    static std::string CaesarEncrypt(const std::string& plainText, int shift);
    static std::string CaesarDecrypt(const std::string& cipherText, int shift);

    // 哈希（返回十六进制字符串）
    static std::string MD5Hash(const std::string& input);
    static std::string SHA1Hash(const std::string& input);
    static std::string SHA256Hash(const std::string& input);

    // 希尔密码
    static std::string HillEncrypt(const std::string& plainText,
                                   const std::vector<std::vector<int>>& keyMatrix);
    static std::string HillDecrypt(const std::string& cipherText,
                                   const std::vector<std::vector<int>>& inverseKeyMatrix);

    // Base64
    static std::string Base64Encode(const std::string& input);
    static std::string Base64Decode(const std::string& input);
};

}  // namespace mytools
