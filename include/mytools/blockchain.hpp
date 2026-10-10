#pragma once

#include <cstddef>
#include <ctime>
#include <string>
#include <vector>

namespace mytools {

struct Block {
    std::string previousHash;
    std::string hash;
    std::string data;
    std::time_t timestamp;

    explicit Block(std::string data, std::string previousHash = "");
    std::string calculateHash() const;
};

class Blockchain {
public:
    Blockchain() { chain_.push_back(createGenesisBlock()); }

    void addBlock(Block newBlock) {
        newBlock.previousHash = getLatestBlock().hash;
        newBlock.hash = newBlock.calculateHash();
        chain_.push_back(newBlock);
    }

    const Block& getLatestBlock() const { return chain_.back(); }
    const std::vector<Block>& chain() const { return chain_; }
    std::size_t size() const { return chain_.size(); }

private:
    std::vector<Block> chain_;
    Block createGenesisBlock() const { return Block("Genesis Block"); }
};

}  // namespace mytools
