#!/usr/bin/env node
/**
 * Star Client - prototype screenshot + smoke test harness.
 *
 * Renders prototype/index.html in headless Chrome, drives the UI the way a
 * user would, captures PNGs and fails loudly on any console error or failed
 * network request. Used as a visual check while porting the design to Qt.
 *
 * Usage:
 *   node tools/screenshot.js                       # all shots -> tools/shots
 *   node tools/screenshot.js --out /tmp/shots
 *   node tools/screenshot.js --only oled-home
 *   node tools/screenshot.js --url file:///path/to/index.html
 */

const fs = require("fs");
const path = require("path");
const puppeteer = require("puppeteer");

const ROOT = path.resolve(__dirname, "..");
const DEFAULT_URL =
  "file://" + path.join(ROOT, "prototype", "index.html");
const VIEWPORT = { width: 1440, height: 900, deviceScaleFactor: 1 };

function parseArgs(argv) {
  const out = { out: path.join(__dirname, "shots"), only: null, url: DEFAULT_URL };
  for (let i = 2; i < argv.length; i++) {
    if (argv[i] === "--out") out.out = argv[++i];
    else if (argv[i] === "--only") out.only = argv[++i];
    else if (argv[i] === "--url") out.url = argv[++i];
  }
  return out;
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

/**
 * Find a usable Chrome. Order:
 *   1. CHROME_PATH / PUPPETEER_EXECUTABLE_PATH env vars
 *   2. puppeteer's own managed download
 *   3. @sparticuz/chromium (npm-packaged build, for sandboxes where the
 *      Chrome CDN is unreachable but the npm registry is not)
 */
async function resolveChrome() {
  if (process.env.CHROME_PATH) return process.env.CHROME_PATH;
  if (process.env.PUPPETEER_EXECUTABLE_PATH) return process.env.PUPPETEER_EXECUTABLE_PATH;
  try {
    const p = await puppeteer.executablePath();
    if (p && fs.existsSync(p)) return p;
  } catch (err) {
    /* fall through */
  }
  try {
    const mod = require("@sparticuz/chromium");
    const chromium = mod.default || mod;
    const exe = await chromium.executablePath();

    // This build ships its shared libraries as a separate tar.br (it targets
    // bare serverless images). Extract them and point the loader at them, or
    // the binary dies with "libnspr4.so: cannot open shared object file".
    // Done by hand rather than via the package's inflate(), which silently
    // produced nothing here.
    const libDir = "/tmp/starclient-chromium-libs";
    const marker = path.join(libDir, "lib", "libnspr4.so");
    if (!fs.existsSync(marker)) {
      const tarBr = path.join(
        ROOT, "tools", "node_modules", "@sparticuz", "chromium", "bin", "al2023.tar.br"
      );
      if (fs.existsSync(tarBr)) {
        const zlib = require("zlib");
        const { spawnSync } = require("child_process");
        const tarPath = path.join("/tmp", "starclient-al2023.tar");
        fs.mkdirSync(libDir, { recursive: true });
        fs.writeFileSync(tarPath, zlib.brotliDecompressSync(fs.readFileSync(tarBr)));
        const res = spawnSync("tar", ["-xf", tarPath, "-C", libDir], { encoding: "utf8" });
        if (res.status !== 0) console.error("  ! lib extraction failed:", res.stderr);
        fs.unlinkSync(tarPath);
      } else {
        console.error("  ! chromium lib tarball not found; browser may not start");
      }
    }
    if (fs.existsSync(path.join(libDir, "lib"))) {
      process.env.LD_LIBRARY_PATH = [
        path.join(libDir, "lib"),
        process.env.LD_LIBRARY_PATH || "",
      ].filter(Boolean).join(":");
    }
    console.log(`Using @sparticuz/chromium at ${exe}`);
    return exe;
  } catch (err) {
    return null;
  }
}

async function main() {
  const args = parseArgs(process.argv);
  fs.mkdirSync(args.out, { recursive: true });

  const executablePath = await resolveChrome();
  const launchOpts = {
    headless: true,
    args: [
      "--no-sandbox",
      "--disable-setuid-sandbox",
      "--disable-dev-shm-usage",
      "--disable-gpu",
      "--font-render-hinting=none",
      "--allow-file-access-from-files",
    ],
  };
  if (executablePath) launchOpts.executablePath = executablePath;

  const browser = await puppeteer.launch(launchOpts);
  const page = await browser.newPage();
  await page.setViewport(VIEWPORT);

  const problems = [];
  page.on("console", (msg) => {
    if (msg.type() === "error") problems.push(`console.error: ${msg.text()}`);
    if (msg.type() === "warning" && /404|Failed to load/i.test(msg.text()))
      problems.push(`console.warn: ${msg.text()}`);
  });
  page.on("pageerror", (err) => problems.push(`pageerror: ${err.message}`));
  page.on("requestfailed", (req) =>
    problems.push(`requestfailed: ${req.url()} (${req.failure()?.errorText})`)
  );

  await page.goto(args.url, { waitUntil: "networkidle0" });
  await sleep(900); // let the first-run wizard appear, then dismiss it

  const shots = [];
  const shot = async (name, selector) => {
    if (args.only && args.only !== name) return;
    const target = selector ? await page.$(selector) : null;
    const file = path.join(args.out, `${name}.png`);
    await (target ? target.screenshot({ path: file }) : page.screenshot({ path: file }));
    shots.push(name);
  };

  const click = async (selector) => {
    await page.waitForSelector(selector, { timeout: 5000 });
    await page.click(selector);
    await sleep(450);
  };

  const setTheme = async (theme, accent) => {
    await page.evaluate(
      (t, a) => {
        document.documentElement.setAttribute("data-theme", t);
        if (a) document.documentElement.setAttribute("data-accent", a);
      },
      theme,
      accent || null
    );
    await sleep(320);
  };

  // --- wizard --------------------------------------------------------- #
  // opened explicitly: the first-run auto-open depends on sessionStorage,
  // which some file:// configurations block.
  await click("#quickStartChip");
  await shot("01-wizard-welcome");

  await click('[data-wstep="2"]');
  await shot("02-wizard-appearance");

  await click('[data-wstep="4"]');
  await shot("03-wizard-memory");

  await click('[data-wstep="5"]');
  await shot("04-wizard-summary");

  await click("#wizardSkip");
  await sleep(500);

  // --- OLED dark (signature look) ------------------------------------- #
  await setTheme("oled", "starlight");
  await shot("10-oled-star-start");

  await click('[data-goto="instances"]');
  await shot("11-oled-instances");

  // scoped: the home page holds a hidden duplicate of this card
  await click('#allInstances .inst[data-instance="star"]');
  await shot("12-oled-instance-overview");

  await click('.tab[data-tab="starmenu"]');
  await shot("13-oled-star-menu");

  await click('.tab[data-tab="mods"]');
  await shot("14-oled-mods");

  await click('.tab[data-tab="logs"]');
  await click("#instLaunch");
  await sleep(2200);
  await shot("15-oled-console");

  await click('.tab[data-tab="settings"]');
  await shot("16-oled-instance-settings");

  await click('[data-goto="browse"]');
  await shot("17-oled-browse");

  await click('[data-goto="accounts"]');
  await shot("18-oled-accounts");

  await click('[data-goto="settings"]');
  await shot("19-oled-settings-appearance");

  await click('.settings__link[data-panel="memory"]');
  await shot("20-oled-settings-memory");

  await click('.settings__link[data-panel="java"]');
  await shot("21-oled-settings-java");

  await click('.settings__link[data-panel="about"]');
  await shot("22-oled-settings-about");

  // --- Midnight ------------------------------------------------------- #
  await setTheme("dark", "aurora");
  await shot("30-midnight-star-start");

  // --- Daylight ------------------------------------------------------- #
  await setTheme("light", "nova");
  await shot("40-light-star-start");
  await click('[data-goto="settings"]');
  await shot("41-light-settings-appearance");

  // --- density / accent variants -------------------------------------- #
  await setTheme("light", "emerald");
  await click('[data-goto="instances"]');
  await shot("42-light-instances-emerald");

  // --- RAM interaction smoke test ------------------------------------- #
  await setTheme("oled", "starlight");
  await click('[data-goto="settings"]');
  await click('.settings__link[data-panel="memory"]');
  await page.evaluate(() => {
    const slider = document.querySelector("#ram");
    slider.value = 2048;
    slider.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await sleep(300);
  const ramText = await page.$eval("#ramValue", (el) => el.textContent);
  const notice = await page.$eval("#ramNoticeBody", (el) => el.textContent.trim());
  if (ramText !== "2048") problems.push(`RAM slider did not update label (got "${ramText}")`);
  await shot("50-oled-ram-low-warning");

  // GC card selection
  await click('.settings__link[data-panel="java"]');
  await click('.radio-card[data-gc="zgc"]');
  const gcLabel = await page.$eval("#sumCollector", (el) => el.textContent);
  if (gcLabel !== "ZGC") problems.push(`GC selection did not propagate (got "${gcLabel}")`);
  await shot("51-oled-gc-selection");

  await browser.close();

  console.log(`\nCaptured ${shots.length} screenshot(s) -> ${args.out}`);
  shots.forEach((s) => console.log("  " + s + ".png"));

  if (problems.length) {
    console.error(`\n${problems.length} problem(s) detected:`);
    [...new Set(problems)].forEach((p) => console.error("  ! " + p));
    process.exit(1);
  }
  console.log("\nNo console errors, page errors or failed requests.");
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
