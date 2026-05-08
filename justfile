build:
    cmake -S . -B build
    cmake --build build -j

format:
    rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i 

test:
    ctest --test-dir build --output-on-failure

run audio_file_path:
    ./build/vocalplayer {{audio_file_path}}

clean:
    rm -rf build
