alias qc := quick-check
alias cw := cross-windows

build:
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build -j

quick-check:
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build -j --target test_playlist test_keybindings
    ctest --test-dir build --output-on-failure -R "(playlist_test|keybindings_test)"

bootstrap:
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    ln -sf build/compile_commands.json compile_commands.json
    pre-commit install

format:
    rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i 

test:
    ctest --test-dir build --output-on-failure

cross-windows *ARGS:
    scripts/build-windows.sh {{ARGS}}

clean:
    rm -rf build build-win
    rm -f compile_commands.json
