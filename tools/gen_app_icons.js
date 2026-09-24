#!/usr/bin/env node
/**
 * Rebrands the program_info brand assets with the Star Client mark.
 *
 * The upstream launcher ships its identity as a set of SVGs, a 256px PNG,
 * a multi-resolution .ico and a macOS .icns. All of them have to become the
 * four-pointed star, otherwise the old logo leaks into the taskbar, the
 * About dialog and the macOS dock.
 *
 * Source of truth: branding/svg/star-mark-oled.svg (the star on a dark plate).
 *
 * Usage:
 *   node tools/gen_app_icons.js
 */

const fs = require("fs");
const path = require("path");
const { Resvg } = require("@resvg/resvg-js");

const ROOT = path.resolve(__dirname, "..");
const INFO = path.join(ROOT, "program_info");
const STAR_SVG = path.join(ROOT, "branding", "svg", "star-mark-oled.svg");

/** Renders the star mark to a PNG buffer at `size` px on a black plate. */
function renderPng(size) {
  const svg = fs.readFileSync(STAR_SVG, "utf8");
  const resvg = new Resvg(svg, {
    fitTo: { mode: "width", value: size },
    background: "#000000",
  });
  return resvg.render().asPng();
}

/** Renders at `size` but on a transparent plate (for monochrome/desktop art). */
function renderPngTransparent(size) {
  const svg = fs.readFileSync(
    path.join(ROOT, "branding", "svg", "star-mark-color.svg"),
    "utf8"
  );
  const resvg = new Resvg(svg, { fitTo: { mode: "width", value: size } });
  return resvg.render().asPng();
}

/**
 * Builds a modern .icns containing PNG-encoded entries.
 * Apple's containers are just a flat list of typed chunks, so this is a
 * straight byte-assembly - no external tooling needed.
 */
function buildIcns(entries) {
  const chunks = [];
  for (const [type, png] of entries) {
    const header = Buffer.alloc(8);
    header.write(type, 0, 4, "ascii");
    header.writeUInt32BE(png.length + 8, 4);
    chunks.push(header, png);
  }
  const body = Buffer.concat(chunks);
  const header = Buffer.alloc(8);
  header.write("icns", 0, 4, "ascii");
  header.writeUInt32BE(body.length + 8, 4);
  return Buffer.concat([header, body]);
}

/**
 * Builds a multi-resolution .ico holding PNG frames.
 * Valid for Windows Vista and newer, which is everything we target.
 */
function buildIco(frames) {
  const count = frames.length;
  const dir = Buffer.alloc(6);
  dir.writeUInt16LE(0, 0); // reserved
  dir.writeUInt16LE(1, 2); // type: icon
  dir.writeUInt16LE(count, 4);

  const entries = [];
  let offset = 6 + count * 16;
  for (const { size, png } of frames) {
    const e = Buffer.alloc(16);
    e.writeUInt8(size >= 256 ? 0 : size, 0); // width  (0 means 256)
    e.writeUInt8(size >= 256 ? 0 : size, 1); // height
    e.writeUInt8(0, 2); // palette
    e.writeUInt8(0, 3); // reserved
    e.writeUInt16LE(1, 4); // colour planes
    e.writeUInt16LE(32, 6); // bits per pixel
    e.writeUInt32LE(png.length, 8);
    e.writeUInt32LE(offset, 12);
    offset += png.length;
    entries.push(e);
  }
  return Buffer.concat([dir, ...entries, ...frames.map((f) => f.png)]);
}

function write(file, buf) {
  const full = path.join(INFO, file);
  fs.writeFileSync(full, buf);
  console.log(`  ${file} (${(buf.length / 1024).toFixed(1)} KB)`);
}

function main() {
  console.log("\nGenerating Star Client application icons\n");

  // --- the four-pointed star, on a black plate ---------------------------
  const sizes = [16, 24, 32, 48, 64, 128, 256, 512, 1024];
  const pngs = {};
  for (const s of sizes) pngs[s] = renderPng(s);

  // 256px PNG referenced by the packaging scripts
  write("org.starclient.StarClient_256.png", pngs[256]);

  // Windows icon
  write("starclient.ico", buildIco(
    [16, 24, 32, 48, 64, 128, 256].map((size) => ({ size, png: pngs[size] }))
  ));

  // macOS icon (icns types: ic07=128, ic08=256, ic09=512, ic10=1024,
  //                        ic11=32@2x, ic12=64@2x, ic13=256@2x, ic14=512@2x)
  write("starclient.icns", buildIcns([
    ["ic11", pngs[32]],
    ["ic12", pngs[64]],
    ["ic07", pngs[128]],
    ["ic13", pngs[256]],
    ["ic08", pngs[256]],
    ["ic14", pngs[512]],
    ["ic09", pngs[512]],
    ["ic10", pngs[1024]],
  ]));

  // --- the scalable app icon --------------------------------------------
  const starSvg = fs.readFileSync(STAR_SVG, "utf8");
  fs.writeFileSync(path.join(INFO, "org.starclient.StarClient.svg"), starSvg);
  console.log("  org.starclient.StarClient.svg");

  // window/dock logo variants used by the launcher chrome
  for (const name of [
    "org.starclient.StarClient.logo.svg",
    "org.starclient.StarClient.logo-darkmode.svg",
    "org.starclient.StarClient.bigsur.svg",
    "org.starclient.StarClient.Source.svg",
    "org.starclient.StarClient.logo.source.svg",
    "org.starclient.StarClient.Social.svg",
    "starclient-monochrome.Source.svg",
  ]) {
    fs.writeFileSync(path.join(INFO, name), starSvg);
  }
  console.log("  6 logo/source SVG variants -> star mark");

  // macOS 26 ".icon" asset: block.svg is the background layer, rainbow.svg
  // the foreground glass layer, both in a 12.7 x 12.7 viewBox.
  const ICON_DIR = path.join(INFO, "StarClient.icon", "Assets");
  if (fs.existsSync(ICON_DIR)) {
    const scale = (12.7 / 256).toFixed(6);
    const svgOpen = (id, extra) =>
      `<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n` +
      `<svg width="48" height="48" version="1.1" viewBox="0 0 12.7 12.7" id="${id}"\n` +
      `  xmlns="http://www.w3.org/2000/svg">\n`;
    const star = (fill) =>
      `<g transform="scale(${scale})"><path d="M128 16Q150.4 105.6 240 128Q150.4 150.4 128 240` +
      `Q105.6 150.4 16 128Q105.6 105.6 128 16Z" fill="${fill}"/></g>\n</svg>\n`;

    // background layer: transparent, the plate colour comes from icon.json
    fs.writeFileSync(
      path.join(ICON_DIR, "block.svg"),
      svgOpen("svg_block") + '<g transform="scale(0)"/></svg>\n'
    );
    // foreground layer: the star, in the brand gradient
    fs.writeFileSync(
      path.join(ICON_DIR, "rainbow.svg"),
      svgOpen("svg_star") +
        `  <defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1">` +
        `<stop offset="0" stop-color="#8B7BFF"/>` +
        `<stop offset="0.55" stop-color="#6450F0"/>` +
        `<stop offset="1" stop-color="#4FC3F7"/></linearGradient></defs>\n` +
        star("url(#g)")
    );
    console.log("  StarClient.icon/Assets -> star (block.svg, rainbow.svg)");
  }

  console.log("\nDone.\n");
}

main();
