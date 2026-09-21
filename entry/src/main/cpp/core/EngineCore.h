#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Gaussian.h"
#include "Trie.h"

namespace ime {

// 拼音切分与纠错解码引擎（无任何系统依赖，纯 C++，可独立编译测试）
// 骨架：坐标 -> 最近键量化 -> Trie 前缀/整词 beam 解码；支持基于高斯打分的误触纠正。
class EngineCore {
 public:
  EngineCore();
  ~EngineCore();

  EngineCore(const EngineCore&) = delete;
  EngineCore& operator=(const EngineCore&) = delete;

  void Reset();
  void SetSigma(float sigma);
  void Learn(const std::string& word);
  void ClearUserData();
  void Feed(float x, float y);
  void Backspace();

  // 对当前击键序列解码，返回 topN 个候选词（已按 score 降序）
  std::vector<std::string> Decode(int topN) const;

  const std::vector<KeyPos>& Layout() const { return layout_; }

 private:
  KeyPos* FindNearestKey(float x, float y);
  void BuildLayout();
  void LoadBuiltinDicts();

  std::vector<KeyPos> layout_;
  std::vector<float> xs_;
  std::vector<float> ys_;
  float sigma_ = 28.f;
  Trie trie_;
  std::unordered_map<std::string, long long> user_freq_;
};

}  // namespace ime