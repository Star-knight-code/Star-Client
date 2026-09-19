#!/usr/bin/env python3
"""
Star Client theme generator.

One source of truth (theme/tokens.json) -> every target we ship:

  * prototype/css/tokens.css            web preview variables (CSS custom props)
  * src/theme/generated/starclient-*.qss  Qt Widgets stylesheet, one per theme
  * src/theme/generated/StarThemePalette.h  Qt/C++ constexpr palettes

The Qt stylesheet can't use CSS variables, so we resolve every {{placeholder}}
to a literal at generation time. Placeholders:

    {{c.bg-card}}     theme colour      {{a.accent}}   accent colour
    {{r.lg}}          radius            {{s.5}}        spacing step
    {{f.size-md}}     font              {{m.fast}}     motion duration
    {{l.sidebar-width}} layout size

Usage:
    python3 tools/gen_theme.py            # write all outputs
    python3 tools/gen_theme.py --check    # verify outputs are up to date (CI)
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOKENS_PATH = ROOT / "theme" / "tokens.json"
TEMPLATE_PATH = ROOT / "src" / "theme" / "starclient.qss.tmpl"
CSS_OUT = ROOT / "prototype" / "css" / "tokens.css"
QSS_OUT_DIR = ROOT / "src" / "theme" / "generated"
QSS_OUT_STEM = "starclient"
CPP_OUT = QSS_OUT_DIR / "StarThemePalette.h"

PLACEHOLDER = re.compile(r"\{\{\s*([a-zA-Z0-9_.-]+)\s*\}\}")


# --------------------------------------------------------------------------- #
# token access
# --------------------------------------------------------------------------- #
def load_tokens() -> dict:
    with TOKENS_PATH.open(encoding="utf-8") as fh:
        return json.load(fh)


def build_lookup(tokens: dict, theme: dict, accent: dict) -> dict[str, dict]:
    """Flatten every token group into `prefix.key -> value`."""
    return {
        "c": theme["color"],
        "color": theme["color"],
        "a": accent,
        "accent": accent,
        "r": tokens["radii"],
        "radius": tokens["radii"],
        "s": tokens["space"],
        "space": tokens["space"],
        "f": tokens["font"],
        "font": tokens["font"],
        "m": tokens["motion"],
        "motion": tokens["motion"],
        "l": tokens["layout"],
        "layout": tokens["layout"],
    }


def resolve(template: str, lookup: dict[str, dict]) -> str:
    missing: set[str] = set()

    def sub(match: re.Match[str]) -> str:
        raw = match.group(1)
        group, _, key = raw.partition(".")
        value = lookup.get(group, {}).get(key)
        if value is None:
            missing.add(raw)
            return match.group(0)
        return str(value)

    out = PLACEHOLDER.sub(sub, template)
    if missing:
        raise SystemExit(
            "Unknown token placeholder(s): " + ", ".join(sorted(missing))
        )
    return out


# --------------------------------------------------------------------------- #
# target 1: web preview CSS
# --------------------------------------------------------------------------- #
def render_css(tokens: dict) -> str:
    lines: list[str] = []
    add = lines.append

    add("/* GENERATED FILE - do not edit. Source: theme/tokens.json */")
    add("/* Regenerate with: python3 tools/gen_theme.py */")
    add("")

    # --- static scales ----------------------------------------------------- #
    add(":root {")
    for name, value in tokens["font"].items():
        add(f"  --sc-font-{name}: {value};")
    add("")
    for name, value in tokens["radii"].items():
        add(f"  --sc-radius-{name}: {value};")
    add("")
    for name, value in tokens["space"].items():
        add(f"  --sc-space-{name}: {value};")
    add("")
    for name, value in tokens["motion"].items():
        add(f"  --sc-motion-{name}: {value};")
    add("")
    for name, value in tokens["layout"].items():
        add(f"  --sc-{name}: {value};")
    add("}")

    # --- accents ----------------------------------------------------------- #
    add("")
    add("/* accent presets - first one is the default, JS writes data-accent */")
    for key, accent in tokens["accents"].items():
        add(f':root[data-accent="{key}"] {{')
        for name, value in accent.items():
            if name == "label":
                continue
            add(f"  --sc-{name}: {value};")
        add("}")
    default_accent = next(iter(tokens["accents"]))
    add('/* fallback when no data-accent is set */')
    add(":root {")
    for name, value in tokens["accents"][default_accent].items():
        if name == "label":
            continue
        add(f"  --sc-{name}: {value};")
    add("}")

    # --- themes ------------------------------------------------------------ #
    for key, theme in tokens["themes"].items():
        add("")
        add(f'/* {theme["label"]} ({theme["appearance"]}) */')
        add(f':root[data-theme="{key}"] {{')
        add(f'  color-scheme: {theme["appearance"]};')
        for name, value in theme["color"].items():
            add(f"  --sc-{name}: {value};")
        add("}")

    # --- rgb triples, for rgba() glows that follow the accent -------------- #
    add("")
    add("/* raw channel triples: rgba(var(--sc-accent-rgb), .3) */")
    for key, accent in tokens["accents"].items():
        add(f':root[data-accent="{key}"] {{ --sc-accent-rgb: {accent["accent-rgb"]}; }}')
    add(":root { --sc-accent-rgb: "
        f'{tokens["accents"][default_accent]["accent-rgb"]}; }}')

    # --- dark/light shorthands used by the toggle -------------------------- #
    dark = next(k for k, t in tokens["themes"].items() if t["appearance"] == "dark")
    light = next(k for k, t in tokens["themes"].items() if t["appearance"] == "light")
    add("")
    add(f':root[data-theme="system"] {{ color-scheme: light dark; }}')
    add("@media (prefers-color-scheme: dark) {")
    add(f'  :root[data-theme="system"] {{')
    for name, value in tokens["themes"][dark]["color"].items():
        add(f"    --sc-{name}: {value};")
    add("  }")
    add("}")
    add("@media (prefers-color-scheme: light) {")
    add(f'  :root[data-theme="system"] {{')
    for name, value in tokens["themes"][light]["color"].items():
        add(f"    --sc-{name}: {value};")
    add("  }")
    add("}")
    add("")
    return "\n".join(lines)


# --------------------------------------------------------------------------- #
# target 2: Qt stylesheets
# --------------------------------------------------------------------------- #
def render_qss(template: str, tokens: dict, theme_key: str, accent_key: str) -> str:
    theme = tokens["themes"][theme_key]
    accent = tokens["accents"][accent_key]
    lookup = build_lookup(tokens, theme, accent)
    body = resolve(template, lookup)
    # Drop the template's own documentation header so the generated file
    # starts with real rules and stays diff-friendly.
    end = body.find("*/")
    if body.lstrip().startswith("/*") and end != -1:
        body = body[end + 2 :].lstrip("\n")
    body += "\n"
    header = (
        f"/* GENERATED FILE - do not edit.\n"
        f" * Star Client Qt stylesheet\n"
        f" * theme  : {theme_key} ({theme['label']}, {theme['appearance']})\n"
        f" * accent : {accent_key} ({accent['label']})\n"
        f" * source : theme/tokens.json + src/theme/starclient.qss.tmpl\n"
        f" */\n"
    )
    return header + body


# --------------------------------------------------------------------------- #
# target 3: C++ palette header
# --------------------------------------------------------------------------- #
CPP_IDENT = re.compile(r"[^0-9a-zA-Z]+")


def cpp_ident(name: str, prefix: str) -> str:
    parts = [p for p in CPP_IDENT.split(name) if p]
    camel = parts[0] + "".join(p.capitalize() for p in parts[1:])
    return f"{prefix}{camel}"


def render_cpp(tokens: dict) -> str:
    out: list[str] = []
    add = out.append
    add("// GENERATED FILE - do not edit. Source: theme/tokens.json")
    add("// Regenerate with: python3 tools/gen_theme.py")
    add("#pragma once")
    add("")
    add("#include <QColor>")
    add("#include <QString>")
    add("")
    add("namespace StarTheme {")
    add("")

    # colour helper: store as string (QSS-native) and lazily as QColor
    add("struct Color {")
    add("    const char *hex;                 // '#rrggbb' or 'rgba(r,g,b,a)'")
    add("    QColor qcolor() const;           // parsed for QPalette painting")
    add("};")
    add("")

    for key, accent in tokens["accents"].items():
        add(f"// {accent['label']}")
        add(f"namespace Accent{cpp_ident(key, '').capitalize()} {{")
        for name, value in accent.items():
            if name == "label":
                continue
            add(f'inline constexpr Color {cpp_ident(name, "")} {{ "{value}" }};')
        add("}")
        add("")

    for key, theme in tokens["themes"].items():
        add(f"// {theme['label']} - {theme['appearance']}")
        add(f"namespace Theme{cpp_ident(key, '').capitalize()} {{")
        for name, value in theme["color"].items():
            add(f'inline constexpr Color {cpp_ident(name, "")} {{ "{value}" }};')
        add("}")
        add("")

    # enum + registry consumed by StarTheme.cpp
    add("enum class ThemeId {")
    for key in tokens["themes"]:
        add(f"    {cpp_ident(key, '').capitalize()},")
    add("};")
    add("")
    add("enum class AccentId {")
    for key in tokens["accents"]:
        add(f"    {cpp_ident(key, '').capitalize()},")
    add("};")
    add("")
    add("struct ThemeInfo { ThemeId id; const char *tokenKey; const char *label; bool isDark; };")
    add("struct AccentInfo { AccentId id; const char *tokenKey; const char *label; };")
    add("")
    add("// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)")
    add("inline constexpr ThemeInfo kThemes[] = {")
    for key, theme in tokens["themes"].items():
        dark = "true" if theme["appearance"] == "dark" else "false"
        add(
            f'    {{ ThemeId::{cpp_ident(key, "").capitalize()}, "{key}", '
            f'"{theme["label"]}", {dark} }},'
        )
    add("};")
    add("")
    add("// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)")
    add("inline constexpr AccentInfo kAccents[] = {")
    for key, accent in tokens["accents"].items():
        add(
            f'    {{ AccentId::{cpp_ident(key, "").capitalize()}, "{key}", '
            f'"{accent["label"]}" }},'
        )
    add("};")
    add("")
    add("}  // namespace StarTheme")
    add("")
    return "\n".join(out)


# --------------------------------------------------------------------------- #
# main
# --------------------------------------------------------------------------- #
def main() -> int:
    parser = argparse.ArgumentParser(description="Star Client theme generator")
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if any generated file is stale instead of writing it",
    )
    args = parser.parse_args()

    tokens = load_tokens()
    template = TEMPLATE_PATH.read_text(encoding="utf-8")

    outputs: dict[Path, str] = {}
    outputs[CSS_OUT] = render_css(tokens)
    outputs[CPP_OUT] = render_cpp(tokens)

    default_accent = next(iter(tokens["accents"]))
    for theme_key in tokens["themes"]:
        path = QSS_OUT_DIR / f"{QSS_OUT_STEM}-{theme_key}.qss"
        outputs[path] = render_qss(template, tokens, theme_key, default_accent)

    stale: list[Path] = []
    for path, content in outputs.items():
        if args.check:
            current = path.read_text(encoding="utf-8") if path.exists() else None
            if current != content:
                stale.append(path)
            continue
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"  wrote {path.relative_to(ROOT)}")

    if stale:
        print("Stale generated theme files (run tools/gen_theme.py):", file=sys.stderr)
        for path in stale:
            print(f"  - {path.relative_to(ROOT)}", file=sys.stderr)
        return 1

    if args.check:
        print("Theme outputs are up to date.")
    else:
        print(f"Generated {len(outputs)} files from theme/tokens.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
