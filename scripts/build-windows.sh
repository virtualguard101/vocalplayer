#!/usr/bin/env bash
# Cross-build VocalPlayer for Windows (x86_64) from a Linux host using vcpkg
# manifest + community triplet x64-mingw-static (same third-party flow as
# `just bootstrap` / GitHub Actions, with MinGW instead of x64-linux).
#
# Usage:
#   scripts/build-windows.sh [options]
#
# Prerequisites:
#   - VCPKG_ROOT pointing at a vcpkg clone (baseline should match
#     vcpkg-configuration.json for reproducible installs).
#   - MinGW-w64 GCC with prefix x86_64-w64-mingw32- (e.g. gcc-mingw-w64-x86-64).
#   - cmake, ninja (recommended).
#
# vcpkg packages for this triplet are installed under
#   <build-dir>/vcpkg_installed/ (default build-dir: build-win)
# so the repo-root vcpkg_installed/x64-linux used by `just bootstrap` is left
# untouched. First CMake configure runs manifest install for x64-mingw-static.
#
# Options:
#   -c, --clean        Remove the build directory before configuring.
#   -B, --build-dir D  Build directory (default: build-win).
#   -j, --jobs N       Parallel build jobs (default: nproc).
#       --no-tests     Skip ctest after build.
#       --no-wine      Do not auto-wire Wine as the test emulator.
#   -r, --run [ARGS]   After build, launch vocalplayer.exe via Wine. Any
#                      remaining args are forwarded to the executable.
#   -t, --type TYPE    CMAKE_BUILD_TYPE (default: Release).
#   -h, --help         Show this help and exit.

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
VCPKG_TRIPLET="x64-mingw-static"
CHAINLOAD_FILE="${PROJECT_ROOT}/cmake/toolchains/mingw-w64-vcpkg-chainload.cmake"
MINGW_PREFIX="x86_64-w64-mingw32"

BUILD_DIR="build-win"
BUILD_TYPE="Release"
JOBS="$(nproc 2>/dev/null || echo 4)"
DO_CLEAN=0
RUN_TESTS=1
USE_WINE=1
RUN_AFTER_BUILD=0
RUN_FORWARD_ARGS=()

color() {
  # $1: color code, $2: text
  if [[ -t 1 ]]; then
    printf '\033[%sm%s\033[0m\n' "$1" "$2"
  else
    printf '%s\n' "$2"
  fi
}
info()  { color "1;34" "[build-windows] $*"; }
warn()  { color "1;33" "[build-windows] $*"; }
error() { color "1;31" "[build-windows] $*" >&2; }

usage() {
  sed -n '2,24p' "$0" | sed 's/^# \{0,1\}//'
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -c|--clean)    DO_CLEAN=1; shift ;;
    -B|--build-dir)
      [[ $# -ge 2 ]] || { error "missing value for $1"; exit 2; }
      BUILD_DIR="$2"; shift 2 ;;
    -j|--jobs)
      [[ $# -ge 2 ]] || { error "missing value for $1"; exit 2; }
      JOBS="$2"; shift 2 ;;
    --no-tests)    RUN_TESTS=0; shift ;;
    --no-wine)     USE_WINE=0; shift ;;
    -t|--type)
      [[ $# -ge 2 ]] || { error "missing value for $1"; exit 2; }
      BUILD_TYPE="$2"; shift 2 ;;
    -r|--run)
      RUN_AFTER_BUILD=1; shift
      while [[ $# -gt 0 ]]; do
        RUN_FORWARD_ARGS+=("$1"); shift
      done
      ;;
    -h|--help)     usage; exit 0 ;;
    --) shift; break ;;
    *)
      error "Unknown argument: $1"
      usage
      exit 2 ;;
  esac
done

cd "$PROJECT_ROOT"

require_tool() {
  if ! command -v "$1" >/dev/null 2>&1; then
    error "Missing required tool: $1"
    exit 1
  fi
}

if [[ -z "${VCPKG_ROOT:-}" ]]; then
  error "VCPKG_ROOT is not set. Clone vcpkg, run ./bootstrap-vcpkg.sh, then:"
  error "  export VCPKG_ROOT=/path/to/vcpkg"
  error "Use the same baseline as vcpkg-configuration.json for reproducible builds."
  exit 1
