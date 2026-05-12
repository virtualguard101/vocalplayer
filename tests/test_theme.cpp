#include <cassert>
#include <optional>
#include <string>

#include "ui/theme.hpp"

int main() {
  using vocalplayer::GetBuiltinTheme;
  using vocalplayer::GetThemeDisplayName;
  using vocalplayer::LoadThemeFromConfig;
  using vocalplayer::NextThemeId;
  using vocalplayer::Theme;
  using vocalplayer::ThemeId;

  const Theme& default_theme = GetBuiltinTheme(ThemeId::kDefault);
  const Theme& neon_theme = GetBuiltinTheme(ThemeId::kNeon);
  const Theme& mono_theme = GetBuiltinTheme(ThemeId::kMono);

  assert(GetThemeDisplayName(ThemeId::kDefault) == "Default");
  assert(GetThemeDisplayName(ThemeId::kNeon) == "Neon");
  assert(GetThemeDisplayName(ThemeId::kMono) == "Mono");

  assert(NextThemeId(ThemeId::kDefault) == ThemeId::kNeon);
  assert(NextThemeId(ThemeId::kNeon) == ThemeId::kMono);
  assert(NextThemeId(ThemeId::kMono) == ThemeId::kDefault);

  assert(default_theme.title_color != neon_theme.title_color);
  assert(mono_theme.border_color != neon_theme.border_color);

  std::optional<Theme> maybe_theme = LoadThemeFromConfig("theme.toml");
  assert(!maybe_theme.has_value());

  return 0;
}
