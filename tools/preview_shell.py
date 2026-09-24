"""Render the whole Star Client window off the real styles and palettes.

Run it:  python3 tools/preview_shell.py [output-dir]

Same sandbox needs as tools/preview_rail.py. Everything here mirrors the
launcher's own code - the palettes from StarTheme.cpp, the stylesheet from the
theme generator, StarShell::buildHeaderBar, StarPage and the rail - so the
screenshots tell the truth about what the binary renders.
"""
import sys
from pathlib import Path

from PySide6.QtCore import QRectF, QSize, Qt
from PySide6.QtGui import (QAction, QColor, QCursor, QIcon, QImage, QPainter, QPainterPath, QPalette, QPixmap, QTransform)
from PySide6.QtWidgets import (QApplication, QFrame, QGridLayout, QHBoxLayout, QLabel, QLineEdit, QMainWindow,
                               QScrollArea, QSizePolicy, QStackedWidget, QToolBar, QToolButton, QVBoxLayout, QWidget)

REPO = Path(__file__).resolve().parent.parent
GLYPH, MARK = 22, 26

THEMES = {
    "oled": dict(qss="starclient-oled.qss", window="#000000", base="#0A0A0E", text="#F3F4F8",
                 button="#0B0B0F", highlight="#7C6BFF", name="OLED Black"),
    "midnight": dict(qss="starclient-dark.qss", window="#0C0C11", base="#101017", text="#EEF0F6",
                     button="#15151C", highlight="#7C6BFF", name="Midnight"),
    "daylight": dict(qss="starclient-light.qss", window="#F2F4FA", base="#FFFFFF", text="#0F131C",
                     button="#FFFFFF", highlight="#6450F0", name="Daylight"),
    "navy": dict(qss="starclient-navy.qss", window="#000000", base="#0A0A0E", text="#F3F4F8",
                 button="#0B0B0F", highlight="#4C7DF0", name="Navy"),
    "white": dict(qss="starclient-white.qss", window="#FFFFFF", base="#FFFFFF", text="#0F131C",
                  button="#FFFFFF", highlight="#6450F0", name="White"),
}


def palette_for(t):
    p = QPalette()
    p.setColor(QPalette.Window, QColor(t["window"]))
    p.setColor(QPalette.WindowText, QColor(t["text"]))
    p.setColor(QPalette.Base, QColor(t["base"]))
    p.setColor(QPalette.Text, QColor(t["text"]))
    p.setColor(QPalette.Button, QColor(t["button"]))
    p.setColor(QPalette.ButtonText, QColor(t["text"]))
    p.setColor(QPalette.Highlight, QColor(t["highlight"]))
    p.setColor(QPalette.HighlightedText, QColor("#FFFFFF"))
    return p


def mix(a, b, bias):
    return QColor(round(a.red() * (1 - bias) + b.red() * bias),
                  round(a.green() * (1 - bias) + b.green() * bias),
                  round(a.blue() * (1 - bias) + b.blue() * bias))


def tinted(src: QIcon, color: QColor, size: int, dpr: float) -> QPixmap:
    masked = src.pixmap(QSize(size, size), dpr)
    out = QPixmap(masked.size()); out.setDevicePixelRatio(dpr); out.fill(Qt.transparent)
    p = QPainter(out); p.drawPixmap(0, 0, masked)
    p.setCompositionMode(QPainter.CompositionMode.CompositionMode_SourceIn)
    p.fillRect(out.rect(), color); p.end()
    return out


def star_path(box):
    p = QPainterPath()
    p.moveTo(128.0, 16.0)
    p.quadTo(150.4, 105.6, 240.0, 128.0)
    p.quadTo(150.4, 150.4, 128.0, 240.0)
    p.quadTo(105.6, 150.4, 16.0, 128.0)
    p.quadTo(105.6, 105.6, 128.0, 16.0)
    p.closeSubpath()
    t = QTransform(); t.translate(box.x(), box.y()); t.scale(box.width() / 256.0, box.height() / 256.0)
    return t.map(p)


def star_icon(size, color, dpr):
    pm = QPixmap(QSize(size, size) * dpr); pm.setDevicePixelRatio(dpr); pm.fill(Qt.transparent)
    p = QPainter(pm); p.setRenderHint(QPainter.Antialiasing); p.setPen(Qt.NoPen)
    p.setBrush(color); p.drawPath(star_path(QRectF(0, 0, size, size))); p.end()
    return QIcon(pm)


