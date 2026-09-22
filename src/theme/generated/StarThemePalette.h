// GENERATED FILE - do not edit. Source: theme/tokens.json
// Regenerate with: python3 tools/gen_theme.py
#pragma once

#include <QColor>
#include <QString>

namespace StarTheme {

struct Color {
    const char *hex;                 // '#rrggbb' or 'rgba(r,g,b,a)'
    QColor qcolor() const;           // parsed for QPalette painting
};

// Starlight
namespace AccentStarlight {
inline constexpr Color accent { "#7C6BFF" };
inline constexpr Color accentHover { "#9083FF" };
inline constexpr Color accentPressed { "#6450F0" };
inline constexpr Color accentSoft { "rgba(124,107,255,0.14)" };
inline constexpr Color accentSoftStrong { "rgba(124,107,255,0.24)" };
inline constexpr Color accentRing { "rgba(124,107,255,0.45)" };
inline constexpr Color gradientFrom { "#8B7BFF" };
inline constexpr Color gradientTo { "#4FC3F7" };
inline constexpr Color accentRgb { "124,107,255" };
}

// Aurora
namespace AccentAurora {
inline constexpr Color accent { "#35D6ED" };
inline constexpr Color accentHover { "#5FE3F5" };
inline constexpr Color accentPressed { "#1FBAD2" };
inline constexpr Color accentSoft { "rgba(53,214,237,0.14)" };
inline constexpr Color accentSoftStrong { "rgba(53,214,237,0.24)" };
inline constexpr Color accentRing { "rgba(53,214,237,0.45)" };
inline constexpr Color gradientFrom { "#35D6ED" };
inline constexpr Color gradientTo { "#7C6BFF" };
inline constexpr Color accentRgb { "53,214,237" };
}

// Nova
namespace AccentNova {
inline constexpr Color accent { "#FF5FA2" };
inline constexpr Color accentHover { "#FF7DB6" };
inline constexpr Color accentPressed { "#EC4A8D" };
inline constexpr Color accentSoft { "rgba(255,95,162,0.14)" };
inline constexpr Color accentSoftStrong { "rgba(255,95,162,0.24)" };
inline constexpr Color accentRing { "rgba(255,95,162,0.45)" };
inline constexpr Color gradientFrom { "#FF7DB6" };
inline constexpr Color gradientTo { "#8B7BFF" };
inline constexpr Color accentRgb { "255,95,162" };
}

// Solar
namespace AccentSolar {
inline constexpr Color accent { "#FFB63D" };
inline constexpr Color accentHover { "#FFC862" };
inline constexpr Color accentPressed { "#EFA227" };
inline constexpr Color accentSoft { "rgba(255,182,61,0.14)" };
inline constexpr Color accentSoftStrong { "rgba(255,182,61,0.24)" };
inline constexpr Color accentRing { "rgba(255,182,61,0.45)" };
inline constexpr Color gradientFrom { "#FFD06A" };
inline constexpr Color gradientTo { "#FF8A3D" };
inline constexpr Color accentRgb { "255,182,61" };
}

// Emerald
namespace AccentEmerald {
inline constexpr Color accent { "#3ED598" };
inline constexpr Color accentHover { "#5FE2AC" };
inline constexpr Color accentPressed { "#2BBF83" };
inline constexpr Color accentSoft { "rgba(62,213,152,0.14)" };
inline constexpr Color accentSoftStrong { "rgba(62,213,152,0.24)" };
inline constexpr Color accentRing { "rgba(62,213,152,0.45)" };
inline constexpr Color gradientFrom { "#5FE2AC" };
inline constexpr Color gradientTo { "#35D6ED" };
inline constexpr Color accentRgb { "62,213,152" };
}

// Ember
namespace AccentEmber {
inline constexpr Color accent { "#FF6B4A" };
inline constexpr Color accentHover { "#FF8468" };
inline constexpr Color accentPressed { "#EA5636" };
inline constexpr Color accentSoft { "rgba(255,107,74,0.14)" };
inline constexpr Color accentSoftStrong { "rgba(255,107,74,0.24)" };
inline constexpr Color accentRing { "rgba(255,107,74,0.45)" };
inline constexpr Color gradientFrom { "#FF9A6A" };
inline constexpr Color gradientTo { "#FF5FA2" };
inline constexpr Color accentRgb { "255,107,74" };
}

// OLED Black - dark
namespace ThemeOled {
inline constexpr Color bgBase { "#000000" };
inline constexpr Color bgSunken { "#000000" };
inline constexpr Color bgSurface { "#07070A" };
inline constexpr Color bgCard { "#0B0B0F" };
inline constexpr Color bgCardHover { "#121218" };
inline constexpr Color bgElevated { "#121218" };
inline constexpr Color bgHover { "#16161D" };
inline constexpr Color bgActive { "#1E1E27" };
inline constexpr Color bgSelected { "#1A1730" };
inline constexpr Color bgSidebar { "#040406" };
inline constexpr Color bgTitlebar { "#000000" };
inline constexpr Color bgInput { "#0A0A0E" };
inline constexpr Color bgScrim { "rgba(0,0,0,0.72)" };
inline constexpr Color bgSkeleton { "#15151C" };
inline constexpr Color borderSubtle { "#17171F" };
inline constexpr Color border { "#20202A" };
inline constexpr Color borderStrong { "#2C2C38" };
inline constexpr Color borderFocus { "#7C6BFF" };
inline constexpr Color textPrimary { "#F3F4F8" };
inline constexpr Color textSecondary { "#A6AAB8" };
inline constexpr Color textMuted { "#6C7080" };
inline constexpr Color textDisabled { "#4A4E5C" };
inline constexpr Color textInverse { "#0A0A0E" };
inline constexpr Color accentContrast { "#FFFFFF" };
inline constexpr Color success { "#3ED598" };
inline constexpr Color successSoft { "rgba(62,213,152,0.14)" };
inline constexpr Color warning { "#FFC24B" };
inline constexpr Color warningSoft { "rgba(255,194,75,0.14)" };
inline constexpr Color danger { "#FF5C6C" };
inline constexpr Color dangerSoft { "rgba(255,92,108,0.14)" };
inline constexpr Color info { "#4CC2FF" };
inline constexpr Color infoSoft { "rgba(76,194,255,0.14)" };
inline constexpr Color shadow1 { "0 1px 2px rgba(0,0,0,0.85)" };
inline constexpr Color shadow2 { "0 8px 24px rgba(0,0,0,0.80)" };
inline constexpr Color shadow3 { "0 18px 48px rgba(0,0,0,0.88)" };
inline constexpr Color glow { "0 0 32px rgba(124,107,255,0.28)" };
inline constexpr Color scrollbarThumb { "#26262F" };
inline constexpr Color scrollbarThumbHover { "#34343F" };
inline constexpr Color star { "#FFFFFF" };
}

// Midnight - dark
namespace ThemeDark {
inline constexpr Color bgBase { "#0C0C11" };
inline constexpr Color bgSunken { "#08080C" };
inline constexpr Color bgSurface { "#111117" };
inline constexpr Color bgCard { "#15151C" };
inline constexpr Color bgCardHover { "#1C1C25" };
inline constexpr Color bgElevated { "#1B1B23" };
inline constexpr Color bgHover { "#22222C" };
inline constexpr Color bgActive { "#2A2A36" };
inline constexpr Color bgSelected { "#241F42" };
inline constexpr Color bgSidebar { "#0A0A0F" };
inline constexpr Color bgTitlebar { "#0A0A0F" };
inline constexpr Color bgInput { "#101017" };
inline constexpr Color bgScrim { "rgba(6,6,10,0.66)" };
inline constexpr Color bgSkeleton { "#1E1E27" };
inline constexpr Color borderSubtle { "#1E1E27" };
inline constexpr Color border { "#282833" };
inline constexpr Color borderStrong { "#35353F" };
inline constexpr Color borderFocus { "#7C6BFF" };
inline constexpr Color textPrimary { "#EEF0F6" };
inline constexpr Color textSecondary { "#A2A6B5" };
inline constexpr Color textMuted { "#71758A" };
inline constexpr Color textDisabled { "#4E5260" };
inline constexpr Color textInverse { "#0C0C11" };
inline constexpr Color accentContrast { "#FFFFFF" };
inline constexpr Color success { "#3ED598" };
inline constexpr Color successSoft { "rgba(62,213,152,0.16)" };
inline constexpr Color warning { "#FFC24B" };
inline constexpr Color warningSoft { "rgba(255,194,75,0.16)" };
inline constexpr Color danger { "#FF5C6C" };
inline constexpr Color dangerSoft { "rgba(255,92,108,0.16)" };
inline constexpr Color info { "#4CC2FF" };
inline constexpr Color infoSoft { "rgba(76,194,255,0.16)" };
inline constexpr Color shadow1 { "0 1px 2px rgba(0,0,0,0.5)" };
inline constexpr Color shadow2 { "0 10px 28px rgba(0,0,0,0.45)" };
inline constexpr Color shadow3 { "0 20px 52px rgba(0,0,0,0.55)" };
inline constexpr Color glow { "0 0 32px rgba(124,107,255,0.22)" };
inline constexpr Color scrollbarThumb { "#2C2C38" };
inline constexpr Color scrollbarThumbHover { "#3A3A48" };
inline constexpr Color star { "#FFFFFF" };
}

// Navy - dark
namespace ThemeNavy {
inline constexpr Color bgBase { "#070D1A" };
inline constexpr Color bgSunken { "#04070F" };
inline constexpr Color bgSurface { "#0A1322" };
inline constexpr Color bgCard { "#0D1730" };
inline constexpr Color bgCardHover { "#122040" };
inline constexpr Color bgElevated { "#13203C" };
inline constexpr Color bgHover { "#14223E" };
inline constexpr Color bgActive { "#1B2E50" };
inline constexpr Color bgSelected { "#1C2A4E" };
inline constexpr Color bgSidebar { "#060B14" };
inline constexpr Color bgTitlebar { "#050A13" };
inline constexpr Color bgInput { "#0A1424" };
inline constexpr Color bgScrim { "rgba(6,6,10,0.66)" };
inline constexpr Color bgSkeleton { "#111B30" };
inline constexpr Color borderSubtle { "#16233C" };
inline constexpr Color border { "#1E2E4E" };
inline constexpr Color borderStrong { "#2A405F" };
inline constexpr Color borderFocus { "#7C6BFF" };
inline constexpr Color textPrimary { "#E9EEF9" };
inline constexpr Color textSecondary { "#9FAAC6" };
inline constexpr Color textMuted { "#68728E" };
inline constexpr Color textDisabled { "#4A536C" };
inline constexpr Color textInverse { "#0C0C11" };
inline constexpr Color accentContrast { "#FFFFFF" };
inline constexpr Color success { "#3ED598" };
inline constexpr Color successSoft { "rgba(62,213,152,0.16)" };
inline constexpr Color warning { "#FFC24B" };
inline constexpr Color warningSoft { "rgba(255,194,75,0.16)" };
inline constexpr Color danger { "#FF5C6C" };
inline constexpr Color dangerSoft { "rgba(255,92,108,0.16)" };
inline constexpr Color info { "#4CC2FF" };
inline constexpr Color infoSoft { "rgba(76,194,255,0.16)" };
inline constexpr Color shadow1 { "0 1px 2px rgba(0,0,0,0.5)" };
inline constexpr Color shadow2 { "0 10px 28px rgba(0,0,0,0.45)" };
inline constexpr Color shadow3 { "0 20px 52px rgba(0,0,0,0.55)" };
inline constexpr Color glow { "0 0 32px rgba(124,107,255,0.22)" };
inline constexpr Color scrollbarThumb { "#2C2C38" };
inline constexpr Color scrollbarThumbHover { "#3A3A48" };
inline constexpr Color star { "#FFFFFF" };
}

// Daylight - light
namespace ThemeLight {
inline constexpr Color bgBase { "#F2F4FA" };
inline constexpr Color bgSunken { "#E9ECF5" };
inline constexpr Color bgSurface { "#FFFFFF" };
inline constexpr Color bgCard { "#FFFFFF" };
inline constexpr Color bgCardHover { "#F5F7FD" };
inline constexpr Color bgElevated { "#FFFFFF" };
inline constexpr Color bgHover { "#EDF0F9" };
inline constexpr Color bgActive { "#E3E8F6" };
inline constexpr Color bgSelected { "#E8E5FF" };
inline constexpr Color bgSidebar { "#FFFFFF" };
inline constexpr Color bgTitlebar { "#FFFFFF" };
inline constexpr Color bgInput { "#F7F8FC" };
inline constexpr Color bgScrim { "rgba(23,28,45,0.34)" };
inline constexpr Color bgSkeleton { "#E7EAF3" };
inline constexpr Color borderSubtle { "#E6E9F2" };
inline constexpr Color border { "#D8DCE9" };
inline constexpr Color borderStrong { "#C2C8DA" };
inline constexpr Color borderFocus { "#6450F0" };
inline constexpr Color textPrimary { "#0F131C" };
inline constexpr Color textSecondary { "#4B5163" };
inline constexpr Color textMuted { "#767C90" };
inline constexpr Color textDisabled { "#A7ADBE" };
inline constexpr Color textInverse { "#FFFFFF" };
inline constexpr Color accentContrast { "#FFFFFF" };
inline constexpr Color success { "#17996A" };
inline constexpr Color successSoft { "rgba(23,153,106,0.12)" };
inline constexpr Color warning { "#B47500" };
inline constexpr Color warningSoft { "rgba(180,117,0,0.12)" };
inline constexpr Color danger { "#D93A4C" };
inline constexpr Color dangerSoft { "rgba(217,58,76,0.12)" };
inline constexpr Color info { "#1273C4" };
inline constexpr Color infoSoft { "rgba(18,115,196,0.12)" };
inline constexpr Color shadow1 { "0 1px 2px rgba(19,26,48,0.08)" };
inline constexpr Color shadow2 { "0 10px 24px rgba(19,26,48,0.10)" };
inline constexpr Color shadow3 { "0 20px 44px rgba(19,26,48,0.14)" };
inline constexpr Color glow { "0 0 32px rgba(100,80,240,0.16)" };
inline constexpr Color scrollbarThumb { "#C9CEDD" };
inline constexpr Color scrollbarThumbHover { "#B3B9CB" };
inline constexpr Color star { "#0F131C" };
}

// White - light
namespace ThemeWhite {
inline constexpr Color bgBase { "#FFFFFF" };
inline constexpr Color bgSunken { "#F4F6FA" };
inline constexpr Color bgSurface { "#FBFCFE" };
inline constexpr Color bgCard { "#FFFFFF" };
inline constexpr Color bgCardHover { "#F3F5FA" };
inline constexpr Color bgElevated { "#FFFFFF" };
inline constexpr Color bgHover { "#F0F2F8" };
inline constexpr Color bgActive { "#E7EAF4" };
inline constexpr Color bgSelected { "#ECE9FF" };
inline constexpr Color bgSidebar { "#F7F8FC" };
inline constexpr Color bgTitlebar { "#FFFFFF" };
inline constexpr Color bgInput { "#FFFFFF" };
inline constexpr Color bgScrim { "rgba(23,28,45,0.34)" };
inline constexpr Color bgSkeleton { "#EDEFF6" };
inline constexpr Color borderSubtle { "#E6E9F2" };
inline constexpr Color border { "#D8DCE9" };
inline constexpr Color borderStrong { "#C2C8DA" };
inline constexpr Color borderFocus { "#6450F0" };
inline constexpr Color textPrimary { "#0F131C" };
inline constexpr Color textSecondary { "#4B5163" };
inline constexpr Color textMuted { "#767C90" };
inline constexpr Color textDisabled { "#A7ADBE" };
inline constexpr Color textInverse { "#FFFFFF" };
inline constexpr Color accentContrast { "#FFFFFF" };
inline constexpr Color success { "#17996A" };
inline constexpr Color successSoft { "rgba(23,153,106,0.12)" };
inline constexpr Color warning { "#B47500" };
inline constexpr Color warningSoft { "rgba(180,117,0,0.12)" };
inline constexpr Color danger { "#D93A4C" };
inline constexpr Color dangerSoft { "rgba(217,58,76,0.12)" };
inline constexpr Color info { "#1273C4" };
inline constexpr Color infoSoft { "rgba(18,115,196,0.12)" };
inline constexpr Color shadow1 { "0 1px 2px rgba(19,26,48,0.08)" };
inline constexpr Color shadow2 { "0 10px 24px rgba(19,26,48,0.10)" };
inline constexpr Color shadow3 { "0 20px 44px rgba(19,26,48,0.14)" };
inline constexpr Color glow { "0 0 32px rgba(100,80,240,0.16)" };
inline constexpr Color scrollbarThumb { "#C9CEDD" };
inline constexpr Color scrollbarThumbHover { "#B3B9CB" };
inline constexpr Color star { "#0F131C" };
}

enum class ThemeId {
    Oled,
    Dark,
    Navy,
    Light,
    White,
};

enum class AccentId {
    Starlight,
    Aurora,
    Nova,
    Solar,
    Emerald,
    Ember,
};

struct ThemeInfo { ThemeId id; const char *tokenKey; const char *label; bool isDark; };
struct AccentInfo { AccentId id; const char *tokenKey; const char *label; };

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
inline constexpr ThemeInfo kThemes[] = {
    { ThemeId::Oled, "oled", "OLED Black", true },
    { ThemeId::Dark, "dark", "Midnight", true },
    { ThemeId::Navy, "navy", "Navy", true },
    { ThemeId::Light, "light", "Daylight", false },
    { ThemeId::White, "white", "White", false },
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
inline constexpr AccentInfo kAccents[] = {
    { AccentId::Starlight, "starlight", "Starlight" },
    { AccentId::Aurora, "aurora", "Aurora" },
    { AccentId::Nova, "nova", "Nova" },
    { AccentId::Solar, "solar", "Solar" },
    { AccentId::Emerald, "emerald", "Emerald" },
    { AccentId::Ember, "ember", "Ember" },
};

}  // namespace StarTheme
