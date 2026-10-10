// LDA demo：逻辑都在 mytools::Lda，这里只做参数解析和流程编排。
// 构建：见 scripts/build_lib.sh 生成 dist/lib/libmytools.a 后
//   g++ -std=c++17 -I include src/tools/LDA.cpp -L dist/lib -lmytools -o bin/LDA
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "mytools/lda.hpp"

namespace {
std::string parseArgs(int argc, char* argv[]) {
    std::string config_path;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-c" && i + 1 < argc) config_path = argv[++i];
    }
    return config_path;
}
}  // namespace

int main(int argc, char* argv[]) {
    std::string config_path = parseArgs(argc, argv);
    if (config_path.empty()) {
        std::cout << "Usage: LDA -c <config_file_path>" << std::endl;
        return 1;
    }

    try {
        mytools::LdaConfig config = mytools::readLdaConfig(config_path);
        auto documents = mytools::readDocuments(config.train_file);
        if (documents.empty()) {
            std::cerr << "Error: No documents loaded" << std::endl;
            return 1;
        }

        mytools::Lda lda(config.num_topics, config.num_iterations, config.alpha, config.beta);
        lda.fit(documents);
        lda.printTopics();

        if (!config.doc_topic_file.empty()) lda.saveDocTopics(config.doc_topic_file);
        if (!config.topic_word_file.empty()) lda.saveTopicWords(config.topic_word_file);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
