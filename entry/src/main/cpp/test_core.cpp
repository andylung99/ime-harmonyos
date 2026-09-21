#include <cstdio>

#include "EngineCore.h"
#include "WubiDict.h"

using namespace ime;

static KeyPos FindKey(const std::vector<KeyPos>& layout, char letter) {
  for (const auto& kp : layout) {
    if (kp.letter == letter) {
      return kp;
    }
  }
  return KeyPos{letter, 0.0f, 0.0f};
}

int main() {
  EngineCore core;

  // 场景 1：点击 "hello" 键中心加轻微抖动，期望纠错出 hello
  const char* word = "hello";
  for (int i = 0; word[i] != '\0'; ++i) {
    KeyPos kp = FindKey(core.Layout(), word[i]);
    core.Feed(kp.cx + 2.0f, kp.cy - 1.5f);
  }
  printf("== 英文倾向候选择（含抖动） ==\n");
  for (const auto& c : core.Decode(6)) {
    printf("  %s\n", c.c_str());
  }

  // 场景 2：拼音串切分 demo，输入 "nihao"
  core.Reset();
  const char* py = "nihao";
  for (int i = 0; py[i] != '\0'; ++i) {
    KeyPos kp = FindKey(core.Layout(), py[i]);
    core.Feed(kp.cx, kp.cy);
  }
  printf("== 拼音候选（nihao -> 你好） ==\n");
  for (const auto& c : core.Decode(6)) {
    printf("  %s\n", c.c_str());
  }

  // 场景 3：个性化学习后再次排序
  core.Learn("kind");
  printf("== learn(kind) 后候选 ==\n");
  for (const auto& c : core.Decode(6)) {
    printf("  %s\n", c.c_str());
  }

  // 场景 4：五笔查询与拼音反查
  WubiDict wu;
  std::string tab = "gkhg 或 1000\nnnvn 五 900\nqyte 你 800\nwtrt 的 100\n"
                    "tkgf 五笔 600\n";
  std::string pyTab = "我 wo\n你 ni\n五 wu\n或 huo\nto too\n";
  wu.Load(tab, pyTab);
  printf("== 五笔查询 QYTE ==\n");
  for (const auto& e : wu.Query("QYTE", 5)) {
    printf("  %s (%lld)\n", e.word.c_str(), e.freq);
  }
  printf("== 拼音反查 ni ==\n");
  for (const auto& e : wu.ReverseByPinyin("ni", 5)) {
    printf("  %s (%lld)\n", e.word.c_str(), e.freq);
  }

  // 场景 5：万能键 Z（NNZN 第三个位置用 Z 通配，应命中 nnvn -> 五）
  printf("== 万能键 Z：Query(\"nnzn\") ==\n");
  for (const auto& e : wu.Query("nnzn", 5)) {
    printf("  %s (%lld)\n", e.word.c_str(), e.freq);
  }
  return 0;
}