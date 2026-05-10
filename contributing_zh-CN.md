## 贡献指南（简体中文）

[English](contributing.md) | 简体中文

感谢你参与改进 `vocalplayer`。

## 本地环境准备

- CMake >= 3.20
- C++20 编译器（`clang++` 或 `g++`）
- Python（用于 `pre-commit`）
- 可选：TagLib 开发包

```bash
just bootstrap
```

该命令会完成 CMake 配置并链接 `compile_commands.json` 供 clangd/LSP 使用，以及安装 `pre-commit` hooks。

## 本地质量门禁

### 代码格式（Google C++ Style）

本仓库遵循 Google C++ Style。仓库根目录已提供 `.clang-format` 配置。

```bash
rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i
```

或者：

```bash
just format
```

### 测试

```bash
ctest --test-dir build --output-on-failure
```

或者：

```bash
just test
```

### pre-commit Hooks

仓库根目录已提供 `.pre-commit-config.yaml` 配置。

安装 `pre-commit`：
```bash
pip install pre-commit
pre-commit install
```

默认 hooks 会执行：

- 对暂存的 C/C++ 文件执行 `clang-format` 自动修复
- 快速测试（`playlist_test`、`keybindings_test`）

如果格式化改动了文件，提交会被阻止；请重新 `git add` 后再次提交。

手动执行检查：

```bash
just qc
just test
```

建议开发过程中跑 `just qc`，发起 PR 前跑 `just test`。

## CI 与发布

- CI（PR / push 到 `main`）：
  - 对改动 C++ 源文件运行 `clang-tidy`（当前为非阻断引导阶段）
  - Linux 构建 + 全量测试
- Release（push `v*.*.*` tag）：
  - 自动构建并发布 Linux/macOS/Windows 产物
