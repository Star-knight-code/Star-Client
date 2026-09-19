# Credits and attribution

Star Client is a rebranded, heavily modified fork. It is free software under
the **GNU General Public License v3.0** (see `LICENSE`), and it exists because
of the work of the projects below.

This file is the canonical attribution record. It is deliberately not shouted
about in the interface — the in-app About page links here, and the binary
ships it.

---

## Upstream base

### Prism Launcher
Star Client's launcher core (instance management, version resolution, asset
downloading, the launch pipeline, Java discovery) descends from Prism Launcher.

    Prism Launcher - Minecraft Launcher
    Copyright (C) 2022-2026 Prism Launcher Contributors
    Licensed under the GNU General Public License v3.0

### PolyMC and MultiMC
Prism Launcher itself incorporates work from PolyMC and MultiMC.

    PolyMC - Minecraft Launcher
    Copyright (C) 2021-2022 PolyMC Contributors

    MultiMC - Minecraft Launcher
    Copyright 2013-2021 MultiMC Contributors
    Licensed under the Apache License, Version 2.0

Full notices for both are in `COPYING.md`.

### Prism Launcher (cracked / offline-account build)
The offline-account handling that Star Client's account layer builds on comes
from the community Prism Launcher fork by Diegiwg.

    PrismLauncher-Cracked
    Copyright (C) Diegiwg and contributors
    Licensed under the GNU General Public License v3.0
    https://github.com/Diegiwg/PrismLauncher-Cracked

---

## Code borrowed with modification

Where a component was borrowed, it was rewritten and rebranded for Star
Client's architecture. Nothing here is a verbatim copy of a UI or an asset
from another launcher.

### OneLauncher
Ideas and implementation patterns for the account flow, the quick-start
experience and the update/self-management model were adapted from OneLauncher.
Star Client's versions of these are independent C++/Qt implementations.

    OneLauncher
    Copyright (C) Polyfrost and contributors
    Licensed under the GNU General Public License v3.0
    https://github.com/Polyfrost/OneLauncher

### OneConfig
Configuration-screen ergonomics — searchable settings, live-preview controls,
reset-to-default affordances — were informed by OneConfig. Star Client's
settings UI is its own Qt implementation against the design tokens in
`theme/tokens.json`; no OneConfig code or assets are redistributed.

    OneConfig
    Copyright (C) Polyfrost and contributors
    LGPL-3.0 (see the upstream project for the exact terms)
    https://github.com/Polyfrost/OneConfig

---

## Design and assets

The four-pointed star mark, the OLED dark / Midnight / Daylight themes, the
accent presets, every icon and the UI layout are original to Star Client.

The star mark is generated, not hand-drawn: `tools/gen_logo.py` builds it from
four circular arcs and exports every size. The themes are generated from
`theme/tokens.json` by `tools/gen_theme.py`.

---

## Third-party components

The launcher links against Qt 6, zlib, libarchive, and the CMake libraries listed
in `launcher/CMakeLists.txt` and `libraries/`. Their licences are unchanged and
are reproduced by the packaging scripts where required. Minecraft itself is not
included and is not modified — Star Client only downloads and launches it.

Minecraft is a trademark of Mojang Synergies AB. Star Client is not affiliated
with, endorsed by, or sponsored by Mojang or Microsoft.