fi

VCPKG_TOOLCHAIN="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
if [[ ! -f "${VCPKG_TOOLCHAIN}" ]]; then
  error "Invalid VCPKG_ROOT: missing ${VCPKG_TOOLCHAIN}"
  exit 1
fi

if [[ ! -f "${CHAINLOAD_FILE}" ]]; then
  error "Missing chainload toolchain: ${CHAINLOAD_FILE}"
  exit 1
fi

require_tool cmake
require_tool "${MINGW_PREFIX}-gcc"
require_tool "${MINGW_PREFIX}-g++"

GENERATOR_ARGS=()
if command -v ninja >/dev/null 2>&1; then
  GENERATOR_ARGS+=( -G Ninja )
else
  warn "ninja not found, falling back to default generator (slower)"
fi

WINE_BIN=""
if [[ $USE_WINE -eq 1 ]]; then
  if command -v wine >/dev/null 2>&1; then
    WINE_BIN="$(command -v wine)"
  elif command -v wine64 >/dev/null 2>&1; then
    WINE_BIN="$(command -v wine64)"
  fi
fi

if [[ $DO_CLEAN -eq 1 ]]; then
  info "Removing existing build directory: ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

info "VCPKG_ROOT     : ${VCPKG_ROOT}"
info "Triplet        : ${VCPKG_TRIPLET}"
info "Chainload      : ${CHAINLOAD_FILE}"
info "Build dir      : ${BUILD_DIR}"
info "Build type     : ${BUILD_TYPE}"
info "Parallel jobs  : ${JOBS}"
if [[ -n "$WINE_BIN" ]]; then
  info "Test emulator  : ${WINE_BIN}"
else
  info "Test emulator  : (none; tests will be skipped or executed natively)"
fi

VCPKG_INSTALLED_DIR="${PROJECT_ROOT}/${BUILD_DIR}/vcpkg_installed"
info "vcpkg installed dir: ${VCPKG_INSTALLED_DIR}"

CONFIGURE_ARGS=(
  -S .
  -B "${BUILD_DIR}"
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_TOOLCHAIN}"
  -DVCPKG_TARGET_TRIPLET="${VCPKG_TRIPLET}"
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="${CHAINLOAD_FILE}"
  -DVCPKG_INSTALLED_DIR="${VCPKG_INSTALLED_DIR}"
  -DVOCALPLAYER_FIND_TAGLIB=OFF
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
)
CONFIGURE_ARGS+=( "${GENERATOR_ARGS[@]}" )
if [[ -n "$WINE_BIN" ]]; then
  CONFIGURE_ARGS+=( -DCMAKE_CROSSCOMPILING_EMULATOR="${WINE_BIN}" )
fi

info "Configuring (CMake)..."
cmake "${CONFIGURE_ARGS[@]}"

info "Building (jobs=${JOBS})..."
cmake --build "${BUILD_DIR}" -j "${JOBS}"

BINARY_PATH="${BUILD_DIR}/vocalplayer.exe"
if [[ ! -f "$BINARY_PATH" ]]; then
  error "Expected binary not found: ${BINARY_PATH}"
  exit 1
fi
info "Built: ${BINARY_PATH}"

if [[ $RUN_TESTS -eq 1 ]]; then
  if [[ -n "$WINE_BIN" ]]; then
    info "Running tests via ctest..."
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
  else
    warn "Skipping ctest: wine not available and --no-wine not enforced."
    warn "Install wine or pass --no-tests to silence this notice."
  fi
fi

if [[ $RUN_AFTER_BUILD -eq 1 ]]; then
  if [[ -z "$WINE_BIN" ]]; then
    error "--run requested but wine is not available."
    exit 1
  fi
  info "Launching: ${WINE_BIN} ${BINARY_PATH} ${RUN_FORWARD_ARGS[*]:-}"
  "${WINE_BIN}" "${BINARY_PATH}" "${RUN_FORWARD_ARGS[@]}"
fi

info "Done."
