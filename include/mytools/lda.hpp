#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace mytools {

// LDA 配置（对应旧的 ini 文件字段）
struct LdaConfig {
    int num_topics = 0;
    int num_iterations = 0;
    double alpha = 0.0;
    double beta = 0.0;
    std::string train_file;
    std::string doc_topic_file;
    std::string topic_word_file;
};

// 读取 "key = value" 形式的配置文件；非法时抛出 std::runtime_error。
LdaConfig readLdaConfig(const std::string& config_file);

// 读取训练语料：每行一个文档，空格分词。
std::vector<std::vector<std::string>> readDocuments(const std::string& filename);

// 吉布斯采样的 LDA 模型。
class Lda {
public:
    Lda(int num_topics, int num_iterations, double alpha, double beta);

    void fit(const std::vector<std::vector<std::string>>& documents);
    void printTopics() const;

    void saveDocTopics(const std::string& filename) const;
    void saveTopicWords(const std::string& filename) const;

    int numTopics() const { return num_topics_; }
    std::size_t vocabSize() const { return word2id_.size(); }

private:
    void initialize(const std::vector<std::vector<std::string>>& documents);
    int sample(const std::vector<double>& p);

    int num_topics_;
    int num_iterations_;
    double alpha_;
    double beta_;
    int vocab_size_ = 0;

    std::vector<std::vector<int>> topics_;
    std::vector<std::vector<int>> doc_topic_count_;
    std::vector<std::unordered_map<int, int>> topic_word_count_;
    std::vector<int> topic_count_;
    std::vector<int> doc_count_;

    std::unordered_map<std::string, int> word2id_;
    std::unordered_map<int, std::string> id2word_;

    std::mt19937 rng_;
};

}  // namespace mytools
