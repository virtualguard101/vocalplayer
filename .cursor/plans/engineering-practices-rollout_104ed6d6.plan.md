---
name: engineering-practices-rollout
overview: 分阶段落地本地提交门禁、CI 静态检查与基于 tag 的多平台发布流程，先保证开发体验，再逐步提升质量门槛。
todos:
  - id: precommit-bootstrap
    content: 设计并落地 pre-commit hooks（clang-format 自动修复 + 快速测试）
    status: completed
  - id: ci-quality-gate
    content: 新增 CI workflow，接入 clang-tidy、构建与测试流程
    status: completed
  - id: tag-release-pipeline
    content: 新增 tag 触发的多平台 release workflow 并上传构建产物
    status: completed
  - id: docs-and-changelog-sync
    content: 同步 README/README_zh-CN/changelog 的工程实践说明
    status: completed
isProject: false
---

# VocalPlayer Engineering Practices Plan

## 目标与范围
在不影响日常开发效率的前提下，完成三项工程化能力建设：
- 本地 `pre-commit`（`clang-format` 自动修复 + 快速测试门禁）
- GitHub CI（`clang-tidy` + 构建 + 测试）
- 基于 `git tag` 的多平台发布（Linux/macOS/Windows）

## 现状基线
- 当前无 GitHub workflow 和 pre-commit 配置。
- 现有构建/测试命令已在 [README.md](/home/virtualguard/vg101/dev/vocalplayer/README.md)、[README_zh-CN.md](/home/virtualguard/vg101/dev/vocalplayer/README_zh-CN.md)、[justfile](/home/virtualguard/vg101/dev/vocalplayer/justfile) 中约定。
- CMake 已启用 `CMAKE_EXPORT_COMPILE_COMMANDS`，满足 `clang-tidy` 运行前提（见 [CMakeLists.txt](/home/virtualguard/vg101/dev/vocalplayer/CMakeLists.txt)）。

## 实施阶段

### 阶段 1：本地提交门禁（Pre-commit）
- 新增 `pre-commit` 配置文件，定义两个 hook：
  - `clang-format`：仅针对已暂存 C/C++ 文件自动修复；若有改动则阻止当前提交并提示 `git add`。
  - 快速测试：执行轻量测试命令（基于 `ctest` 或 `just test` 的快速路径）。
- 在 [README.md](/home/virtualguard/vg101/dev/vocalplayer/README.md) 与 [README_zh-CN.md](/home/virtualguard/vg101/dev/vocalplayer/README_zh-CN.md) 新增“安装与启用 pre-commit”说明。
- 补充 `just` 目标（如 `just hook-install`/`just check-fast`），统一团队入口。

### 阶段 2：CI 持续集成（PR / Push）
- 新增 CI workflow（如 `.github/workflows/ci.yml`），触发 `pull_request` 与 `push`。
- 任务拆分：
  - **lint job**：运行 `clang-tidy`（优先改动文件范围，降低时长）。
  - **build-and-test job**：Linux 环境执行 configure/build/ctest。
- 初始门禁策略：`clang-tidy` 先采用“可见但不过度阻断”的策略（告警输出可视化），稳定后切换为强门禁。
- 在仓库文档中明确“本地与 CI 检查职责分工”（本地快、CI严）。

### 阶段 3：Tag 驱动 Release（多平台）
- 新增 release workflow（如 `.github/workflows/release.yml`），触发条件：`push.tags: v*.*.*`。
- 构建矩阵覆盖 `ubuntu-latest` / `macos-latest` / `windows-latest`。
- 每个平台执行 configure/build/test 后打包二进制（统一命名：`vocalplayer-<tag>-<os>-<arch>`）。
- 使用 GitHub Release Action 自动创建 release 并上传各平台产物。
- 配置 workflow `permissions: contents: write`，确保可创建 release。

## 配置与文档同步
- 新增/更新文件（计划）：
  - [.pre-commit-config.yaml](/home/virtualguard/vg101/dev/vocalplayer/.pre-commit-config.yaml)
  - [.clang-tidy](/home/virtualguard/vg101/dev/vocalplayer/.clang-tidy)
  - [.github/workflows/ci.yml](/home/virtualguard/vg101/dev/vocalplayer/.github/workflows/ci.yml)
  - [.github/workflows/release.yml](/home/virtualguard/vg101/dev/vocalplayer/.github/workflows/release.yml)
  - [justfile](/home/virtualguard/vg101/dev/vocalplayer/justfile)
  - [README.md](/home/virtualguard/vg101/dev/vocalplayer/README.md)
  - [README_zh-CN.md](/home/virtualguard/vg101/dev/vocalplayer/README_zh-CN.md)
  - [changelog.md](/home/virtualguard/vg101/dev/vocalplayer/changelog.md)

## 验收标准
- 本地：提交时能自动格式化并阻止未重新暂存的格式变更；快速检查可重复执行。
- CI：PR 能看到 `clang-tidy` + build + test 结果，失败原因可定位。
- Release：推送 `vX.Y.Z` tag 后自动生成 GitHub Release，并包含三平台可下载构建产物。
- 文档：中英文 README 与 changelog 对应更新完成。