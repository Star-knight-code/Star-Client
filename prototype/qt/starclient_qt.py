#!/usr/bin/env python3
"""
Star Client - Qt Widgets prototype.

This is the answer to "can Qt Widgets + QSS really do this UI?" - it is the
same design as prototype/index.html, built with real Qt Widgets, styled by the
QSS that tools/gen_theme.py generates from theme/tokens.json.

It exists for three reasons:
  1. proof that the design ports to Qt Widgets without compromise;
  2. a rehearsal of the widget structure that goes into the launcher fork;
  3. a theme/regression harness - if the generated QSS is broken, the
     screenshots this produces make it obvious.

Run (Linux/macOS, a real display or offscreen):
    python3 prototype/qt/starclient_qt.py
    QT_QPA_PLATFORM=offscreen python3 prototype/qt/starclient_qt.py --shots out/

Requires PySide6. The launcher itself will use C++ instead; this file is a
design harness, not shipped code.
"""

from __future__ import annotations

import argparse
import math
import os
import sys
from pathlib import Path

from PySide6.QtCore import QPointF, QRectF, QSize, Qt, Signal
from PySide6.QtGui import (
    QBrush,
    QColor,
    QFont,
    QIcon,
    QLinearGradient,
    QPainter,
    QPainterPath,
    QPen,
    QPixmap,
)
from PySide6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QRadioButton,
    QScrollArea,
    QSizePolicy,
    QSlider,
    QStackedWidget,
    QVBoxLayout,
    QWidget,
)

ROOT = Path(__file__).resolve().parents[2]
TOKENS_QSS_DIR = ROOT / "src" / "theme" / "generated"
ICON_DIR = ROOT / "src" / "theme" / "icons"

# --------------------------------------------------------------------------- #
# Brand mark: the four-pointed star, same geometry as tools/gen_logo.py
# --------------------------------------------------------------------------- #
STAR_TIP = 112.0
STAR_BOW = 0.30


def star_path(rect: QRectF) -> QPainterPath:
    """Four-pointed star with concave edges, fitted into `rect`."""
    cx, cy = rect.center().x(), rect.center().y()
    scale = min(rect.width(), rect.height()) / 256.0 * 1.06
    tip = STAR_TIP * scale * 0.94

    def cusp(deg: float) -> QPointF:
        a = math.radians(deg)
        return QPointF(cx + tip * math.cos(a), cy + tip * math.sin(a))

    # control point pulled toward the centre produces the concave edge;
    # QPainterPath.quadTo gives the same result as the SVG Q commands
    def bow(p0: QPointF, p1: QPointF) -> QPointF:
        mid = (p0 + p1) / 2.0
        vx, vy = cx - mid.x(), cy - mid.y()
        length = math.hypot(vx, vy) or 1.0
        return QPointF(mid.x() + vx / length * length * STAR_BOW * 2.0,
                       mid.y() + vy / length * length * STAR_BOW * 2.0)

    pts = [cusp(-90), cusp(0), cusp(90), cusp(180)]
    path = QPainterPath(pts[0])
    for i in range(4):
        p0, p1 = pts[i], pts[(i + 1) % 4]
        path.quadTo(bow(p0, p1), p1)
    path.closeSubpath()
    return path


def star_pixmap(size: int, color: QColor | None = None, gradient: bool = False) -> QPixmap:
    """Renders the mark to a transparent pixmap for use as a window/QIcon."""
    dpr = 2
    pm = QPixmap(size * dpr, size * dpr)
    pm.setDevicePixelRatio(dpr)
    pm.fill(Qt.transparent)
    p = QPainter(pm)
    p.setRenderHint(QPainter.Antialiasing)
    rect = QRectF(0, 0, size, size)
    if gradient:
        g = QLinearGradient(rect.topLeft(), rect.bottomRight())
        g.setColorAt(0.0, QColor("#8B7BFF"))
        g.setColorAt(0.55, QColor("#6450F0"))
        g.setColorAt(1.0, QColor("#4FC3F7"))
        p.setBrush(QBrush(g))
        p.setPen(Qt.NoPen)
    else:
        p.setBrush(color or QColor("#FFFFFF"))
        p.setPen(Qt.NoPen)
    p.drawPath(star_path(rect))
    p.end()
    return pm


