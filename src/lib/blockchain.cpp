#include "mytools/blockchain.hpp"

#include <openssl/sha.h>

#include <iomanip>
#include <sstream>

namespace mytools {

Block::Block(std::string data, std::string previousHash)
    : previousHash(std::move(previousHash)), data(std::move(data)), timestamp(std::time(0)) {
    hash = calculateHash();
}

std::string Block::calculateHash() const {
    std::stringstream ss;
    ss << previousHash << timestamp << data;
    std::string input = ss.str();

    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), digest);

    std::stringstream hashString;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hashString << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return hashString.str();
}

}  // namespace mytools
