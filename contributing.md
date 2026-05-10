# Contributing

English | [简体中文](contributing_zh-CN.md)

Thanks for improving `vocalplayer`.

## Local Setup

### Prerequisites

- CMake >= 3.20
- A C++20 compiler (`clang++` or `g++`)
- Python (for `pre-commit`)
- Optional: TagLib development package

### Formatting (Google C++ Style)

This repository follows Google C++ Style. A `.clang-format` config is provided
in the repository root.

```bash
rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i
```

or:

```bash
just format
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```

or:

```bash
just test
```

### Bootstrap

```bash
just bootstrap
```

This configures CMake, links `compile_commands.json` for clangd/LSP, and
installs git hooks via `pre-commit install`.

## Local Quality Gates

### Install pre-commit hooks

```bash
pip install pre-commit
just bootstrap
```

Configured hooks run:

- `clang-format` on staged C/C++ files (auto-fix)
- fast tests (`playlist_test`, `keybindings_test`)

If formatting changed files, commit is blocked intentionally. Re-stage and retry.

### Run checks manually

```bash
just qc
just test
```

Use `just qc` (alias of `just quick-check`) for quick feedback, and run
`just test` before opening a PR.

## CI

GitHub Actions runs:

- `clang-tidy` on changed C++ source files (bootstrap stage: non-blocking)
- Linux configure/build/full tests

CI triggers on pull requests and pushes to `main`.

## Release

Release workflow triggers when a semantic version tag is pushed:

```bash
git tag v0.2.0
git push --tags
```

It builds, tests, and publishes artifacts for Linux, macOS, and Windows.

### Release Process

1. Develop and document in `dev` branch, update `changelog.md` and project metadata, and submit PR.
2. Maintainer merges PR to `main`, CI automatically runs `clang-tidy` + Linux build/test pipeline.
3. Switch to `main` branch locally, and execute `git pull` to fetch latest code.
4. Execute `git tag -a v*.*.*` to create a new version tag.
5. Execute `git push --tags` to push the tag to the remote repository, which triggers the GitHub Actions release workflow, building and uploading Linux/macOS/Windows triple-platform artifacts.
