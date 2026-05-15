/**
 * @file console.hpp
 * @brief Platform-specific console preparation before TUI startup.
 */
#ifndef VOCALPLAYER_SRC_PLATFORM_CONSOLE_HPP_
#define VOCALPLAYER_SRC_PLATFORM_CONSOLE_HPP_

namespace vocalplayer {
namespace platform {

/**
 * @brief Applies OS-specific console settings needed for correct TUI output.
 *
 * @note On Windows this enables UTF-8 code page and UTF-8 locale when
 * available. POSIX builds use a no-op stub until extended.
 */
void PrepareConsoleEnvironment();

}  // namespace platform
}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_PLATFORM_CONSOLE_HPP_
