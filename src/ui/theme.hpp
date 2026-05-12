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
  kNeon,
  kMono,
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
  static const Theme kNeonTheme{
      .title_color = ftxui::Color::BlueLight,
      .text_color = ftxui::Color::White,
      .accent_color = ftxui::Color::CyanLight,
      .spectrum_color = ftxui::Color::BlueLight,
      .peak_color = ftxui::Color::YellowLight,
      .waveform_color = ftxui::Color::MagentaLight,
      .meter_color = ftxui::Color::GreenLight,
      .warning_color = ftxui::Color::YellowLight,
      .border_color = ftxui::Color::BlueLight,
  };
  static const Theme kMonoTheme{
      .title_color = ftxui::Color::White,
      .text_color = ftxui::Color::GrayLight,
      .accent_color = ftxui::Color::White,
      .spectrum_color = ftxui::Color::GrayLight,
      .peak_color = ftxui::Color::White,
      .waveform_color = ftxui::Color::GrayDark,
      .meter_color = ftxui::Color::White,
      .warning_color = ftxui::Color::GrayLight,
      .border_color = ftxui::Color::GrayLight,
  };

  switch (theme_id) {
    case ThemeId::kNeon:
      return kNeonTheme;
    case ThemeId::kMono:
      return kMonoTheme;
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
      return ThemeId::kNeon;
    case ThemeId::kNeon:
      return ThemeId::kMono;
    case ThemeId::kMono:
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
    case ThemeId::kNeon:
      return "Neon";
    case ThemeId::kMono:
      return "Mono";
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
