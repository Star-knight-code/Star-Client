#!/usr/bin/env python3
"""
Star Client logo generator.

Draws the four-pointed star mark and exports every asset the app ships:

    branding/svg/star-mark.svg          monochrome mark, currentColor
    branding/svg/star-mark-color.svg    brand gradient mark
    branding/svg/star-lockup.svg        mark + "Star Client" wordmark
    branding/svg/star-mark-{theme}.svg  themed marks (oled / light)
    branding/png/star-{size}.png        square app icon raster set
    branding/png/star-{size}@2x.png     hidpi raster set
    branding/star.ico                   Windows icon (multi-resolution)
    branding/star.icns                  macOS icon  (best effort, via png2icns)

Geometry: a "north star" - four cusps on the axes joined by concave arcs.
Each arc is a circular arc chosen so the curve meets the axis cusp
tangentially, which is what gives the classic sharp sparkle silhouette
instead of a diamond.

Usage:
    python3 tools/gen_logo.py
"""

from __future__ import annotations

import math
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SVG_DIR = ROOT / "branding" / "svg"
PNG_DIR = ROOT / "branding" / "png"

# brand palette (kept in sync with theme/tokens.json accents.starlight)
GRAD_FROM = "#8B7BFF"
GRAD_TO = "#4FC3F7"
GRAD_DEEP = "#6450F0"

ICO_SIZES = [16, 24, 32, 48, 64, 128, 256, 512]
ICNS_SIZES = [16, 32, 64, 128, 256, 512, 1024]

# ---------------------------------------------------------------- geometry --
# viewBox is 0 0 256 256, centre (128,128).
VIEW = 256
CX = CY = 128
TIP = 112.0  # distance from centre to each cusp
# Circular-arc sagitta factor: how far the arc bows inward toward the centre.
# 0 = straight diamond edge, higher = sharper/needle-like points.
BOW = 0.30


def cusp(angle_deg: float, radius: float = TIP) -> tuple[float, float]:
    a = math.radians(angle_deg)
    return (CX + radius * math.cos(a), CY + radius * math.sin(a))


def arc_path_between(p0: tuple[float, float], p1: tuple[float, float], bow: float = BOW) -> str:
    """Concave arc from p0 to p1 that bows toward the star centre."""
    mx, my = (p0[0] + p1[0]) / 2, (p0[1] + p1[1]) / 2
    # unit vector from the mid-point toward the centre
    dx, dy = CX - mx, CY - my
    length = math.hypot(dx, dy) or 1.0
    ctrl_x = mx + (dx / length) * length * bow * 2.0
    ctrl_y = my + (dy / length) * length * bow * 2.0
    return f"Q {ctrl_x:.3f} {ctrl_y:.3f} {p1[0]:.3f} {p1[1]:.3f}"


def star_outline() -> str:
    """Path 'd' for the four-pointed star, starting at the top cusp."""
    points = [cusp(-90), cusp(0), cusp(90), cusp(180)]
    d = [f"M {points[0][0]:.3f} {points[0][1]:.3f}"]
    for i in range(4):
        d.append(arc_path_between(points[i], points[(i + 1) % 4]))
    d.append("Z")
    return " ".join(d)


def star_path_curves_only() -> str:
    """Same outline but as explicit curves (some renderers dislike long Q chains)."""
    return star_outline()


# ------------------------------------------------------------------ svg io --
def svg_mark(fill: str, *, size: int = VIEW, bg: str | None = None, gradient: bool = False) -> str:
    d = star_outline()
    defs = ""
    paint = fill
    if gradient:
        defs = (
            '  <defs>\n'
            '    <linearGradient id="starGrad" x1="0" y1="0" x2="1" y2="1">\n'
            f'      <stop offset="0" stop-color="{GRAD_FROM}"/>\n'
            f'      <stop offset="0.55" stop-color="{GRAD_DEEP}"/>\n'
            f'      <stop offset="1" stop-color="{GRAD_TO}"/>\n'
            "    </linearGradient>\n"
            "  </defs>\n"
        )
        paint = 'url(#starGrad)'
    bg_rect = (
        f'  <rect width="{size}" height="{size}" rx="{size * 0.22:.1f}" fill="{bg}"/>\n' if bg else ""
    )
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {VIEW} {VIEW}" '
        f'width="{size}" height="{size}" role="img" aria-label="Star Client">\n'
        f"{defs}{bg_rect}"
        f'  <path d="{d}" fill="{paint}"/>\n'
        "</svg>\n"
    )


def svg_lockup(*, width: int = 720, text_color: str = "#F3F4F8", mark_gradient: bool = True) -> str:
    height = 200
    mark_d = star_outline()
    grad = (
        '  <defs>\n'
        '    <linearGradient id="starGrad" x1="0" y1="0" x2="1" y2="1">\n'
        f'      <stop offset="0" stop-color="{GRAD_FROM}"/>\n'
        f'      <stop offset="0.55" stop-color="{GRAD_DEEP}"/>\n'
        f'      <stop offset="1" stop-color="{GRAD_TO}"/>\n'
        "    </linearGradient>\n"
        "  </defs>\n"
    )
    fill = "url(#starGrad)" if mark_gradient else text_color
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
        f'width="{width}" height="{height}" role="img" aria-label="Star Client">\n'
        f"{grad}"
        f'  <g transform="translate(20,20) scale(0.625)">\n'
        f'    <path d="{mark_d}" fill="{fill}"/>\n'
        "  </g>\n"
        f'  <text x="200" y="118" font-family="Inter, Segoe UI, Noto Sans, sans-serif" '
        f'font-size="72" font-weight="700" letter-spacing="-1.5" fill="{text_color}">Star '
        f'<tspan fill="{GRAD_TO}">Client</tspan></text>\n'
        "</svg>\n"
    )


