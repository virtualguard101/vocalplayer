# MinGW-w64 cross toolchain (Linux host -> Windows x86_64 target).
#
# Prefer the vcpkg-aligned flow (manifest deps, same as CI / `just bootstrap`):
#   see `scripts/build-windows.sh` or CMake preset `mingw-cross`, which use
#   `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` + triplet `x64-mingw-static`
#   and `mingw-w64-vcpkg-chainload.cmake` for compiler/pkg-config pinning.
#
# This file remains for ad-hoc CMake without vcpkg (no third-party packages from
# vcpkg); `find_path(miniaudio)` and `find_package(ftxui)` will not resolve unless
# you supply those deps yourself.
#
# Usage (legacy, no vcpkg):
#   cmake -S . -B build-win -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake \
#     -DCMAKE_BUILD_TYPE=Release
#
# Notes:
# - Produces fully static PE binaries (no extra MinGW runtime DLLs needed).
# - TagLib is not present in the MinGW sysroot by default, so the project
#   falls back to filename-based metadata (VOCALPLAYER_HAS_TAGLIB undefined).

set(CMAKE_SYSTEM_NAME      Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(VOCALPLAYER_MINGW_PREFIX "x86_64-w64-mingw32"
    CACHE STRING "MinGW-w64 toolchain prefix")

set(CMAKE_C_COMPILER   ${VOCALPLAYER_MINGW_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${VOCALPLAYER_MINGW_PREFIX}-g++)
set(CMAKE_RC_COMPILER  ${VOCALPLAYER_MINGW_PREFIX}-windres)
set(CMAKE_AR           ${VOCALPLAYER_MINGW_PREFIX}-ar)
set(CMAKE_RANLIB       ${VOCALPLAYER_MINGW_PREFIX}-ranlib)
set(CMAKE_STRIP        ${VOCALPLAYER_MINGW_PREFIX}-strip)

# Constrain find_* to the MinGW sysroot so we never pick up Linux libs by mistake.
set(CMAKE_FIND_ROOT_PATH /usr/${VOCALPLAYER_MINGW_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# CMAKE_FIND_ROOT_PATH_MODE_* does NOT constrain pkg-config. Without the env
# overrides below, pkg_check_modules() happily resolves host packages such as
# TagLib via /usr/lib/pkgconfig, which then injects /usr/include (Linux glibc)
# into MinGW compile lines and breaks the build with a cascade of conflicting
# typedef and missing-symbol errors. Pin pkg-config to the MinGW sysroot.
set(ENV{PKG_CONFIG_LIBDIR}
    "/usr/${VOCALPLAYER_MINGW_PREFIX}/lib/pkgconfig:/usr/${VOCALPLAYER_MINGW_PREFIX}/share/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/usr/${VOCALPLAYER_MINGW_PREFIX}")

# Static link the MinGW C/C++ runtimes and winpthreads so the .exe is portable.
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-static -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-static-libgcc -static-libstdc++")
