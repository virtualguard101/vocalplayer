set_project("vocalplayer")
set_version("0.2.0")
set_languages("cxx20")
add_rules("mode.debug", "mode.release")

option("build_tests")
    set_default(true)
    set_showmenu(true)
    set_description("Build vocalplayer test binaries.")
option_end()

option("kissfft_source_dir")
    set_default("build/_deps/kissfft-src")
    set_showmenu(true)
    set_description("Path to kissfft source directory containing kiss_fft.c and kiss_fft.h.")
option_end()

add_requires("miniaudio")
add_requires("ftxui 5.0.0")
add_requires("taglib", {optional = true})

local kissfft_source_dir = get_config("kissfft_source_dir") or "build/_deps/kissfft-src"
if not os.isdir(kissfft_source_dir) then
    raise("kissfft source not found at " .. kissfft_source_dir ..
          ", pass --kissfft_source_dir=/path/to/kissfft")
end

target("kissfft_lib")
    set_kind("static")
    add_files(path.join(kissfft_source_dir, "kiss_fft.c"))
    add_includedirs(kissfft_source_dir, {public = true})

target("vocalplayer_core")
    set_kind("static")
    add_files(
        "src/app/app_controller.cpp",
        "src/app/playlist.cpp",
        "src/audio/audio_engine.cpp",
        "src/audio/decoder.cpp",
        "src/audio/metadata.cpp",
        "src/analysis/spectrum_analyzer.cpp",
        "src/ui/tui_renderer.cpp"
    )
    add_includedirs("src", {public = true})
    add_packages("miniaudio", "ftxui", {public = true})
    add_deps("kissfft_lib")

    if has_package("taglib") then
        add_packages("taglib", {public = true})
        add_defines("VOCALPLAYER_HAS_TAGLIB=1", {public = true})
    end

target("vocalplayer")
    set_kind("binary")
    add_files("src/main.cpp")
    add_deps("vocalplayer_core")

if has_config("build_tests") then
    target("test_spectrum_analyzer")
        set_kind("binary")
        add_files("tests/test_spectrum_analyzer.cpp", "src/analysis/spectrum_analyzer.cpp")
        add_includedirs("src")
        add_deps("kissfft_lib")

    target("test_playlist")
        set_kind("binary")
        add_files("tests/test_playlist.cpp", "src/app/playlist.cpp")
        add_includedirs("src")

    target("test_keybindings")
        set_kind("binary")
        add_files("tests/test_keybindings.cpp")
        add_includedirs("src")

    target("test_theme")
        set_kind("binary")
        add_files("tests/test_theme.cpp")
        add_includedirs("src")
        add_packages("ftxui")
end
