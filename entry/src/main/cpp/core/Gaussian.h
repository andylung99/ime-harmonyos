#pragma once

#include <cmath>

namespace ime {

struct KeyPos {
  char letter;
  float cx;  // 键中心 x（vp）
  float cy;  // 键中心 y（vp）
};

// 二维高斯对数打分：点 (x,y) 想按 target 键的概率（省略归一化常数，仅用于排序）
inline double LogGaussian(float x, float y, const KeyPos& key, float sigma) {
  double dx = static_cast<double>(x) - key.cx;
  double dy = static_cast<double>(y) - key.cy;
  double s = static_cast<double>(sigma);
  return -0.5 * (dx * dx + dy * dy) / (s * s);
}

}  // namespace ime