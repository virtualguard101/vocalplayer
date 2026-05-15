## 贡献指南（简体中文）

[English](contributing.md) | 简体中文

感谢你参与改进 `vocalplayer`。

## 本地环境准备

- CMake >= 3.20
- vcpkg（用于管理第三方库）
- C++20 编译器（`clang++` 或 `g++`）
- Python（用于 `pre-commit`）
- 可选：TagLib 开发包

```bash
just bootstrap
```

该命令会完成 CMake 配置并链接 `compile_commands.json` 供 clangd/LSP 使用，并执行 `pre-commit install` 安装 git hooks。

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
just test
```

### pre-commit Hooks

仓库根目录已提供 `.pre-commit-config.yaml` 配置。

安装 `pre-commit`：
```bash
pip install pre-commit
just bootstrap
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

建议开发过程中跑 `just qc`（`just quick-check` 的别名），发起 PR 前跑
`just test`。

## CI

PR / push 到 `main` 时，CI 会自动运行 `clang-tidy` + Linux 构建测试流水线。

## 发布

推送 `v*.*.*` tag 时，CI 会自动构建并发布二进制产物。

注意使用 [Semantic Versioning](https://semver.org/) 规范的版本号。

```bash
git tag -a v0.1.0 -m "Release version 0.1.0"
git push --tags
```

### 发布流程

1. 在`dev`分支进行开发、文档工作，更新 `changelog.md` 与项目元信息并提交 PR。

2. 维护者合并 PR 到 `main`，CI 会自动进行 `clang-tidy` + Linux 构建测试流水线。

3. 在本地切换到 `main` 分支，并执行 `git pull` 拉取最新代码。

4. 执行 `git tag -a v*.*.*` 创建新版本标签。

5. 执行 `git push --tags` 推送标签到远程仓库，这会触发 GitHub Actions 发布流程，构建并上传二进制产物。
