#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace ime {

struct TrieNode {
  std::unordered_map<char, TrieNode*> next;
  bool terminal = false;
  std::vector<std::pair<std::string, long long>> words;

  ~TrieNode() {
    for (auto& kv : next) {
      delete kv.second;
    }
    next.clear();
  }
};

class Trie {
 public:
  Trie() : root_(new TrieNode()) {}
  ~Trie() { delete root_; }

  void Insert(const std::string& code, const std::string& word, long long freq) {
    TrieNode* cur = root_;
    for (char ch : code) {
      auto it = cur->next.find(ch);
      if (it == cur->next.end()) {
        cur->next[ch] = new TrieNode();
      }
      cur = cur->next[ch];
    }
    cur->terminal = true;
    cur->words.emplace_back(word, freq);
  }

  const TrieNode* Root() const { return root_; }

 private:
  TrieNode* root_;
};

}  // namespace ime