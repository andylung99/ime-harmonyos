# 华为云 CodeArts 云编译 + 鸿蒙云手机部署指南

本文档说明如何从当前 `ime-harmonyos` 工程产出可安装的 `.hap`，并部署到鸿蒙云手机验证。

## 0. 现状与限制（为什么用 CodeArts）

- 本地既有环境（云容器 `DevEnvC_MMS6bb`）是 **Linux ARM64 / Huawei Cloud EulerOS**，缺少 DevEco Studio、hvigor、HarmonyOS SDK，且无法跨架构模拟 x86_64 工具链。
- 官方 DevEco 工具链目前只稳定支持 Windows / macOS / Linux **x86_64**。
- AI 侧当前可用的连接器只有 GitHub 与 GitCode，**没有 CodeArts、没有云手机连接器**，所以 AI 无法代替你发起云编译或创建云手机，这些步骤需要你在控制台完成。

因此：**HAP 构建走华为云 CodeArts 云编译**（官方支持 HarmonyOS/OpenHarmony 模板，环境为 x86_64）；**部署走 `hdspace cloudphone`**。

## 1. 首选路径：CodeArts 云编译（Debug，先跑通）

Debug 模式配合自动签名即可产出 HAP，无需先申请发布证书，适合首轮验证。

1. 打开华为云 CodeArts 控制台 → **代码仓库 / 代码托管**。
2. 导入外部仓库：`https://github.com/andylung99/ime-harmonyos.git`（若已用过 GitCode 也可接 GitCode 仓库）。
3. 新建 **构建任务**：
   - 构建模板：`HarmonyOS` 或 `OpenHarmony`（对应 CIPD/构建环境为 x86_64 Linux）。
   - 代码源：上面导入的仓库，分支 `main`。
   - 构建命令（重点）：
     ```bash
     # 安装依赖（若模板未内置 hvigor）
     npm config set registry https://repo.harmonyos.com/npm/
     npm install -g @ohos/hvigor
     # 首次构建
     hvigorw assembleHap --mode module -p product=default --no-daemon
     ```
     > 说明：本仓库根目录已含 `hvigorw`、`hvigor/` 配置与 `build-profile.json5`，正常情况下模板会从 `oh-package.json5` 检查依赖并自动拉起 hvigor。
   - 关键环境要求：构建机需包含 **HarmonyOS SDK（API 12）、命令行工具（ohpm / hvigor）、Node.js 18+**；若模板未预置 SDK 会报「找不到 SDK」，需在模板或流水线里选择带 DevEco 工具链的镜像。
4. 产物路径（构建成功后下载）：
   ```
   entry/build/default/outputs/default/entry-default-unsigned.hap
   entry/build/default/outputs/default/entry-default-signed.hap   # 自动签名成功后
   ```
5. 先跑通 Debug HAP；确认工程能编译后再处理签名与 Release。

## 2. 签名（上机安装 / 发布需要）

安装到云手机或真机时，需要有效签名。材料在 **AppGallery Connect / 开发者联盟** 申请：

- `.p12`：调试或发布的数字证书（含公私钥）。
- `.cer`：证书。
- `.p7b`：证书链（Profile 里的 Certificate）。

拿到后，二选一配置：

### 方式 A：修改 `build-profile.json5`（命令行构建用）

```json5
{
  "app": {
    "signingConfigs": [
      {
        "name": "default",
        "type": "HarmonyOS",
        "material": {
          "certpath": "./sign/xxx.cer",
          "storePassword": "<p12密码>",
          "keyAlias": "<证书别名>",
          "keyPassword": "<key密码>",
          "profile": "./sign/xxx.p7b",
          "signAlg": "SHA256withECDSA",
          "storeFile": "./sign/xxx.p12"
        }
      }
    ],
    "products": [
      {
        "name": "default",
        "signingConfig": "default"
        // ...
      }
    ]
  }
}
```

> 别名/密码等可用 `keytool -list -v -keystore xxx.p12` 查看。敏感信息勿提交到公开仓库（可加 `.gitignore` 排除 `sign/`）。

### 方式 B：DevEco Studio 可视化签名

File → Project Structure → Signing Configs → 勾选 Support HarmonyOS → 填入 p12/cer/p7b 路径与密码，自动生成到本地配置。

## 3. Release 构建

签名配置就绪后：

```bash
hvigorw assembleHap --mode module -p product=default -p buildMode=release --no-daemon
```

产物同样在 `entry/build/default/outputs/default/` 下。

## 4. 部署到鸿蒙云手机

```bash
# 1) 列出/创建云手机（首次会提示创建）
/tmp/hdspace cloudphone list
/tmp/hdspace cloudphone create --help   # 按提示选机型/规格后创建

# 2) 安装 HAP
/tmp/hdspace cloudphone install --apk <entry-default-signed.hap>

# 3) 启动输入法：设置 → 系统和更新 → 语言和输入法 → 勾选「自研输入法」→ 设为默认 IME
```

> `hdspace` 为已配置好 AK/SK 的 Arm64 CLI（位于 `/tmp/hdspace`）。具体子命令以 `hdspace cloudphone --help` 实际输出为准。

## 5. 验收清单

- [ ] Debug HAP 编译通过（在 CodeArts 构建机 x86_64 环境）
- [ ] 签名成功，云手机能安装 `.hap`
- [ ] 输入法出现在「语言和输入法」列表
- [ ] 英文键盘输入（`KeyboardPage` 布局渲染）
- [ ] 五笔编码查询可用：`qyte→你`、通配 `nnzn→五`、`qytz→你`
- [ ] 候选栏 `CandidateBar` 展示正常

## 6. 下一步工程优化（HAP 跑通后）

- 拼音切分 DP、模糊音展开
- 完整码表接入（`rawfile/wubi_sample.txt` / `pinyin_sample.txt` → 正式码表）
- `pinyin → chars` 正向索引
- IME 悬浮键盘 / 深色模式 / 主题切换