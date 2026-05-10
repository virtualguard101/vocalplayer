#!/usr/bin/env bash

set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format is required but not found in PATH." >&2
  exit 1
fi

if [ "$#" -eq 0 ]; then
  exit 0
fi

for file_path in "$@"; do
  if [ -f "$file_path" ]; then
    clang-format -i "$file_path"
  fi
done

if ! git diff --quiet -- "$@"; then
  echo "clang-format updated files. Please run git add and commit again." >&2
  exit 1
fi
