"""Render the Star Client navigation rail the way the launcher draws it.

Run it:  python3 tools/preview_rail.py [output-dir]

Needs PySide6-Essentials. On a headless machine with no GL stack, run it with
QT_QPA_PLATFORM=offscreen and a stub libGL/libEGL on LD_LIBRARY_PATH; the Qt
SVG icon plugin must be present, since the rail's glyphs are SVG.

Mirrors launcher/ui/StarNavRail.cpp: the same glyphs, the same tinting from the
palette, the same star geometry, and the real generated stylesheet. Used to
check the look without a Windows desktop to run it on.
"""
import sys
from pathlib import Path
from PySide6.QtCore import Qt, QSize, QRectF
from PySide6.QtGui import (QAction, QColor, QCursor, QIcon, QImage, QPainter, QPainterPath, QPalette, QPixmap, QTransform)
from PySide6.QtWidgets import (QApplication, QFrame, QHBoxLayout, QLabel, QMainWindow, QSizePolicy, QToolBar, QVBoxLayout, QWidget)

REPO = Path(__file__).resolve().parent.parent
GLYPH_SIZE, MARK_SIZE = 22, 26

# palettes from launcher/ui/themes/StarTheme.cpp
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


def mix(c1, c2, bias):
    """Rainbow::mix, which the launcher uses."""
    return QColor(
        round(c1.red() * (1 - bias) + c2.red() * bias),
        round(c1.green() * (1 - bias) + c2.green() * bias),
        round(c1.blue() * (1 - bias) + c2.blue() * bias),
    )


def tinted(source: QIcon, color: QColor, size: int, dpr: float) -> QPixmap:
    masked = source.pixmap(QSize(size, size), dpr)
    out = QPixmap(masked.size())
    out.setDevicePixelRatio(dpr)
    out.fill(Qt.transparent)
    p = QPainter(out)
    p.drawPixmap(0, 0, masked)
    p.setCompositionMode(QPainter.CompositionMode.CompositionMode_SourceIn)
    p.fillRect(out.rect(), color)
    p.end()
    return out


def star_path(box: QRectF) -> QPainterPath:
    path = QPainterPath()
    path.moveTo(128.0, 16.0)
    path.quadTo(150.4, 105.6, 240.0, 128.0)
    path.quadTo(150.4, 150.4, 128.0, 240.0)
    path.quadTo(105.6, 150.4, 16.0, 128.0)
    path.quadTo(105.6, 105.6, 128.0, 16.0)
    path.closeSubpath()
    t = QTransform()
    t.translate(box.x(), box.y())
    t.scale(box.width() / 256.0, box.height() / 256.0)
    return t.map(path)


def star_icon(size, color, dpr):
    pm = QPixmap(QSize(size, size) * dpr)
    pm.setDevicePixelRatio(dpr)
    pm.fill(Qt.transparent)
    p = QPainter(pm)
    p.setRenderHint(QPainter.Antialiasing)
    p.setPen(Qt.NoPen)
    p.setBrush(color)
    p.drawPath(star_path(QRectF(0, 0, size, size)))
    p.end()
    return QIcon(pm)


def glyph_icon(name, size, dpr):
    src = QIcon(str(REPO / f"src/theme/icons/{name}.svg"))
    pal = QApplication.palette()
    idle = mix(pal.color(QPalette.WindowText), pal.color(QPalette.Window), 0.45)
    hover = mix(pal.color(QPalette.WindowText), pal.color(QPalette.Window), 0.15)
    accent = pal.color(QPalette.Highlight)
    icon = QIcon()
    icon.addPixmap(tinted(src, idle, size, dpr), QIcon.Normal, QIcon.Off)
    icon.addPixmap(tinted(src, hover, size, dpr), QIcon.Active, QIcon.Off)
    icon.addPixmap(tinted(src, accent, size, dpr), QIcon.Normal, QIcon.On)
    return icon


