// 演示：只依赖 dist/（include + lib），不碰 conan/cmake。
//   source dist/env.sh
//   g++ -std=c++20 examples/use_mytools.cpp \
//       $(pkg-config --cflags --libs --static mytools) -o use_mytools
#include <iostream>

#include "mytools/blockchain.hpp"
#include "mytools/crypto.hpp"
#include "mytools/lru.hpp"

int main() {
    mytools::LruCache<std::string, int> cache(2);
    cache.put("a", 1);
    cache.put("b", 2);
    cache.put("c", 3);
    int v = 0;
    std::cout << "get(a) -> " << cache.get("a", v) << " (expect 0, evicted)\n";
    std::cout << "get(c) -> " << cache.get("c", v) << " val=" << v << "\n";

    std::cout << "md5     = " << mytools::CryptoToolbox::MD5Hash("Hello, Hash!") << "\n";
    std::cout << "sha256  = " << mytools::CryptoToolbox::SHA256Hash("Hello, Hash!") << "\n";
    std::cout << "base64  = " << mytools::CryptoToolbox::Base64Encode("Hello, Base64!") << "\n";

    mytools::Blockchain chain;
    chain.addBlock(mytools::Block("Block 1 Data"));
    std::cout << "chain size = " << chain.size() << " latest=" << chain.getLatestBlock().data << "\n";
    return 0;
}