def glyph_icon(name, size, dpr):
    src = QIcon(str(REPO / f"src/theme/icons/{name}.svg"))
    pal = QApplication.palette()
    idle = mix(pal.color(QPalette.WindowText), pal.color(QPalette.Window), 0.45)
    accent = pal.color(QPalette.Highlight)
    icon = QIcon()
    icon.addPixmap(tinted(src, idle, size, dpr), QIcon.Normal, QIcon.Off)
    icon.addPixmap(tinted(src, accent, size, dpr), QIcon.Normal, QIcon.On)
    return icon


def build_rail(parent, dpr, star_color):
    rail = QToolBar(parent)
    rail.setObjectName("starNavRail")
    rail.setOrientation(Qt.Vertical); rail.setMovable(False)
    rail.setIconSize(QSize(GLYPH, GLYPH)); rail.setToolButtonStyle(Qt.ToolButtonIconOnly)
    rail.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)
    rail.addAction(star_icon(MARK, star_color, dpr), "")
    for name, label, checked in [("rail-home", "Star Start", True), ("rail-grid", "Instances", False),
                                 ("rail-compass", "Browse content", False), ("rail-users", "Accounts", False)]:
        a = QAction(label, rail); a.setCheckable(True); a.setChecked(checked)
        a.setToolTip(label); a.setIcon(glyph_icon(name, GLYPH, dpr)); rail.addAction(a)
    rail.addSeparator()
    spacer = QWidget(rail); spacer.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
    rail.addWidget(spacer)
    for name, label in [("rail-moon", "Switch appearance"), ("rail-cog", "Settings")]:
        a = QAction(label, rail); a.setToolTip(label); a.setIcon(glyph_icon(name, GLYPH, dpr)); rail.addAction(a)
    from PySide6.QtWidgets import QToolButton
    for w in rail.findChildren(QToolButton):
        w.setAutoRaise(True); w.setFocusPolicy(Qt.NoFocus)
    return rail


def build_header(parent, dpr):
    bar = QWidget(parent); bar.setObjectName("starHeader")
    row = QHBoxLayout(bar); row.setContentsMargins(24, 14, 24, 14); row.setSpacing(12)
    brand = QWidget(bar); brow = QHBoxLayout(brand); brow.setContentsMargins(0, 0, 0, 0); brow.setSpacing(9)
    mark = QLabel(brand)
    mark.setPixmap(QIcon(str(REPO / "src/theme/icons/star-glyph.svg")).pixmap(QSize(20, 20)))
    brow.addWidget(mark)
    name = QLabel("Star Client", brand); name.setProperty("starRole", "h2"); brow.addWidget(name)
    row.addWidget(brand); row.addSpacing(18)
    search = QLineEdit(bar); search.setObjectName("starSearch"); search.setProperty("starRole", "search")
    search.setClearButtonEnabled(True); search.setPlaceholderText("Search instances, worlds, mods…")
    row.addWidget(search, 1)
    quick = QToolButton(bar); quick.setObjectName("starQuickStart"); quick.setProperty("starRole", "primary")
    quick.setToolButtonStyle(Qt.ToolButtonTextBesideIcon); quick.setText("Quick start")
    row.addWidget(quick)
    acc = QToolButton(bar); acc.setObjectName("starAccount"); acc.setProperty("starRole", "outline")
    acc.setToolButtonStyle(Qt.ToolButtonTextBesideIcon); acc.setText("steve")
    row.addWidget(acc)
    return bar


