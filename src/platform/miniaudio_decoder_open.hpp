/**
 * @file miniaudio_decoder_open.hpp
 * @brief Platform-specific miniaudio decoder open from a UTF-8 path string.
 */
#ifndef VOCALPLAYER_SRC_PLATFORM_MINIAUDIO_DECODER_OPEN_HPP_
#define VOCALPLAYER_SRC_PLATFORM_MINIAUDIO_DECODER_OPEN_HPP_

#include "miniaudio.h"

namespace vocalplayer {
namespace platform {

/**
 * @brief Initializes a miniaudio decoder from a UTF-8 filesystem path.
 *
 * @param decoder Decoder instance to initialize (non-null).
 * @param config Decoder format configuration (non-null).
 * @param utf8_path File path encoded as UTF-8 text (non-null).
 * @return `MA_SUCCESS` on success, otherwise a miniaudio error code.
 *
 * @note On Windows this uses wide-character file APIs for Unicode paths; on
 * POSIX hosts the path is passed as a narrow UTF-8 string to miniaudio.
 * @note On Windows, invalid UTF-8 in `utf8_path` throws `std::runtime_error`
 * before miniaudio is invoked.
 */
ma_result MaDecoderInitFromUtf8Path(ma_decoder* decoder,
                                    const ma_decoder_config* config,
                                    const char* utf8_path);

}  // namespace platform
}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_PLATFORM_MINIAUDIO_DECODER_OPEN_HPP_
