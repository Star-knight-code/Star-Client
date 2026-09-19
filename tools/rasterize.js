#!/usr/bin/env node
/**
 * SVG -> PNG rasteriser for Star Client branding assets.
 *
 * Uses @resvg/resvg-js (prebuilt, no system deps). Kept separate from
 * gen_logo.py so the Python side stays dependency-free: the generator
 * shells out to this script and degrades gracefully if node is missing.
 *
 * Usage:
 *   node tools/rasterize.js <in.svg> <out.png> <width> [--opaque #000000]
 */

const fs = require("fs");
const path = require("path");
const { Resvg } = require("@resvg/resvg-js");

function main(argv) {
  const args = argv.slice(2).filter((a) => !a.startsWith("--"));
  if (args.length < 3) {
    console.error("usage: rasterize.js <in.svg> <out.png> <width> [--opaque color]");
    return 2;
  }
  const [inSvg, outPng, widthRaw] = args;
  const width = parseInt(widthRaw, 10);
  const opaqueIdx = argv.indexOf("--opaque");
  const opaque = opaqueIdx !== -1 ? argv[opaqueIdx + 1] : null;

  const svg = fs.readFileSync(inSvg, "utf8");
  const resvg = new Resvg(svg, {
    fitTo: { mode: "width", value: width },
    background: opaque || undefined,
    font: { loadSystemFonts: true },
  });
  const png = resvg.render().asPng();
  fs.mkdirSync(path.dirname(path.resolve(outPng)), { recursive: true });
  fs.writeFileSync(outPng, png);
  return 0;
}

process.exit(main(process.argv));
