# ime-harmonyos：HarmonyOS 4.2+ 自研输入法工程骨架

对应文档：《自研输入法方案.md》第九章（HarmonyOS 4.2+ 适配与优化）的工程落地。

## 功能范围（骨架可跑通的最小闭环）

- 输入法扩展 `InputMethodExtensionAbility`（`type: "ime"`）注册与生命周期；
- ArkUI 自绘键盘 + 候选条，坐标 feed 给解码引擎；
- C++ 解码引擎（Trie + Beam Search + 高斯误触打分 + 词频/用户学习）通过 NAPI 暴露；
- 拼音/英文整词与拼音词候选；
- 五笔编码查表 + 拼音反查 + 万能键 Z（通配任意字根，切换模式见设置页）；
- 设置页：模式切换、模糊音（σ 容错）、一键清除用户词库。

## 目录结构

```
ime-harmonyos/
├── AppScope/
│   ├── app.json5
│   └── systemConfig.json              # 系统应用配置位（默认占位）
├── build-profile.json5 / hvigorfile.ts / oh-package.json5
├── docs/五笔字典设计.md                # 五笔码表 + 拼音反查完整设计
└── entry/src/main/
    ├── module.json5                    # ime 扩展注册 + 权限
    ├── ets/
    │   ├── entryability/EntryAbility.ets
    │   ├── ime/ImeExtensionAbility.ets
    │   ├── engine/EngineBridge.ets + native.d.ts
    │   ├── keyboard/{KeyboardPage,KeyboardView,KeyButton,KeyLayout,CandidateBar}.ets
    │   └── pages/Index.ets             # 设置页
    ├── cpp/
    │   ├── CMakeLists.txt
    │   ├── napi_init.cpp               # NAPI 绑定（libengine.so）
    │   ├── core/EngineCore.{h,cpp}     # 拼音/英文纠错解码（纯 C++）
    │   ├── core/WubiDict.{h,cpp}       # 五笔表 + 拼音反查
    │   ├── core/Trie.h / Gaussian.h
    │   └── test_core.cpp               # 引擎独立冒烟测试（可 g++ 编译）
    └── resources/rawfile/              # wubi_sample.txt / pinyin_sample.txt
```

## 构建与运行

1. 使用 DevEco Studio 5.x（API 12 SDK）打开本目录。
2. 生成签名（Project Structure → Signing Configs → Automatically generate），并真机运行。
3. 安装后到「系统设置 > 系统和更新 > 语言和输入法」启用本输入法。
4. 在任意输入框长按弹出输入法菜单切换到本输入法。

注意：

- `ohos.permission.INPUT_METHOD_EXTENSION` 与 `InputMethodType` 元数据 Key、`getKeyboardController()`
  等接口以 API 12 官方文档为准（SDK 版本迭代偶尔调整签名）。
- 应用图标为占位图，替换 `resources/base/media/*.png` 后重新构建。

## C++ 引擎独立验证（不依赖 DevEco）

核心引擎是纯 C++17、无系统依赖，可在任意 g++ 环境验证：

```bash
g++ -std=c++17 -I entry/src/main/cpp/core \
  entry/src/main/cpp/test_core.cpp \
  entry/src/main/cpp/core/EngineCore.cpp \
  entry/src/main/cpp/core/WubiDict.cpp -o /tmp/test_core && /tmp/test_core
```

预期输出（含英文抖动纠错、拼音候选、五笔查询与拼音反查）。

## 与《自研输入法方案》映射与扩展点

| 方案章节 | 工程落地 | 待替换/扩展点 |
| --- | --- | --- |
| 9.3 工程结构 | 目录完整 | — |
| 9.4 输入法框架 | ImeExtensionAbility + KeyboardPage | 生命周期 API 以真机 SDK 校验 |
| 9.5 引擎鸿蒙化 | cpp/core（NAPI 桥在 napi_init.cpp） | 换成完整词库（见 docs） |
| 9.6 键盘自绘 | ArkUI 键盘 + 候选 | 长按/滑动/单手布局等 UI 增强 |
| 9.7 权限合规 | module.json5 仅申请 IME 权限 | 上架材料清单 |
| 9.9 12 周里程碑 | W1–W12 验收项 | — |

尚未实现的二期功能（已在文档/代码注释中标注）：拼音切分 DP、模糊音展开、
词组编码联想、`pinyin → chars` 正向索引、完整码表接入。