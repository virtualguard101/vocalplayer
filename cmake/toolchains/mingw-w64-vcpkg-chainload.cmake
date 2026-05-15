# MinGW-w64 chainload for vcpkg (Linux host -> Windows x86_64 PE).
#
# Use with:
#   -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#   -DVCPKG_TARGET_TRIPLET=x64-mingw-static
#   -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$PWD/cmake/toolchains/mingw-w64-vcpkg-chainload.cmake
#
# vcpkg owns CMAKE_FIND_ROOT_PATH / installed package discovery; this file only
# pins the GCC prefix, static MinGW runtimes, and pkg-config to the MinGW
# sysroot so host Linux TagLib is never mixed into the cross build.

set(CMAKE_SYSTEM_NAME Windows CACHE STRING "")
set(CMAKE_SYSTEM_PROCESSOR AMD64 CACHE STRING "")

set(VOCALPLAYER_MINGW_PREFIX "x86_64-w64-mingw32"
    CACHE STRING "MinGW-w64 compiler/tool prefix (e.g. x86_64-w64-mingw32-gcc)")

set(CMAKE_C_COMPILER "${VOCALPLAYER_MINGW_PREFIX}-gcc" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "${VOCALPLAYER_MINGW_PREFIX}-g++" CACHE FILEPATH "")
set(CMAKE_RC_COMPILER "${VOCALPLAYER_MINGW_PREFIX}-windres" CACHE FILEPATH "")
set(CMAKE_AR "${VOCALPLAYER_MINGW_PREFIX}-ar" CACHE FILEPATH "")
set(CMAKE_RANLIB "${VOCALPLAYER_MINGW_PREFIX}-ranlib" CACHE FILEPATH "")
set(CMAKE_STRIP "${VOCALPLAYER_MINGW_PREFIX}-strip" CACHE FILEPATH "")

# Static link the MinGW C/C++ runtimes and winpthreads so the .exe is portable.
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-static -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-static-libgcc -static-libstdc++")

# pkg_check_modules does not honor CMAKE_FIND_ROOT_PATH; pin to MinGW .pc files.
set(ENV{PKG_CONFIG_LIBDIR}
    "/usr/${VOCALPLAYER_MINGW_PREFIX}/lib/pkgconfig:/usr/${VOCALPLAYER_MINGW_PREFIX}/share/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/usr/${VOCALPLAYER_MINGW_PREFIX}")
