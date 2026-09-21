#include "WubiDict.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <utility>

namespace ime {

namespace {

std::vector<std::string> Split(const std::string& s) {
  std::vector<std::string> parts;
  std::istringstream iss(s);
  std::string tok;
  while (iss >> tok) {
    parts.push_back(tok);
  }
  return parts;
}

std::vector<std::string> SplitLines(const std::string& s) {
  std::vector<std::string> lines;
  std::string cur;
  for (char c : s) {
    if (c == '\n' || c == '\r') {
      if (!cur.empty()) {
        lines.push_back(cur);
        cur.clear();
      }
    } else {
      cur.push_back(c);
    }
  }
  if (!cur.empty()) {
    lines.push_back(cur);
  }
  return lines;
}

std::string UpperAscii(const std::string& s) {
  std::string o = s;
  for (char& c : o) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return o;
}

std::string LowerAscii(const std::string& s) {
  std::string o = s;
  for (char& c : o) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return o;
}

bool IsSingleHanChar(const std::string& s) {
  // UTF-8 单汉字 3 字节
  return s.size() == 3;
}

// 万能键 Z 最多允许出现在编码中的次数（防止 25^n 组合爆炸）
constexpr size_t kMaxWildcardZ = 2;

}  // namespace

void WubiDict::Load(const std::string& tab, const std::string& pyTab) {
  code2words_.clear();
  char2pinyin_.clear();
  char2code_.clear();

  for (const auto& line : SplitLines(tab)) {
    const auto parts = Split(line);
    if (parts.size() < 2) {
      continue;
    }
    std::string code = UpperAscii(parts[0]);
    std::string word = parts[1];
    long long freq = 100;
    if (parts.size() >= 3) {
      freq = std::atoll(parts[2].c_str());
    }
    code2words_[code].push_back({word, freq});
    if (IsSingleHanChar(word)) {
      if (code.size() >= 3 && code.size() <= 4) {
        char2code_[word] = code;
      }
    }
  }

  for (const auto& line : SplitLines(pyTab)) {
    const auto parts = Split(line);
    if (parts.size() < 2) {
      continue;
    }
    const std::string& word = parts[0];
    if (!IsSingleHanChar(word)) {
      continue;
    }
    std::vector<std::string> pys;
    for (size_t i = 1; i < parts.size(); ++i) {
      pys.push_back(parts[i]);
    }
    char2pinyin_[word] = pys;
  }
}

std::vector<WubiEntry> WubiDict::Query(const std::string& code, int topN) const {
  std::vector<WubiEntry> out;
  const std::string key = UpperAscii(code);  // 码表统一大写存储，查询参数规范化

  // 无万能键：精确命中
  if (key.find('Z') == std::string::npos) {
    auto it = code2words_.find(key);
    if (it == code2words_.end()) {
      return out;
    }
    auto items = it->second;
    std::sort(items.begin(), items.end(),
              [](const WubiEntry& a, const WubiEntry& b) { return a.freq > b.freq; });
    for (size_t i = 0; i < items.size() && out.size() < static_cast<size_t>(topN); ++i) {
      out.push_back(items[i]);
    }
    return out;
  }

  // 万能键 Z：把每个 Z 替换为 A–Y 任一键，枚举合并后按词频排序
  size_t zCount = static_cast<size_t>(std::count(key.begin(), key.end(), 'Z'));
  if (zCount > kMaxWildcardZ) {
    return out;  // 组合爆炸，放弃（业界习惯：Z 通配仅 1 个位置）
  }
  std::vector<std::string> combos(1, key);
  for (size_t i = 0; i < key.size(); ++i) {
    if (key[i] != 'Z') {
      continue;
    }
    std::vector<std::string> nxt;
    nxt.reserve(combos.size() * 25);
    for (const auto& s : combos) {
      for (char c = 'A'; c <= 'Y'; ++c) {
        std::string t = s;
        t[i] = c;
        nxt.push_back(std::move(t));
      }
    }
    combos = std::move(nxt);
  }
  std::unordered_map<std::string, long long> best;
  for (const auto& c : combos) {
    auto it = code2words_.find(c);
    if (it == code2words_.end()) {
      continue;
    }
    for (const auto& e : it->second) {
      auto& f = best[e.word];
      if (e.freq > f) {
        f = e.freq;
      }
    }
  }
  std::vector<WubiEntry> merged;
  merged.reserve(best.size());
  for (const auto& kv : best) {
    merged.push_back({kv.first, kv.second});
  }
  std::sort(merged.begin(), merged.end(),
            [](const WubiEntry& a, const WubiEntry& b) { return a.freq > b.freq; });
  for (size_t i = 0; i < merged.size() && out.size() < static_cast<size_t>(topN); ++i) {
    out.push_back(merged[i]);
  }
  return out;
}

std::vector<WubiEntry> WubiDict::ReverseByPinyin(const std::string& pinyin,
                                                 int topN) const {
  std::vector<WubiEntry> out;
  const std::string target = LowerAscii(pinyin);
  for (const auto& kv : char2pinyin_) {
    const std::string& word = kv.first;
    const auto& pys = kv.second;
    bool hit = false;
    for (const auto& py : pys) {
      if (LowerAscii(py) == target) {
        hit = true;
        break;
      }
    }
    if (!hit) {
      continue;
    }
    long long freq = 100;
    auto codeIt = char2code_.find(word);
    if (codeIt != char2code_.end()) {
      auto wIt = code2words_.find(codeIt->second);
      if (wIt != code2words_.end() && !wIt->second.empty()) {
        freq = wIt->second[0].freq;
      }
    }
    out.push_back({word, freq});
  }
  std::sort(out.begin(), out.end(),
            [](const WubiEntry& a, const WubiEntry& b) { return a.freq > b.freq; });
  if (out.size() > static_cast<size_t>(topN)) {
    out.resize(static_cast<size_t>(topN));
  }
  return out;
}

}  // namespace ime