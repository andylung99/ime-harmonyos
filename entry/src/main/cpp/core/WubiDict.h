#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace ime {

struct WubiEntry {
  std::string word;
  long long freq;
};

// 五笔字典：编码 -> 候选词，以及字 -> 拼音 反查
class WubiDict {
 public:
  // tab: 每行 "编码 词 词频"；pyTab: 每行 "单字 拼音1 [拼音2...]"
  void Load(const std::string& tab, const std::string& pyTab);

  std::vector<WubiEntry> Query(const std::string& code, int topN) const;
  // 拼音反查：输入拼音，返回能匹配的单字候选
  std::vector<WubiEntry> ReverseByPinyin(const std::string& pinyin, int topN) const;

 private:
  std::unordered_map<std::string, std::vector<WubiEntry>> code2words_;
  std::unordered_map<std::string, std::vector<std::string>> char2pinyin_;
  std::unordered_map<std::string, std::string> char2code_;
};

}  // namespace ime