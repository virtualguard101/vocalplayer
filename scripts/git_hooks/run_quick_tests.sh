#!/usr/bin/env bash

set -euo pipefail

echo "[quick-tests] Configuring project..."
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "[quick-tests] Building fast test targets..."
cmake --build build -j --target test_playlist test_keybindings

echo "[quick-tests] Running fast tests..."
ctest --test-dir build --output-on-failure -R "(playlist_test|keybindings_test)"
