#ifndef VOCALPLAYER_SRC_UI_THEME_HPP_
#define VOCALPLAYER_SRC_UI_THEME_HPP_

#include <optional>
#include <string>

#include "ftxui/screen/color.hpp"

namespace vocalplayer {

/**
 * @brief Built-in theme identifiers for terminal rendering.
 */
enum class ThemeId {
  kDefault,
  kMiku,
  kTeto,
};

/**
 * @brief Color palette contract used by TUI renderer panels.
 */
struct Theme {
  ftxui::Color title_color = ftxui::Color::Cyan;
  ftxui::Color text_color = ftxui::Color::White;
  ftxui::Color accent_color = ftxui::Color::BlueLight;
  ftxui::Color spectrum_color = ftxui::Color::Cyan;
  ftxui::Color peak_color = ftxui::Color::YellowLight;
  ftxui::Color waveform_color = ftxui::Color::Magenta;
  ftxui::Color meter_color = ftxui::Color::GreenLight;
  ftxui::Color warning_color = ftxui::Color::Yellow;
  ftxui::Color border_color = ftxui::Color::GrayLight;
};

/**
 * @brief Return a built-in theme palette by ID.
 *
 * @param theme_id Built-in theme enum value.
 * @return const Theme& Immutable theme palette.
 */
inline const Theme& GetBuiltinTheme(ThemeId theme_id) {
  static const Theme kDefaultTheme{};
  static const Theme kMikuTheme{
      .title_color = ftxui::Color::RGB(57, 197, 187),
      .text_color = ftxui::Color::White,
      .accent_color = ftxui::Color::RGB(137, 248, 226),
      .spectrum_color = ftxui::Color::RGB(57, 197, 187),
      .peak_color = ftxui::Color::RGB(223, 255, 246),
      .waveform_color = ftxui::Color::RGB(0, 221, 192),
      .meter_color = ftxui::Color::RGB(98, 255, 210),
      .warning_color = ftxui::Color::RGB(255, 220, 122),
      .border_color = ftxui::Color::RGB(57, 197, 187),
  };
  static const Theme kTetoTheme{
      .title_color = ftxui::Color::RGB(234, 84, 141),
      .text_color = ftxui::Color::White,
      .accent_color = ftxui::Color::RGB(255, 165, 198),
      .spectrum_color = ftxui::Color::RGB(234, 84, 141),
      .peak_color = ftxui::Color::RGB(255, 218, 232),
      .waveform_color = ftxui::Color::RGB(255, 109, 165),
      .meter_color = ftxui::Color::RGB(255, 129, 179),
      .warning_color = ftxui::Color::RGB(255, 212, 92),
      .border_color = ftxui::Color::RGB(234, 84, 141),
  };

  switch (theme_id) {
    case ThemeId::kMiku:
      return kMikuTheme;
    case ThemeId::kTeto:
      return kTetoTheme;
    case ThemeId::kDefault:
    default:
      return kDefaultTheme;
  }
}

/**
 * @brief Return the next built-in theme for runtime cycling.
 *
 * @param current Current theme ID.
 * @return ThemeId Next theme ID.
 */
inline ThemeId NextThemeId(ThemeId current) {
  switch (current) {
    case ThemeId::kDefault:
      return ThemeId::kMiku;
    case ThemeId::kMiku:
      return ThemeId::kTeto;
    case ThemeId::kTeto:
    default:
      return ThemeId::kDefault;
  }
}

/**
 * @brief Build a user-friendly display name for a built-in theme.
 *
 * @param theme_id Built-in theme enum value.
 * @return std::string Theme display label.
 */
inline std::string GetThemeDisplayName(ThemeId theme_id) {
  switch (theme_id) {
    case ThemeId::kMiku:
      return "Miku";
    case ThemeId::kTeto:
      return "Teto";
    case ThemeId::kDefault:
    default:
      return "Default";
  }
}

/**
 * @brief Reserved extension point for future user-defined theme loading.
 *
 * @param config_path Path to a future theme config file.
 * @return std::optional<Theme> No value for now (parsing deferred).
 */
inline std::optional<Theme> LoadThemeFromConfig(
    const std::string& config_path) {
  (void)config_path;
  return std::nullopt;
}

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_THEME_HPP_