def instance_card(parent, name, version, badges, playing=False):
    card = QFrame(parent); card.setObjectName("card"); card.setProperty("starRole", "card")
    v = QVBoxLayout(card); v.setContentsMargins(18, 16, 18, 16); v.setSpacing(12)
    top = QHBoxLayout(); top.setSpacing(10)
    ic = QLabel(card); ic.setFixedSize(38, 38); ic.setProperty("starRole", "badge")
    ic.setAlignment(Qt.AlignCenter); ic.setText(name[:2].upper())
    top.addWidget(ic)
    info = QVBoxLayout(); info.setSpacing(2)
    n = QLabel(name, card); n.setProperty("starRole", "h2"); info.addWidget(n)
    sub = QLabel(version, card); sub.setProperty("starRole", "subtitle"); info.addWidget(sub)
    top.addLayout(info, 1)
    star = QLabel(card); star.setPixmap(star_icon(16, card.palette().color(QPalette.Highlight), 2.0).pixmap(QSize(16, 16)))
    top.addWidget(star, 0, Qt.AlignTop)
    v.addLayout(top)
    brows = QHBoxLayout(); brows.setSpacing(6)
    for b in badges:
        chip = QLabel(b, card); chip.setProperty("starRole", "badge"); chip.setProperty("starState", "accent")
        brows.addWidget(chip)
    brows.addStretch(1)
    v.addLayout(brows)
    play = QToolButton(card); play.setProperty("starRole", "primary")
    play.setText("Playing" if playing else "Play")
    v.addWidget(play)
    return card


def build_mainwindow(key):
    t = THEMES[key]
    app = QApplication.instance()
    app.setPalette(palette_for(t)); app.setStyleSheet((REPO / f"src/theme/generated/{t['qss']}").read_text())

    win = QMainWindow()
    win.resize(1000, 640)

    # the rail
    star_color = QColor("#FFFFFF") if key != "daylight" else QColor("#0F131C")
    rail = build_rail(win, 2.0 if key != "daylight" else 2.0, star_color)
    win.addToolBar(Qt.LeftToolBarArea, rail)

    # the shell: page over a content stack
    shell = QStackedWidget(win)
    win.setCentralWidget(shell)

    page = QWidget(); page.setProperty("starRole", "surface")
    outer = QVBoxLayout(page); outer.setContentsMargins(28, 24, 28, 24); outer.setSpacing(0)
    header = QHBoxLayout(); header.setSpacing(14)
    titling = QVBoxLayout(); titling.setSpacing(2)
    title = QLabel("Instances", page); title.setProperty("starRole", "h1"); titling.addWidget(title)
    sub = QLabel("Launch and manage your Minecraft installs.", page); sub.setProperty("starRole", "subtitle")
    titling.addWidget(sub)
    header.addLayout(titling, 1)
    outer.addLayout(header)
    outer.addSpacing(18)

    inner = QWidget(page); col = QVBoxLayout(inner); col.setContentsMargins(0, 0, 0, 0); col.setSpacing(0)
    col.addWidget(build_header(inner, 2.0))
    content = QWidget(inner); col.addWidget(content, 1)
    grid = QGridLayout(content); grid.setContentsMargins(0, 0, 0, 0); grid.setSpacing(10)
    examples = [
        ("All of Fabric 6", "Minecraft 1.20.1 • Fabric 0.14.22", ["Fabric", "Forge"], False),
        ("Sky Factory 4", "Minecraft 1.12.2 • Forge 14.23.5", ["Modded", "Skyblock"], False),
        ("Vault Hunters", "Minecraft 1.16.5 • Forge 36.2.39", ["Modpack"], False),
        ("BetterMC", "Minecraft 1.20.1 • Forge 47.1.3", ["3 updates"], False),
        ("Playing now", "Minecraft 1.20.1 • Fabric", ["Recent"], True),
        ("ATM 9", "Minecraft 1.20.1 • NeoForge 47.1.76", ["New"], False),
    ]
    for i, (nm, ver, badges, play) in enumerate(examples):
        grid.addWidget(instance_card(content, nm, ver, badges, play), i // 3, i % 3)
    outer.addWidget(inner, 1)
    shell.addWidget(page)
    shell.setCurrentWidget(page)
    return win


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/shellshot")
    out.mkdir(parents=True, exist_ok=True)
    app = QApplication(sys.argv)
    for key in THEMES:
        win = build_mainwindow(key)
        QCursor.setPos(9000, 9000)
        win.show(); app.processEvents(); app.processEvents()
        img = QImage(win.size(), QImage.Format_ARGB32); img.fill(Qt.transparent)
        p = QPainter(img); win.render(p, img.rect().topLeft()); p.end()
        target = out / f"shell-{key}.png"
        img.save(str(target))
        print("wrote", target, img.width(), "x", img.height())
        win.hide()


if __name__ == "__main__":
    main()
