/**
 * @file miniaudio_decoder_open_windows.cpp
 * @brief Windows miniaudio decoder open using UTF-16 file APIs.
 */
#include <windows.h>

#include <stdexcept>
#include <string>

#include "platform/miniaudio_decoder_open.hpp"

namespace vocalplayer {
namespace {

std::wstring ConvertUtf8ToWide(const char* utf8_path) {
  int wide_size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_path,
                                      -1, nullptr, 0);
  if (wide_size <= 0) {
    throw std::runtime_error("Failed to convert UTF-8 path to UTF-16.");
  }
  std::wstring wide(static_cast<size_t>(wide_size), L'\0');
  int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_path,
                                      -1, wide.data(), wide_size);
  if (converted <= 0) {
    throw std::runtime_error("Failed to convert UTF-8 path to UTF-16.");
  }
  if (!wide.empty() && wide.back() == L'\0') {
    wide.pop_back();
  }
  return wide;
}

}  // namespace

namespace platform {

ma_result MaDecoderInitFromUtf8Path(ma_decoder* decoder,
                                    const ma_decoder_config* config,
                                    const char* utf8_path) {
  std::wstring wide_path = ConvertUtf8ToWide(utf8_path);
  return ma_decoder_init_file_w(wide_path.c_str(), config, decoder);
}

}  // namespace platform
}  // namespace vocalplayer
