# Star Client

**Reach for the stars.**

A fast, lightweight Minecraft launcher. Star Client runs official Microsoft
accounts and offline profiles side by side, keeps its own memory footprint
small, and gives you real control over the JVM instead of a single opaque
"memory" box.

---

## Status

| Piece | State |
| --- | --- |
| Design system (tokens, themes, accents) | done |
| Brand (four-pointed star, icons, raster set) | done |
| UI prototype (`prototype/`) | done — 23 screens, no console errors |
| Qt Widgets port harness (`prototype/qt/`) | done — renders the same design in real Qt |
| Launcher fork integration | **next** |
| Windows portable build | CI workflow in progress |
| macOS build | after Windows |

---

## What makes it different

### Real memory control
A slider with a safe band computed from your physical RAM and mod count, live
validation, quick presets from 2 GB to 16 GB, an auto-tune button, and warnings
when a value is too small to work or too large to help. The launcher tells you
*why* a number is bad instead of letting you find out at launch.

### Garbage collector presets, not a JVM string
Pick G1GC, ZGC, Generational ZGC, Shenandoah, Serial or Parallel. Star Client
applies the matching flags, shows you the exact resolved command line, strips
conflicting flags from your custom arguments, and can reset to the tuned
default in one click.

### A launcher that stays small
Lazy icon decoding, a bounded thumbnail cache, caches released when Minecraft
starts, no bundled browser engine, and background work suspended during
gameplay. The Star Start page shows the current footprint against a typical
launcher.

### Star Menu
An optional set of quality-of-life presets (performance renderer, zoom and
waypoints, performance HUD, and more) that Star Client installs and manages.
Switch the whole set on or off **per instance**, because heavily modded packs
conflict with injected mods. Conflicting presets are disabled automatically
and the launcher names the file responsible. Nothing is ever deleted.

### OLED black, properly
Three themes — **OLED Black** (true `#000000` panels), **Midnight** (soft dark)
and **Daylight** (light) — plus six accent presets. A four-pointed star mark
that is white on dark themes and ink on the light theme.

---

## Layout

```
theme/tokens.json          single source of truth for colours, spacing, fonts
tools/gen_theme.py         tokens -> CSS variables, Qt stylesheets, C++ palettes
tools/gen_logo.py          generates the star mark at every size
src/theme/                 Qt stylesheet template, icon resources, generated QSS
prototype/                 the browser-drawn UI prototype (design reference)
prototype/qt/              the same design in real Qt Widgets (port harness)
branding/                  SVG, PNG and .ico assets
docs/                      design and build documentation
```

The theme pipeline is the important bit: one token file generates the web
prototype's CSS **and** the launcher's Qt stylesheet **and** the C++ colour
palettes. Change a colour in `theme/tokens.json`, run one command, and both
surfaces move together.

```bash
python3 tools/gen_theme.py      # regenerate CSS + QSS + C++ palette
python3 tools/gen_logo.py       # regenerate the star assets
python3 tools/gen_theme.py --check   # fail if generated files are stale
```

---

## Try the prototype

```bash
# browser prototype (design reference)
python3 -m http.server 8080
# then open http://localhost:8080/prototype/

# Qt Widgets harness (needs PySide6)
python3 prototype/qt/starclient_qt.py
QT_QPA_PLATFORM=offscreen python3 prototype/qt/starclient_qt.py --shots out/

# screenshot + smoke test the prototype (needs the tools/ npm deps)
node tools/screenshot.js --out shots
```

---

## Building the launcher

See [`docs/BUILDING.md`](docs/BUILDING.md). Windows is the primary target;
macOS follows. Release archives are **portable** — unzip and run, no installer.

---

## Licence

Star Client is free software released under the **GNU General Public License
v3.0**. See [`LICENSE`](LICENSE) and [`COPYING.md`](COPYING.md).

Attribution for the projects this fork is built on lives in
[`CREDITS.md`](CREDITS.md).

Minecraft is a trademark of Mojang Synergies AB. Star Client is not affiliated
with, endorsed by, or sponsored by Mojang or Microsoft.
