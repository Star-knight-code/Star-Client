# Building Star Client

Windows x64 is the primary target. macOS follows. Release archives are
**portable**: unzip and run, no installer.

---

## Requirements

| | |
| --- | --- |
| CMake | 3.24 or newer |
| Compiler | MSVC 2022 (Windows), Clang 15+ (macOS), GCC 12+ (Linux) |
| Qt | 6.6 or newer — Core, Gui, Widgets, Network, Concurrent |
| Java | 17+ to *test* launches (the launcher itself does not need Java to build) |
| vcpkg | for zlib, libarchive, cmark, tomlplusplus, libqrencode |
| Ninja | recommended generator |

Qt modules: `qtbase`, `qtimageformats`, plus Qt's SVG plugin (the theme
icons are SVG).

---

## Windows

```powershell
# 1. toolchain
choco install ninja -y
git clone https://github.com/microsoft/vcpkg $env:USERPROFILE\vcpkg
$env:VCPKG_INSTALLATION_ROOT = "$env:USERPROFILE\vcpkg"

# 2. Qt (any 6.6+ msvc2022_64 build; aqtinstall is what CI uses)
pip install aqtinstall
aqt install-qt windows desktop 6.8.1 win64_msvc2022_64 --modules qtimageformats

# 3. configure + build, from a "x64 Native Tools" prompt
cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT\scripts\buildsystems\vcpkg.cmake" `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.8.1\msvc2022_64"
cmake --build build --parallel

# 4. portable archive
./scripts/package-portable.ps1 `
  -BuildDir build `
  -QtDir C:\Qt\6.8.1\msvc2022_64 `
  -Version 21.1.0
# -> dist/StarClient-21.1.0-windows-x64-portable.zip
```

CI does exactly this in [`.github/workflows/windows-portable.yml`](../.github/workflows/windows-portable.yml).

### What "portable" means

The archive contains a `portable.txt` marker next to `StarClient.exe`. When
that file is present the launcher keeps **all** state — instances, settings,
downloads, Java runtimes, logs — inside `.\data` beside the executable, and
writes nothing to `%APPDATA%` or the registry. Delete the folder and every
trace of the launcher is gone.

Without the marker it behaves like a normal install.

Archive layout:

```
StarClient.exe              main launcher
StarClient-console.exe      with an attached console, for log output
StarClient_updater.exe      self-updater (when built)
Qt6*.dll                    Qt runtime, fetched by windeployqt
plugins/                    platform, imageformats, styles, tls
qt.conf                     points Qt at ./plugins
portable.txt                marks this as a portable install
licenses/                   GPL-3.0, COPYING.md, CREDITS.md, Qt licences
README.txt                  first-run notes
```

---

## macOS

Unsigned local build:

```bash
brew install cmake ninja qt@6
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --parallel
```

A universal (arm64 + x86_64) build needs `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`
and universal Qt. The `.app` bundle is produced by CMake; `macdeployqt` copies
the Qt frameworks in. Running on another Mac requires signing and notarising —
the workflow for that lands with the macOS packaging script.

The macOS release will ship a `.dmg` alongside the portable `.tar.gz`.

---

## Linux

Not a release target yet, but the build works for development:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

---

## Theme and asset generation

Both generators are pure Python 3 with no dependencies.

```bash
python3 tools/gen_theme.py            # tokens.json -> CSS, QSS, C++ palettes
python3 tools/gen_theme.py --check    # verify committed output is current
python3 tools/gen_logo.py             # regenerate the star assets (.ico needs ImageMagick or node+resvg)
```

`--check` belongs in CI: it fails the build when someone edits a generated
file by hand or forgets to regenerate after touching `theme/tokens.json`.

---

## Tests

```bash
ctest --test-dir build --output-on-failure     # C++ unit tests

# design regression: renders the prototype, fails on console errors
npm --prefix tools install
node tools/screenshot.js --out shots
```

---

## Troubleshooting

**`Could not find a package configuration file provided by "Qt6"`**
Point `CMAKE_PREFIX_PATH` at the Qt *prefix* (`C:\Qt\6.8.1\msvc2022_64`), not
at `bin` or `lib`.

**`vcpkg` takes forever on a clean machine**
That is the first build only. CI caches the binary cache; locally, set
`VCPKG_BINARY_SOURCES` or keep the vcpkg checkout between builds.

**App starts but has no icons / unstyled controls**
The `.qrc` resources were not compiled in. Check that `src/theme/starclient.qrc`
is in the target sources and that the QSS is loaded before the first window
is shown.

**Blank window on an older GPU**
`QT_OPENGL=software` or `-DCMAKE_BUILD_TYPE=Release` with the software
rasterizer. Report the driver version if it persists.