def build_theme(parent, key, dpr):
    t = THEMES[key]
    app = QApplication.instance()
    app.setPalette(palette_for(t))
    app.setStyleSheet((REPO / f"src/theme/generated/{t['qss']}").read_text())

    # the launcher's main window is a QMainWindow, which owns the background
    page = QMainWindow(parent)
    page.setObjectName(f"page-{key}")
    page.setPalette(palette_for(t))
    central = QWidget(page)
    page.setCentralWidget(central)
    row = QHBoxLayout(central)
    row.setContentsMargins(0, 0, 0, 0)
    row.setSpacing(0)

    # ---- the rail, exactly as StarNavRail builds it ------------------------
    rail = QToolBar(page)
    rail.setObjectName("starNavRail")
    rail.setOrientation(Qt.Vertical)
    rail.setMovable(False)
    rail.setFloatable(False)
    rail.setIconSize(QSize(GLYPH_SIZE, GLYPH_SIZE))
    rail.setToolButtonStyle(Qt.ToolButtonIconOnly)
    rail.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)

    # Mirrors StarNavRail::buildActions(): mark, the four destinations, a rule,
    # an expanding spacer, then the appearance switch above settings.
    star = rail.addAction(star_icon(MARK_SIZE, QColor("#FFFFFF") if key != "daylight" else QColor("#0F131C"), dpr), "")

    entries = [("rail-home", "Star Start", True), ("rail-grid", "Instances", False),
               ("rail-compass", "Browse content", False), ("rail-users", "Accounts", False)]
    bottom = [("rail-moon", "Switch appearance", False), ("rail-cog", "Settings", False)]

    def add_entry(name, label, checked):
        a = QAction(label, rail)
        a.setCheckable(True)
        a.setChecked(checked)
        a.setToolTip(label)
        a.setIcon(glyph_icon(name, GLYPH_SIZE, dpr))
        rail.addAction(a)
        return a

    for name, label, checked in entries:
        add_entry(name, label, checked)

    rail.addSeparator()
    spacer = QWidget(rail)
    spacer.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
    rail.addWidget(spacer)

    for name, label, checked in bottom:
        add_entry(name, label, checked)

    from PySide6.QtWidgets import QToolButton
    for w in rail.findChildren(QToolButton):
        w.setAutoRaise(True)
        w.setFocusPolicy(Qt.NoFocus)

    row.addWidget(rail)

    # ---- a stand-in for the window content, to judge contrast --------------
    body = QWidget(page)
    body.setObjectName("starPreviewBody")
    col = QVBoxLayout(body)
    col.setContentsMargins(24, 24, 24, 24)
    col.setSpacing(10)

    title = QLabel(t["name"], body)
    title.setProperty("starRole", "h2")
    col.addWidget(title)

    sub = QLabel("Navigation rail — glyphs tinted from the palette", body)
    sub.setProperty("starRole", "subtitle")
    col.addWidget(sub)

    # the three icon states, side by side, for review
    states = QWidget(body)
    srow = QHBoxLayout(states)
    srow.setContentsMargins(0, 8, 0, 0)
    srow.setSpacing(18)
    pal = QApplication.palette()
    idle = mix(pal.color(QPalette.WindowText), pal.color(QPalette.Window), 0.45)
    hover = mix(pal.color(QPalette.WindowText), pal.color(QPalette.Window), 0.15)
    accent = pal.color(QPalette.Highlight)
    for label, color in (("idle", idle), ("hover", hover), ("selected", accent)):
        cellrow = QVBoxLayout()
        pm = QLabel(states)
        pm.setPixmap(tinted(QIcon(str(REPO / "src/theme/icons/rail-grid.svg")), color, 28, dpr))
        cap = QLabel(label, states)
        cap.setProperty("starRole", "caption")
        cellrow.addWidget(pm)
        cellrow.addWidget(cap)
        srow.addLayout(cellrow)
    srow.addStretch(1)
    col.addWidget(states)

    card = QFrame(body)
    card.setObjectName("starPreviewCard")
    card.setProperty("starRole", "card")
    cl = QVBoxLayout(card)
    cl.setContentsMargins(16, 16, 16, 16)
    t2 = QLabel("Star Client", card)
    t2.setProperty("starRole", "h3")
    cl.addWidget(t2)
    d = QLabel("The rail is a real QToolBar docked on the left, so Qt keeps its\n"
               "keyboard handling, tooltips and high-DPI scaling.", card)
    d.setProperty("starRole", "muted")
    cl.addWidget(d)
    col.addWidget(card)
    col.addStretch(1)

    row.addWidget(body, 1)
    return page


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/railshot")
    out.mkdir(parents=True, exist_ok=True)
    app = QApplication(sys.argv)
    dpr = 2.0

    for key in THEMES:
        page = build_theme(None, key, dpr)
        page.resize(600, 460)
        page.show()
        QCursor.setPos(9000, 9000)
        app.processEvents()
        app.processEvents()
        # render() rather than grab(): a nested grab in the offscreen plugin
        # composites hover chrome that is not actually painted
        img = QImage(page.size(), QImage.Format_ARGB32)
        img.fill(Qt.transparent)
        painter = QPainter(img)
        page.render(painter, img.rect().topLeft())
        painter.end()
        target = out / f"rail-{key}.png"
        img.save(str(target))
        print("wrote", target, img.width(), "x", img.height())
        page.hide()


if __name__ == "__main__":
    main()
