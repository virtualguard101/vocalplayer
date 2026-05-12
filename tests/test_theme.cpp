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
  const Theme& miku_theme = GetBuiltinTheme(ThemeId::kMiku);
  const Theme& teto_theme = GetBuiltinTheme(ThemeId::kTeto);

  assert(GetThemeDisplayName(ThemeId::kDefault) == "Default");
  assert(GetThemeDisplayName(ThemeId::kMiku) == "Miku");
  assert(GetThemeDisplayName(ThemeId::kTeto) == "Teto");

  assert(NextThemeId(ThemeId::kDefault) == ThemeId::kMiku);
  assert(NextThemeId(ThemeId::kMiku) == ThemeId::kTeto);
  assert(NextThemeId(ThemeId::kTeto) == ThemeId::kDefault);

  assert(default_theme.title_color != miku_theme.title_color);
  assert(teto_theme.border_color != miku_theme.border_color);

  std::optional<Theme> maybe_theme = LoadThemeFromConfig("theme.toml");
  assert(!maybe_theme.has_value());

  return 0;
}
