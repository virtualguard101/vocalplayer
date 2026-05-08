build:
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build -j

bootstrap:
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    ln -sf build/compile_commands.json compile_commands.json

format:
    rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i 

test:
    ctest --test-dir build --output-on-failure

run audio_file_path:
    ./build/vocalplayer {{audio_file_path}}

clean:
    rm -rf build