class StarButton(QPushButton):
    """Launch button painted with a real QLinearGradient.

    Qt stylesheets cannot express linear-gradient(), so the hero action is a
    small custom widget. This is the pattern the launcher uses wherever a
    gradient matters; everything else stays pure QSS.
    """

    def __init__(self, text: str, accent: str = "#7C6BFF", parent=None):
        super().__init__(text, parent)
        self.setCursor(Qt.PointingHandCursor)
        self.setMinimumHeight(46)
        self.setMinimumWidth(230)
        f = self.font()
        f.setPointSize(12)
        f.setBold(True)
        self.setFont(f)
        self._accent = QColor(accent)
        self.setStyleSheet("QPushButton { border: 0; background: transparent; color: #FFFFFF; padding: 12px 26px; }")

    def set_accent(self, colour: str):
        self._accent = QColor(colour)
        self.update()

    def paintEvent(self, event):  # noqa: N802
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(0.5, 0.5, -0.5, -0.5)

        g = QLinearGradient(r.topLeft(), r.bottomRight())
        g.setColorAt(0.0, self._accent.lighter(112))
        g.setColorAt(1.0, QColor("#4FC3F7"))
        p.setPen(Qt.NoPen)
        p.setBrush(QBrush(g))
        p.drawRoundedRect(r, 14, 14)

        if self.isDown():
            p.setBrush(QColor(0, 0, 0, 40))
            p.drawRoundedRect(r, 14, 14)

        p.setPen(QColor("#FFFFFF"))
        f = self.font()
        f.setPointSize(12)
        f.setBold(True)
        p.setFont(f)
        p.drawText(self.rect(), Qt.AlignCenter, self.text())
        p.end()


class StarMark(QWidget):
    """The brand mark. White by default - colour is a property, not a theme."""

    def __init__(self, size: int = 26, gradient: bool = False, parent=None):
        super().__init__(parent)
        self._size = size
        self._gradient = gradient
        # main theme: a plain white star. The colour is a property so the light
        # theme can switch it to ink without a different asset.
        self._colour = QColor("#FFFFFF")
        self.setFixedSize(size, size)
        self.setAttribute(Qt.WA_TranslucentBackground)

    def set_colour(self, colour: QColor):
        self._colour = QColor(colour)
        self.update()

    def paintEvent(self, event):  # noqa: N802 (Qt naming)
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        rect = QRectF(0, 0, self._size, self._size)
        if self._gradient:
            g = QLinearGradient(rect.topLeft(), rect.bottomRight())
            g.setColorAt(0.0, QColor("#8B7BFF"))
            g.setColorAt(0.55, QColor("#6450F0"))
            g.setColorAt(1.0, QColor("#4FC3F7"))
            p.setBrush(QBrush(g))
        else:
            p.setBrush(QBrush(self._colour))
        p.setPen(Qt.NoPen)
        p.drawPath(star_path(rect))
        p.end()


# --------------------------------------------------------------------------- #
# Tokens read straight out of the generated stylesheet, so the Qt prototype
# and the QSS can never drift apart.
# --------------------------------------------------------------------------- #
class Theme:
    """Parses the generated QSS for the handful of colours the code paints."""

    PRESETS = ("oled", "dark", "light")
    ACCENTS = {
        "starlight": "#7C6BFF",
        "aurora": "#35D6ED",
        "nova": "#FF5FA2",
        "solar": "#FFB63D",
        "emerald": "#3ED598",
        "ember": "#FF6B4A",
    }

    def __init__(self, key: str):
        self.key = key
        self.text = (TOKENS_QSS_DIR / f"starclient-{key}.qss").read_text(encoding="utf-8")
        self.accent = self._pick("accent", "#7C6BFF")

    def _pick(self, prop: str, fallback: str) -> str:
        needle = f"{prop}:"
        for line in self.text.splitlines():
            line = line.strip()
            if line.startswith("background-color:") and needle in line:
                return line.split(needle, 1)[1].split(";")[0].strip()
        return fallback

    @property
    def is_dark(self) -> bool:
        return self.key != "light"

    def stylesheet(self, accent_key: str) -> str:
        """Swaps the generated accent colour for the requested preset.

        The QSS references icons as Qt resources (:/starclient/icons/...).
        This harness is a plain script, so those URLs are redirected at the
        real files on disk. The launcher compiles starclient.qrc instead.
        """
        out = self.text.replace(self.accent, self.ACCENTS[accent_key])
        return out.replace(":/starclient/icons/", str(ICON_DIR) + "/")

    def star_colour(self) -> QColor:
        """The brand mark is white on dark themes and near-black on light."""
        return QColor("#0F131C") if not self.is_dark else QColor("#FFFFFF")


