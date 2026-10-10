#include "mytools/lda.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace mytools {

namespace {
std::string trim(const std::string& s) {
    const auto begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}
}  // namespace

LdaConfig readLdaConfig(const std::string& config_file) {
    LdaConfig config;
    std::ifstream file(config_file);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + config_file);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == '#') continue;
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        const std::string key = trim(line.substr(0, pos));
        const std::string value = trim(line.substr(pos + 1));

        try {
            if (key == "num_topics") config.num_topics = std::stoi(value);
            else if (key == "num_iterations") config.num_iterations = std::stoi(value);
            else if (key == "alpha") config.alpha = std::stod(value);
            else if (key == "beta") config.beta = std::stod(value);
            else if (key == "train_file") config.train_file = value;
            else if (key == "doc_topic_file") config.doc_topic_file = value;
            else if (key == "topic_word_file") config.topic_word_file = value;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error parsing key '" + key + "': " + e.what());
        }
    }

    if (config.num_topics <= 0 || config.num_iterations <= 0 || config.alpha <= 0 ||
        config.beta <= 0 || config.train_file.empty()) {
        throw std::runtime_error("Invalid LDA configuration values");
    }
    return config;
}

std::vector<std::vector<std::string>> readDocuments(const std::string& filename) {
    std::vector<std::vector<std::string>> documents;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open training file: " + filename);
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> doc;
        std::string word;
        while (iss >> word) doc.push_back(word);
        documents.push_back(std::move(doc));
    }
    return documents;
}

Lda::Lda(int num_topics, int num_iterations, double alpha, double beta)
    : num_topics_(num_topics),
      num_iterations_(num_iterations),
      alpha_(alpha),
      beta_(beta),
      rng_(std::random_device{}()) {}

void Lda::initialize(const std::vector<std::vector<std::string>>& documents) {
    int word_id = 0;
    for (const auto& doc : documents) {
        for (const auto& word : doc) {
            if (word2id_.find(word) == word2id_.end()) {
                word2id_[word] = word_id;
                id2word_[word_id] = word;
                ++word_id;
            }
        }
    }
    vocab_size_ = word_id;

    topics_.resize(documents.size());
    doc_topic_count_.assign(documents.size(), std::vector<int>(num_topics_, 0));
    topic_word_count_.resize(num_topics_);
    topic_count_.assign(num_topics_, 0);
    doc_count_.resize(documents.size(), 0);

    std::uniform_int_distribution<int> dist(0, num_topics_ - 1);
    for (std::size_t d = 0; d < documents.size(); ++d) {
        topics_[d].resize(documents[d].size());
        doc_count_[d] = static_cast<int>(documents[d].size());
        for (std::size_t n = 0; n < documents[d].size(); ++n) {
            int topic = dist(rng_);
            topics_[d][n] = topic;
            doc_topic_count_[d][topic]++;
            topic_word_count_[topic][word2id_[documents[d][n]]]++;
            topic_count_[topic]++;
        }
    }
}

int Lda::sample(const std::vector<double>& p) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double rnd = dist(rng_);
    double sum = 0.0;
    for (int i = 0; i < num_topics_; ++i) {
        sum += p[i];
        if (rnd < sum) return i;
    }
    return num_topics_ - 1;
}

void Lda::fit(const std::vector<std::vector<std::string>>& documents) {
    initialize(documents);
    for (int iter = 0; iter < num_iterations_; ++iter) {
        int topic_changes = 0;
        for (std::size_t d = 0; d < documents.size(); ++d) {
            for (std::size_t n = 0; n < documents[d].size(); ++n) {
                int old_topic = topics_[d][n];
                int word_id = word2id_[documents[d][n]];

                doc_topic_count_[d][old_topic]--;
                topic_word_count_[old_topic][word_id]--;
                topic_count_[old_topic]--;
                doc_count_[d]--;

                std::vector<double> p(num_topics_);
                double sum_p = 0.0;
                for (int k = 0; k < num_topics_; ++k) {
                    p[k] = ((doc_topic_count_[d][k] + alpha_) /
                            (doc_count_[d] + num_topics_ * alpha_)) *
                           ((topic_word_count_[k][word_id] + beta_) /
                            (topic_count_[k] + vocab_size_ * beta_));
                    sum_p += p[k];
                }
                for (int k = 0; k < num_topics_; ++k) p[k] /= sum_p;
                int new_topic = sample(p);
                if (new_topic != old_topic) ++topic_changes;

                topics_[d][n] = new_topic;
                doc_topic_count_[d][new_topic]++;
                topic_word_count_[new_topic][word_id]++;
                topic_count_[new_topic]++;
                doc_count_[d]++;
            }
        }
        std::cout << "Iteration " << iter + 1 << ": " << topic_changes
                  << " words changed topics" << std::endl;
    }
}

void Lda::printTopics() const {
    for (int k = 0; k < num_topics_; ++k) {
        std::cout << "Topic " << k << ": ";
        std::vector<std::pair<int, int>> word_counts(topic_word_count_[k].begin(),
                                                     topic_word_count_[k].end());
        std::sort(word_counts.begin(), word_counts.end(),
                  [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                      return a.second > b.second;
                  });
        int top_words = 10;
        for (const auto& p : word_counts) {
            if (top_words == 0) break;
            std::cout << id2word_.at(p.first) << "(" << p.second << ") ";
            --top_words;
        }
        std::cout << std::endl;
    }
}

void Lda::saveDocTopics(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    for (std::size_t d = 0; d < doc_topic_count_.size(); ++d) {
        for (int k = 0; k < num_topics_; ++k) {
            double prob = (doc_topic_count_[d][k] + alpha_) / (doc_count_[d] + num_topics_ * alpha_);
            file << prob;
            if (k < num_topics_ - 1) file << "\t";
        }
        file << "\n";
    }
}

void Lda::saveTopicWords(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    for (int k = 0; k < num_topics_; ++k) {
        std::vector<std::pair<int, double>> word_probs;
        for (const auto& [word_id, count] : topic_word_count_[k]) {
            double prob = (count + beta_) / (topic_count_[k] + vocab_size_ * beta_);
            word_probs.emplace_back(word_id, prob);
        }
        std::sort(word_probs.begin(), word_probs.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        for (const auto& [word_id, prob] : word_probs) {
            file << id2word_.at(word_id) << ":" << prob << "\t";
        }
        file << "\n";
    }
}

}  // namespace mytools
