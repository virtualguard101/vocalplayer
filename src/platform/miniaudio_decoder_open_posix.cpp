/**
 * @file miniaudio_decoder_open_posix.cpp
 * @brief POSIX miniaudio decoder open using narrow UTF-8 paths.
 */
#include "platform/miniaudio_decoder_open.hpp"

namespace vocalplayer {
namespace platform {

ma_result MaDecoderInitFromUtf8Path(ma_decoder* decoder,
                                    const ma_decoder_config* config,
                                    const char* utf8_path) {
  return ma_decoder_init_file(utf8_path, config, decoder);
}

}  // namespace platform
}  // namespace vocalplayer