# --------------------------------------------------------------------------- #
# Small building blocks
# --------------------------------------------------------------------------- #
def role(widget: QWidget, value: str) -> QWidget:
    """Tags a widget for the QSS attribute selectors."""
    widget.setProperty("starRole", value)
    return widget


class Card(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        role(self, "card")
        self.setFrameShape(QFrame.NoFrame)


class RailButton(QPushButton):
    def __init__(self, glyph: str, tip: str, parent=None):
        super().__init__(parent)
        self.setText(glyph)
        self.setToolTip(tip)
        self.setCheckable(True)
        self.setCursor(Qt.PointingHandCursor)
        self.setFixedSize(44, 44)
        f = QFont()
        f.setPointSize(15)
        self.setFont(f)


class Switch(QCheckBox):
    """QCheckBox restyled into an iOS-style switch by the QSS."""

    def __init__(self, on: bool = True, parent=None):
        super().__init__(parent)
        self.setChecked(on)
        role(self, "switch")


class InstanceCard(Card):
    clicked = Signal(str)

    def __init__(self, name: str, meta: str, badges: list[str], tint: QColor, parent=None):
        super().__init__(parent)
        self.name = name
        self.setCursor(Qt.PointingHandCursor)
        self.setMinimumSize(250, 190)

        lay = QVBoxLayout(self)
        lay.setContentsMargins(16, 16, 16, 16)
        lay.setSpacing(8)

        art = _ArtBlock(tint, name[:1])
        lay.addWidget(art)

        title = role(QLabel(name), "h3")
        lay.addWidget(title)

        sub = QLabel(meta)
        role(sub, "caption")
        lay.addWidget(sub)

        row = QHBoxLayout()
        row.setSpacing(6)
        for i, b in enumerate(badges):
            chip = QLabel(b)
            role(chip, "badge")
            if i == 0:
                chip.setProperty("starState", "accent")
            row.addWidget(chip)
        row.addStretch(1)
        lay.addLayout(row)

    def mouseReleaseEvent(self, event):  # noqa: N802
        self.clicked.emit(self.name)
        super().mouseReleaseEvent(event)


class _ArtBlock(QWidget):
    """Blocky stand-in for instance artwork (the real app uses a pixmap)."""

    def __init__(self, colour: QColor, letter: str, parent=None):
        super().__init__(parent)
        self.colour = colour
        self.letter = letter
        self.setFixedHeight(72)
        self.setAttribute(Qt.WA_StyledBackground, True)

    def paintEvent(self, event):  # noqa: N802
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(0.5, 0.5, -0.5, -0.5)
        p.setPen(Qt.NoPen)
        p.setBrush(self.colour)
        p.drawRoundedRect(r, 8, 8)
        # a few lighter blocks, echoing the blocky art style
        lighter = QColor(self.colour).lighter(135)
        for (x, y, w, h) in ((0.12, 0.28, 0.18, 0.34), (0.55, 0.16, 0.22, 0.28), (0.38, 0.55, 0.28, 0.3)):
            p.setBrush(lighter)
            p.drawRect(QRectF(r.x() + r.width() * x, r.y() + r.height() * y,
                              r.width() * w, r.height() * h))
        p.setBrush(QColor(255, 255, 255, 40))
        p.drawRoundedRect(r, 8, 8)
        p.end()


class GcOption(QFrame):
    """Radio card for a garbage collector, showing the flags it applies."""

    def __init__(self, label: str, desc: str, args: str, tag: str = "", parent=None):
        super().__init__(parent)
        role(self, "card")
        self.setCursor(Qt.PointingHandCursor)

        lay = QHBoxLayout(self)
        lay.setContentsMargins(16, 14, 16, 14)
        lay.setSpacing(12)

        self.radio = QRadioButton()
        lay.addWidget(self.radio, 0, Qt.AlignTop)

        body = QVBoxLayout()
        body.setSpacing(4)

        head = QHBoxLayout()
        head.setSpacing(8)
        name = QLabel(label)
        f = name.font()
        f.setBold(True)
        name.setFont(f)
        head.addWidget(name)
        if tag:
            badge = QLabel(tag)
            role(badge, "badge")
            badge.setProperty("starState", "accent")
            head.addWidget(badge)
        head.addStretch(1)
        body.addLayout(head)

        d = QLabel(desc)
        role(d, "muted")
        d.setWordWrap(True)
        body.addWidget(d)

        a = QLabel(args)
        role(a, "mono")
        a.setWordWrap(True)
        a.setProperty("starState", "subtle")
        body.addWidget(a)

        lay.addLayout(body, 1)

    def mouseReleaseEvent(self, event):  # noqa: N802
        self.radio.setChecked(True)
        super().mouseReleaseEvent(event)


# --------------------------------------------------------------------------- #
# Pages
# --------------------------------------------------------------------------- #
class StarStart(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        outer = QScrollArea()
        outer.setWidgetResizable(True)
        outer.setFrameShape(QFrame.NoFrame)

        body = QWidget()
        outer.setWidget(body)
        lay = QVBoxLayout(body)
        lay.setContentsMargins(32, 28, 32, 40)
        lay.setSpacing(20)

        # ---- hero
        hero = Card()
        hero.setMinimumHeight(230)
        h = QHBoxLayout(hero)
        h.setContentsMargins(32, 28, 32, 28)

        text = QVBoxLayout()
        text.setSpacing(10)

        eyebrow = QLabel("◆   STAR START")
        role(eyebrow, "badge")
        eyebrow.setProperty("starState", "accent")
        eyebrow.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Maximum)
        text.addWidget(eyebrow)

        title = QLabel("Ready for lift-off, StarKnight.")
        role(title, "h1")
        text.addWidget(title)

        sub = QLabel("Star Client 21.1  ·  Fabric 0.16.9  ·  87 mods loaded  ·  6.0 GB allocated")
        role(sub, "subtitle")
        text.addWidget(sub)

        actions = QHBoxLayout()
        actions.setSpacing(12)
        launch = StarButton("▶   Launch Star Client")
        launch.setObjectName("heroLaunch")
        actions.addWidget(launch)
        other = QPushButton("Choose another instance")
        role(other, "ghost")
        other.setCursor(Qt.PointingHandCursor)
        actions.addWidget(other)
        actions.addStretch(1)
        text.addLayout(actions)

        stats = QHBoxLayout()
        stats.setSpacing(28)
        for value, label in (("6144 MB", "HEAP"), ("G1GC", "COLLECTOR"),
                             ("Java 21", "RUNTIME"), ("118 MB", "LAUNCHER RAM")):
            cell = QVBoxLayout()
            cell.setSpacing(0)
            v = QLabel(value)
            f = v.font()
            f.setBold(True)
            v.setFont(f)
            cell.addWidget(v)
            cap = QLabel(label)
            role(cap, "caption")
            cell.addWidget(cap)
            stats.addLayout(cell)
        stats.addStretch(1)
        text.addSpacing(6)
        text.addLayout(stats)

        h.addLayout(text, 1)

        star = StarMark(150)
        h.addWidget(star, 0, Qt.AlignVCenter)
        lay.addWidget(hero)

        # ---- instance grid
        heading = QLabel("Your instances")
        role(heading, "h3")
        lay.addWidget(heading)

        grid = QGridLayout()
        grid.setSpacing(16)
        data = [
            ("Star Client", "2 hours ago", ["Fabric", "1.21.1", "87 mods"], QColor("#1B2C40")),
            ("Skyblock Islands", "yesterday", ["Fabric", "1.21.1", "42 mods"], QColor("#4A8FD1")),
            ("Vanilla 1.21.1", "4 days ago", ["Vanilla", "1.21.1", "0 mods"], QColor("#5A8A3C")),
        ]
        for i, (name, meta, badges, tint) in enumerate(data):
            grid.addWidget(InstanceCard(name, meta, badges, tint), 0, i)
        lay.addLayout(grid)
        lay.addStretch(1)

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.addWidget(outer)


class InstancePage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        outer = QScrollArea()
        outer.setWidgetResizable(True)
        outer.setFrameShape(QFrame.NoFrame)
        body = QWidget()
        outer.setWidget(body)
        lay = QVBoxLayout(body)
        lay.setContentsMargins(32, 28, 32, 40)
        lay.setSpacing(18)

        header = Card()
        hh = QHBoxLayout(header)
        hh.setContentsMargins(20, 18, 20, 18)
        hh.setSpacing(16)

        art = _ArtBlock(QColor("#1B2C40"), "S")
        art.setFixedSize(64, 64)
        hh.addWidget(art)

        ident = QVBoxLayout()
        ident.setSpacing(6)
        name = QLabel("Star Client")
        f = name.font()
        f.setPointSize(16)
        f.setBold(True)
        name.setFont(f)
        ident.addWidget(name)
        badges = QHBoxLayout()
        badges.setSpacing(6)
        for i, b in enumerate(("Fabric", "1.21.1", "Java 21", "87 mods")):
            chip = QLabel(b)
            role(chip, "badge")
            if i == 0:
                chip.setProperty("starState", "accent")
            elif i == 2:
                chip.setProperty("starState", "success")
            badges.addWidget(chip)
        badges.addStretch(1)
        ident.addLayout(badges)
        hh.addLayout(ident, 1)

        for value, label in (("6144 MB", "HEAP"), ("G1GC", "COLLECTOR")):
            cell = QVBoxLayout()
            cell.setSpacing(0)
            cap = QLabel(label)
            role(cap, "caption")
            cell.addWidget(cap)
            v = QLabel(value)
            f = v.font()
            f.setBold(True)
            v.setFont(f)
            cell.addWidget(v)
            hh.addLayout(cell)

        play = QPushButton("▶   Play")
        role(play, "launch")
        play.setCursor(Qt.PointingHandCursor)
        hh.addWidget(play)
        lay.addWidget(header)

        # quick tuning: RAM + collector, the two controls people actually touch
        cols = QHBoxLayout()
        cols.setSpacing(16)

        tune = Card()
        tl = QVBoxLayout(tune)
        tl.setContentsMargins(20, 18, 20, 20)
        tl.setSpacing(10)
        tl.addWidget(role(QLabel("Tune before launch"), "h3"))
        hint = QLabel("Changes apply to this instance only")
        role(hint, "caption")
        tl.addWidget(hint)

        tl.addSpacing(6)
        tl.addWidget(role(QLabel("Memory allocation"), "muted"))

        ram_value = QLabel("6144 MB")
        f = ram_value.font()
        f.setPointSize(18)
        f.setBold(True)
        ram_value.setFont(f)
        tl.addWidget(ram_value)

        slider = QSlider(Qt.Horizontal)
        slider.setRange(512, 16384)
        slider.setSingleStep(256)
        slider.setPageStep(1024)
        slider.setValue(6144)
        role(slider, "ram")
        tl.addWidget(slider)

        zones = QLabel("512 MB                              8 GB                              16 GB")
        role(zones, "caption")
        tl.addWidget(zones)

        quick = QHBoxLayout()
        quick.setSpacing(6)
        for gb in (2, 4, 6, 8):
            b = QPushButton(f"{gb} GB")
            role(b, "pill")
            b.setCheckable(True)
            b.setChecked(gb == 6)
            b.setCursor(Qt.PointingHandCursor)
            quick.addWidget(b)
        quick.addStretch(1)
        tl.addLayout(quick)

        def on_slide(v: int):
            ram_value.setText(f"{v} MB")

        slider.valueChanged.connect(on_slide)

        tl.addSpacing(8)
        tl.addWidget(role(QLabel("Garbage collector"), "muted"))
        combo = QComboBox()
        combo.addItems([
            "G1GC — balanced default",
            "ZGC — ultra-low pause (Java 17+)",
            "Generational ZGC — Java 21+",
            "Shenandoah — low pause",
            "Serial GC — smallest footprint",
            "Parallel GC — max throughput",
        ])
        tl.addWidget(combo)
        cd = QLabel("Best all-round choice for 4-8 GB heaps.")
        role(cd, "caption")
        tl.addWidget(cd)
        tl.addStretch(1)

        summary = Card()
        sl = QVBoxLayout(summary)
        sl.setContentsMargins(20, 18, 20, 20)
        sl.setSpacing(10)
        sl.addWidget(role(QLabel("Instance summary"), "h3"))
        for k, v in (("Loader", "Fabric 0.16.9"),
                     ("Minecraft", "1.21.1"),
                     ("Java", "Eclipse Temurin 21.0.4"),
                     ("Memory", "6144 MB"),
                     ("Collector", "G1GC"),
                     ("Star Menu", "Enabled · 6 presets")):
            row = QHBoxLayout()
            kl = QLabel(k)
            role(kl, "muted")
            row.addWidget(kl)
            row.addStretch(1)
            row.addWidget(QLabel(v))
            sl.addLayout(row)
        sl.addStretch(1)

        cols.addWidget(tune, 1)
        cols.addWidget(summary, 1)
        lay.addLayout(cols)
        lay.addStretch(1)

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.addWidget(outer)


class SettingsPage(QWidget):
    def __init__(self, on_theme, on_accent, parent=None):
        super().__init__(parent)
        outer = QScrollArea()
        outer.setWidgetResizable(True)
        outer.setFrameShape(QFrame.NoFrame)
        body = QWidget()
        outer.setWidget(body)
        lay = QVBoxLayout(body)
        lay.setContentsMargins(32, 28, 32, 40)
        lay.setSpacing(16)

        title = QLabel("Settings")
        role(title, "h1")
        lay.addWidget(title)

        # ---- appearance
        appear = Card()
        al = QVBoxLayout(appear)
        al.setContentsMargins(20, 18, 20, 20)
        al.setSpacing(12)
        al.addWidget(role(QLabel("Appearance"), "h3"))

        theme_row = QHBoxLayout()
        theme_row.setSpacing(8)
        theme_row.addWidget(QLabel("Theme"))
        self.theme_buttons: dict[str, QPushButton] = {}
        for key in Theme.PRESETS:
            b = QPushButton({"oled": "OLED Black", "dark": "Midnight", "light": "Daylight"}[key])
            role(b, "pill")
            b.setCheckable(True)
            b.setChecked(key == "oled")
            b.setCursor(Qt.PointingHandCursor)
            b.clicked.connect(lambda _=False, k=key: on_theme(k))
            self.theme_buttons[key] = b
            theme_row.addWidget(b)
        theme_row.addStretch(1)
        al.addLayout(theme_row)

        accent_row = QHBoxLayout()
        accent_row.setSpacing(8)
        accent_row.addWidget(QLabel("Accent"))
        self.accent_buttons: dict[str, QPushButton] = {}
        for key, hexv in Theme.ACCENTS.items():
            b = QPushButton()
            b.setFixedSize(30, 30)
            b.setCheckable(True)
            b.setChecked(key == "starlight")
            b.setCursor(Qt.PointingHandCursor)
            b.setToolTip(key.title())
            b.setStyleSheet(
                f"QPushButton {{ background: {hexv}; border: 2px solid transparent; border-radius: 15px; }}"
                f"QPushButton:checked {{ border-color: #FFFFFF; }}"
                f"QPushButton:hover {{ border-color: rgba(255,255,255,0.5); }}"
            )
            b.clicked.connect(lambda _=False, k=key: on_accent(k))
            self.accent_buttons[key] = b
            accent_row.addWidget(b)
        accent_row.addStretch(1)
        al.addLayout(accent_row)

        for label, desc, checked in (
            ("Reduce transparency & blur", "Fewer compositing layers, lower GPU and memory cost", False),
            ("Reduce motion", "Disable page and card animations", False),
            ("Animate the hero star", "Slow breathing glow on the Star Start page", True),
            ("Compact density", "Tighter rows, more instances on screen", False),
        ):
            row = QHBoxLayout()
            cell = QVBoxLayout()
            cell.setSpacing(1)
            row.addLayout(cell)
            name = QLabel(label)
            cell.addWidget(name)
            d = QLabel(desc)
            role(d, "caption")
            cell.addWidget(d)
            row.addStretch(1)
            row.addWidget(Switch(checked))
            al.addLayout(row)
        lay.addWidget(appear)

        # ---- garbage collector
        gc = Card()
        gl = QVBoxLayout(gc)
        gl.setContentsMargins(20, 18, 20, 20)
        gl.setSpacing(12)
        gl.addWidget(role(QLabel("Garbage collector"), "h3"))
        gl.addWidget(GcOption(
            "G1GC", "Balanced default. Best compatibility and steady frame pacing on 4-8 GB heaps.",
            "-XX:+UseG1GC -XX:MaxGCPauseMillis=200 -XX:G1HeapRegionSize=8M -XX:+DisableExplicitGC",
            "Default"))
        gl.addWidget(GcOption(
            "ZGC", "Ultra-low pause times. Needs Java 17+ and 8 GB+ of RAM to beat G1GC.",
            "-XX:+UseZGC -XX:ZCollectionInterval=120 -XX:ZAllocationSpikeTolerance=5"))
        gl.addWidget(GcOption(
            "Generational ZGC", "Java 21+ only. Low pauses with better throughput than classic ZGC.",
            "-XX:+UseZGC -XX:+ZGenerational", "New"))
        lay.addWidget(gc)

        # ---- memory behaviour
        mem = Card()
        ml = QVBoxLayout(mem)
        ml.setContentsMargins(20, 18, 20, 20)
        ml.setSpacing(12)
        ml.addWidget(role(QLabel("Memory"), "h3"))
        for label, desc, checked in (
            ("Auto-size per instance", "Estimate from mod count and render distance", True),
            ("Leave headroom for the system", "Never allocate more than 75% of physical memory", True),
            ("Warn above 12 GB", "Very large heaps often measure slower than a tuned 8 GB one", True),
            ("Lazy icon loading", "Decode instance art only when it scrolls into view", True),
            ("Free caches while playing", "Release cached art when Minecraft starts", True),
        ):
            row = QHBoxLayout()
            cell = QVBoxLayout()
            cell.setSpacing(1)
            row.addLayout(cell)
            cell.addWidget(QLabel(label))
            d = QLabel(desc)
            role(d, "caption")
            cell.addWidget(d)
            row.addStretch(1)
            row.addWidget(Switch(checked))
            ml.addLayout(row)
        lay.addWidget(mem)
        lay.addStretch(1)

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.addWidget(outer)


# --------------------------------------------------------------------------- #
# Shell
# --------------------------------------------------------------------------- #
class StarClientWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Star Client")
        self.setWindowIcon(QIcon(star_pixmap(256, gradient=False)))
        self.resize(1440, 900)

        self.theme_key = "oled"
        self.accent_key = "starlight"

        root = QHBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        # ---- rail
        rail = QFrame()
        rail.setFixedWidth(64)
        rail.setProperty("starRole", "sidebar")
        rl = QVBoxLayout(rail)
        rl.setContentsMargins(10, 14, 10, 14)
        rl.setSpacing(8)

        logo = StarMark(28)
        logo.setProperty("starRole", "logo")
        rl.addWidget(logo, 0, Qt.AlignHCenter)
        rl.addSpacing(10)

        self.rail_buttons: list[RailButton] = []
        for glyph, tip in (("⌂", "Star Start"), ("▦", "Instances"),
                           ("◈", "Browse content"), ("☺", "Accounts")):
            b = RailButton(glyph, tip)
            b.clicked.connect(lambda _=False, t=tip: self._navigate(t))
            self.rail_buttons.append(b)
            rl.addWidget(b, 0, Qt.AlignHCenter)
        rl.addStretch(1)

        theme_btn = RailButton("◐", "Switch appearance")
        theme_btn.setCheckable(False)
        theme_btn.clicked.connect(self._cycle_theme)
        rl.addWidget(theme_btn, 0, Qt.AlignHCenter)
        settings_btn = RailButton("⚙", "Settings")
        settings_btn.clicked.connect(lambda: self._navigate("Settings"))
        self.rail_buttons.append(settings_btn)
        rl.addWidget(settings_btn, 0, Qt.AlignHCenter)
        root.addWidget(rail)

        # ---- main column
        column = QVBoxLayout()
        column.setContentsMargins(0, 0, 0, 0)
        column.setSpacing(0)

        titlebar = QFrame()
        titlebar.setFixedHeight(48)
        titlebar.setProperty("starRole", "titlebar")
        tb = QHBoxLayout(titlebar)
        tb.setContentsMargins(18, 8, 18, 8)
        tb.setSpacing(12)

        self.brand_star = StarMark(18)
        tb.addWidget(self.brand_star)
        brand = QLabel("Star Client")
        f = brand.font()
        f.setBold(True)
        f.setPointSize(12)
        brand.setFont(f)
        tb.addWidget(brand)

        search = QLineEdit()
        search.setPlaceholderText("Search instances, mods, modpacks…")
        search.setFixedWidth(380)
        role(search, "search")
        tb.addWidget(search)
        tb.addStretch(1)

        quick = QPushButton("⚡  Quick Start")
        role(quick, "pill")
        quick.setCursor(Qt.PointingHandCursor)
        tb.addWidget(quick)

        account = QLabel("StarKnight\nMICROSOFT")
        role(account, "badge")
        tb.addWidget(account)
        column.addWidget(titlebar)

        self.stack = QStackedWidget()
        self.page_home = StarStart()
        self.page_instance = InstancePage()
        self.page_settings = SettingsPage(self._set_theme, self._set_accent)
        self.stack.addWidget(self.page_home)
        self.stack.addWidget(self.page_instance)
        self.stack.addWidget(self.page_settings)
        column.addWidget(self.stack, 1)
        root.addLayout(column, 1)

        self._navigate("Star Start")
        self._apply_theme()

    # -- navigation -------------------------------------------------------- #
    def _navigate(self, target: str):
        mapping = {"Star Start": 0, "Instances": 1, "Browse content": 0,
                   "Accounts": 0, "Settings": 2}
        idx = mapping.get(target, 0)
        # "Instances" opens the instance page in this prototype
        self.stack.setCurrentIndex(1 if target == "Instances" else idx)
        for b in self.rail_buttons:
            b.setChecked(b.toolTip() == target)

    # -- theming ----------------------------------------------------------- #
    def _cycle_theme(self):
        order = list(Theme.PRESETS)
        nxt = order[(order.index(self.theme_key) + 1) % len(order)]
        self._set_theme(nxt)

    def _set_theme(self, key: str):
        self.theme_key = key
        self._apply_theme()
        page = self.page_settings
        for k, b in page.theme_buttons.items():
            b.setChecked(k == key)

    def _set_accent(self, key: str):
        self.accent_key = key
        self._apply_theme()
        for k, b in self.page_settings.accent_buttons.items():
            b.setChecked(k == key)

    def _apply_theme(self):
        theme = Theme(self.theme_key)
        self.setStyleSheet(theme.stylesheet(self.accent_key))
        # the main theme is a white star; the light theme swaps it to ink
        colour = theme.star_colour()
        for mark in self.findChildren(StarMark):
            mark.set_colour(colour)
        # keep the one gradient widget in step with the accent preset
        hero = self.page_home.findChild(StarButton, "heroLaunch")
        if hero:
            hero.set_accent(Theme.ACCENTS[self.accent_key])


# --------------------------------------------------------------------------- #
def main() -> int:
    parser = argparse.ArgumentParser(description="Star Client Qt Widgets prototype")
    parser.add_argument("--shots", metavar="DIR", help="render screenshots and exit")
    args = parser.parse_args()

    app = QApplication(sys.argv)

    fonts = ["Inter", "DejaVu Sans", "Noto Sans"]
    for name in fonts:
        f = QFont(name)
        if f.exactMatch():
            app.setFont(f)
            break

    win = StarClientWindow()

    if args.shots:
        out = Path(args.shots)
        out.mkdir(parents=True, exist_ok=True)
        win.resize(1440, 900)
        win.show()
        app.processEvents()

        def snap(name: str):
            app.processEvents()
            win.grab().save(str(out / f"{name}.png"))
            print(f"  {name}.png")

        snap("qt-10-oled-star-start")
        win._navigate("Instances")
        snap("qt-11-oled-instance")
        win._navigate("Settings")
        snap("qt-20-oled-settings")
        win._set_theme("light")
        snap("qt-40-light-settings")
        win._navigate("Star Start")
        snap("qt-41-light-star-start")
        win._set_theme("oled")
        win._set_accent("aurora")
        snap("qt-50-oled-aurora-star-start")
        print(f"\nSaved to {out}")
        return 0

    win.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
