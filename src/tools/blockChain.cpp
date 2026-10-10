// Blockchain demo：实现都在 mytools::Blockchain / mytools::Block。
//   g++ -std=c++20 -I include src/tools/blockChain.cpp \
//       $(pkg-config --cflags --libs --static mytools) -o bin/blockChain
#include <iostream>

#include "mytools/blockchain.hpp"

int main() {
    mytools::Blockchain chain;
    chain.addBlock(mytools::Block("Block 1 Data"));
    chain.addBlock(mytools::Block("Block 2 Data"));

    for (const auto& block : chain.chain()) {
        std::cout << "Previous Hash: " << block.previousHash << std::endl;
        std::cout << "Data: " << block.data << std::endl;
        std::cout << "Hash: " << block.hash << std::endl;
        std::cout << "Timestamp: " << block.timestamp << std::endl;
        std::cout << "------------------------" << std::endl;
    }
    return 0;
}
