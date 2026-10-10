#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

namespace mytools {

// 通用 LRU 缓存。header-only，直接 #include 即用。
//   mytools::LruCache<std::string, int> cache(2);
//   cache.put("a", 1);
//   int v; cache.get("a", v);
template <typename Key, typename Value>
class LruCache {
public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    bool get(const Key& key, Value& out) {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        list_.splice(list_.begin(), list_, it->second);
        out = it->second->second;
        return true;
    }

    void put(const Key& key, const Value& value) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = value;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }
        if (map_.size() >= capacity_ && !list_.empty()) {
            map_.erase(list_.back().first);
            list_.pop_back();
        }
        list_.emplace_front(key, value);
        map_[key] = list_.begin();
    }

    std::size_t size() const { return map_.size(); }
    bool empty() const { return map_.empty(); }

    void clear() {
        map_.clear();
        list_.clear();
    }

private:
    std::size_t capacity_;
    std::list<std::pair<Key, Value>> list_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_;
};

}  // namespace mytools
