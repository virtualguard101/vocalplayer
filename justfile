alias qc := quick-check
alias cw := cross-windows

quick-check: build-debug
    ctest --test-dir build/debug --output-on-failure -R "(playlist_test|keybindings_test)"

bootstrap type="debug":
    vcpkg install
    cmake --preset {{ type }}
    ln -sf build/{{ type }}/compile_commands.json compile_commands.json
    pre-commit install

build-debug: (bootstrap "debug")
    cmake --preset debug
    cmake --build build/debug -j

build-release: (bootstrap "release")
    cmake --preset release
    cmake --build build/release -j

format:
    rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i 

test:
    ctest --test-dir build --output-on-failure

cross-windows *ARGS:
    scripts/build-windows.sh {{ARGS}}

clean:
    rm -rf build build-win
    rm -f compile_commands.json
