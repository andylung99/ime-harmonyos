#include "EngineCore.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <queue>
#include <utility>

namespace ime {

namespace {

constexpr float kKeyW = 36.0f;
constexpr float kKeyH = 50.0f;
constexpr int kBeamK = 32;

struct SomeWord {
  const char* code;
  const char* word;
  long long freq;
};

// 演示用英文高频词（替换点：换成完整英语词典）
const SomeWord kEnWords[] = {
    {"the", "the", 9000}, {"that", "that", 8000}, {"hello", "hello", 5000},
    {"world", "world", 4000}, {"find", "find", 3000}, {"kind", "kind", 2900},
    {"find", "find", 3000}, {"time", "time", 4500}, {"home", "home", 3200},
    {"good", "good", 2800}, {"friend", "friend", 2500}, {"keyboard", "keyboard", 1500},
    {"ime", "ime", 1200}, {"work", "work", 3500}, {"this", "this", 4200},
};

// 演示用拼音词（替换点：换成完整拼音词库，含模糊音展开）
const SomeWord kPyWords[] = {
    {"ni", "你", 10000}, {"hao", "好", 9000}, {"nihao", "你好", 8000},
    {"zhong", "中", 7000}, {"guo", "国", 6500}, {"zhongguo", "中国", 6000},
    {"wen", "文", 5000}, {"zi", "字", 4500}, {"wenzi", "文字", 4000},
    {"zuo", "作", 3800}, {"tian", "天", 3700}, {"qi", "气", 3500},
    {"xin", "新", 3400}, {"yong", "用", 3300}, {"shu", "书", 3200},
    {"shuru", "输入", 3000}, {"defa", "得法", 100}, {"wubi", "五笔", 2500},
    {"pinyin", "拼音", 2600},
};

}  // namespace

EngineCore::EngineCore() {
  BuildLayout();
  LoadBuiltinDicts();
}

EngineCore::~EngineCore() = default;

void EngineCore::BuildLayout() {
  layout_.clear();
  const char* rows[] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
  const int startCol[] = {0, 0, 1};
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < static_cast<int>(std::char_traits<char>::length(rows[r])); ++c) {
      KeyPos kp;
      kp.letter = rows[r][c];
      kp.cx = (static_cast<float>(startCol[r] + c) + 0.5f) * kKeyW;
      kp.cy = (static_cast<float>(r) + 0.5f) * kKeyH;
      layout_.push_back(kp);
    }
  }
}

void EngineCore::LoadBuiltinDicts() {
  for (const auto& w : kEnWords) {
    trie_.Insert(w.code, w.word, w.freq);
  }
  for (const auto& w : kPyWords) {
    trie_.Insert(w.code, w.word, w.freq);
  }
}

void EngineCore::Reset() {
  xs_.clear();
  ys_.clear();
}

void EngineCore::SetSigma(float sigma) {
  sigma_ = sigma;
}

void EngineCore::Learn(const std::string& word) {
  auto& f = user_freq_[word];
  f += 1;
}

void EngineCore::ClearUserData() {
  user_freq_.clear();
}

void EngineCore::Feed(float x, float y) {
  xs_.push_back(x);
  ys_.push_back(y);
}

void EngineCore::Backspace() {
  if (!xs_.empty()) {
    xs_.pop_back();
    ys_.pop_back();
  }
}

KeyPos* EngineCore::FindNearestKey(float x, float y) {
  KeyPos* best = nullptr;
  float bestD = 1e30f;
  for (auto& kp : layout_) {
    float dx = x - kp.cx;
    float dy = y - kp.cy;
    float d = dx * dx + dy * dy;
    if (d < bestD) {
      bestD = d;
      best = &kp;
    }
  }
  return best;
}

std::vector<std::string> EngineCore::Decode(int topN) const {
  std::vector<std::string> out;
  const size_t n = xs_.size();
  if (n == 0) {
    return out;
  }

  // 状态：节点指针、深度、累计得分
  using State = std::pair<double, const TrieNode*>;
  struct Bag {
    const TrieNode* node;
    std::string buf;
    double score;
  };
  struct BagCmp {
    bool operator()(const Bag& a, const Bag& b) const { return a.score < b.score; }
  };

  std::priority_queue<Bag, std::vector<Bag>, BagCmp> heap;
  heap.push({trie_.Root(), "", 0.0});

  for (size_t i = 0; i < n; ++i) {
    double maxScore = -1e30;
    std::vector<Bag> next;
    while (!heap.empty()) {
      Bag cur = heap.top();
      heap.pop();
      for (const auto& kv : cur.node->next) {
        const char ch = kv.first;
        const TrieNode* childNode = kv.second;
        // 匹配打分为：此点对该字母键的高斯得分（含误触信息）
        float best = -1e30f;
        const KeyPos* bestKey = nullptr;
        for (const auto& kp : layout_) {
          if (kp.letter == ch) {
            bestKey = &kp;
            break;
          }
        }
        double score = cur.score;
        if (bestKey != nullptr) {
          score += LogGaussian(xs_[i], ys_[i], *bestKey, sigma_);
        }
        if (score > maxScore) {
          maxScore = score;
        }
        Bag nb;
        nb.node = childNode;
        nb.buf = cur.buf + ch;
        nb.score = score;
        next.push_back(nb);
      }
    }
    // 只保留前 K 个得分状态（不剪枝时词典穷举会卡顿）
    std::sort(next.begin(), next.end(),
             [](const Bag& a, const Bag& b) { return a.score > b.score; });
    const int keep = static_cast<int>(std::min<size_t>(next.size(), kBeamK));
    for (int i2 = 0; i2 < keep; ++i2) {
      heap.push(next[static_cast<size_t>(i2)]);
    }
    (void)maxScore;
  }

  // 汇总全部终端态候选
  std::vector<std::pair<double, std::string>> ranked;
  std::unordered_map<std::string, double> seen;
  while (!heap.empty()) {
    Bag cur = heap.top();
    heap.pop();
    if (!cur.node->terminal) {
      continue;
    }
    for (const auto& wp : cur.node->words) {
      const std::string& word = wp.first;
      double lmScore = std::log(static_cast<double>(wp.second + 1));
      auto it = user_freq_.find(word);
      if (it != user_freq_.end()) {
        lmScore += std::log(static_cast<double>(it->second + 1)) * 0.3;
      }
      double total = cur.score + lmScore;
      auto sIt = seen.find(word);
      if (sIt == seen.end() || total > sIt->second) {
        seen[word] = total;
        ranked.emplace_back(total, word);
      }
    }
  }

  std::sort(ranked.begin(), ranked.end(),
            [](const std::pair<double, std::string>& a,
               const std::pair<double, std::string>& b) { return a.first > b.first; });

  for (size_t i = 0; i < ranked.size() && out.size() < static_cast<size_t>(topN); ++i) {
    out.push_back(ranked[i].second);
  }
  return out;
}

}  // namespace ime