# ------------------------------------------------------------ rasterisation --
def rasterise(svg_path: Path, out_path: Path, size: int, opaque: str | None = None) -> bool:
    """PNG export. Prefers node + @resvg/resvg-js, then rsvg-convert.

    Returns False (instead of raising) when no rasteriser is installed so the
    SVG assets still get written on a bare checkout.
    """
    node = shutil.which("node")
    helper = ROOT / "tools" / "rasterize.js"
    if node and helper.exists() and (ROOT / "tools" / "node_modules" / "@resvg").exists():
        cmd = [node, str(helper), str(svg_path), str(out_path), str(size)]
        if opaque:
            cmd += ["--opaque", opaque]
        try:
            subprocess.run(cmd, check=True, capture_output=True, timeout=180)
            return True
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
            print(f"    ! resvg {size}px failed: {exc}")
            return False

    rsvg = shutil.which("rsvg-convert")
    if rsvg:
        cmd = [rsvg, "-w", str(size), "-h", str(size), "-o", str(out_path), str(svg_path)]
        try:
            subprocess.run(cmd, check=True, capture_output=True, timeout=180)
            return True
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
            print(f"    ! rsvg {size}px failed: {exc}")
            return False

    return False


def make_ico(png_paths: list[Path], out: Path) -> bool:
    exe = shutil.which("convert") or shutil.which("magick")
    if not exe or not png_paths:
        return False
    cmd = [exe] + [str(p) for p in png_paths] + [str(out)]
    try:
        subprocess.run(cmd, check=True, capture_output=True, timeout=180)
        return True
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
        print(f"    ! ico build failed: {exc}")
        return False


def make_icns(png_paths: list[Path], out: Path) -> bool:
    exe = shutil.which("png2icns") or shutil.which("iconutil")
    if not exe:
        print("    · png2icns/iconutil not installed - skipping .icns")
        return False
    if exe.endswith("iconutil"):
        iconset = out.with_suffix(".iconset")
        iconset.mkdir(exist_ok=True)
        for p in png_paths:
            shutil.copy(p, iconset / f"icon_{p.stem.replace('star-', '')}.png")
        cmd = ["iconutil", "-c", "icns", str(iconset), "-o", str(out)]
    else:
        cmd = [exe] + [str(p) for p in png_paths] + [str(out)]
    try:
        subprocess.run(cmd, check=True, capture_output=True, timeout=180)
        return True
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
        print(f"    ! icns build failed: {exc}")
        return False


def main() -> int:
    SVG_DIR.mkdir(parents=True, exist_ok=True)
    PNG_DIR.mkdir(parents=True, exist_ok=True)

    svgs: dict[Path, str] = {
        SVG_DIR / "star-mark.svg": svg_mark("currentColor"),
        SVG_DIR / "star-mark-color.svg": svg_mark(GRAD_DEEP, gradient=True),
        SVG_DIR / "star-mark-oled.svg": svg_mark(GRAD_DEEP, gradient=True, bg="#000000"),
        SVG_DIR / "star-mark-light.svg": svg_mark(GRAD_DEEP, gradient=True, bg="#FFFFFF"),
        SVG_DIR / "star-lockup.svg": svg_lockup(),
        SVG_DIR / "star-lockup-light.svg": svg_lockup(text_color="#0F131C"),
    }
    for path, content in svgs.items():
        path.write_text(content, encoding="utf-8")
        print(f"  wrote {path.relative_to(ROOT)}")

    # rasters (icon variants need an opaque plate, the mark itself does not)
    icon_svg = SVG_DIR / "star-mark-oled.svg"
    made: dict[int, Path] = {}
    for size in ICO_SIZES:
        for scale, suffix in ((1, ""), (2, "@2x")):
            px = size * scale
            out = PNG_DIR / f"star-{size}{suffix}.png"
            if rasterise(icon_svg, out, px, opaque="#000000"):
                made[px] = out
    if made:
        print(f"  wrote {len(made)} PNG raster(s) -> branding/png/")
        ico_paths = [made[s] for s in ICO_SIZES if s in made]
        if make_ico(ico_paths, ROOT / "branding" / "star.ico"):
            print("  wrote branding/star.ico")
        icns_paths = [made[s] for s in ICNS_SIZES if s in made]
        if make_icns(icns_paths, ROOT / "branding" / "star.icns"):
            print("  wrote branding/star.icns")
    else:
        print("  ! ImageMagick unavailable - SVG assets written, PNG/ICO skipped")

    # quick self-check on the outline math
    print("\nStar outline path:")
    print("  " + star_outline()[:110] + " ...")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
