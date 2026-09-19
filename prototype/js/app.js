/* ==========================================================================
 * Star Client - prototype behaviour
 * --------------------------------------------------------------------------
 * Pure vanilla JS, no framework: this page exists to prove out the UI/UX
 * that gets ported to Qt Widgets. The state object below maps 1:1 onto the
 * settings the launcher will persist, which keeps the port mechanical.
 * ========================================================================== */
(function () {
  "use strict";

  const D = window.StarData;
  const $ = (sel, root = document) => root.querySelector(sel);
  const $$ = (sel, root = document) => Array.from(root.querySelectorAll(sel));

  const ACCENTS = [
    { id: "starlight", color: "#7C6BFF", label: "Starlight" },
    { id: "aurora", color: "#35D6ED", label: "Aurora" },
    { id: "nova", color: "#FF5FA2", label: "Nova" },
    { id: "solar", color: "#FFB63D", label: "Solar" },
    { id: "emerald", color: "#3ED598", label: "Emerald" },
    { id: "ember", color: "#FF6B4A", label: "Ember" },
  ];

  const THEMES = [
    { id: "oled", label: "OLED Black", tag: "True black" },
    { id: "dark", label: "Midnight", tag: "Soft dark" },
    { id: "light", label: "Daylight", tag: "Light" },
    { id: "system", label: "Match system", tag: "Auto" },
  ];

  const state = {
    theme: "oled",
    accent: "starlight",
    ram: 6144,
    gc: "g1gc",
    starMenu: true,
    activeInstance: "star",
    wizardStep: 1,
  };

  /* ----------------------------------------------------------- theme ---- */
  function applyTheme() {
    const root = document.documentElement;
    const resolved =
      state.theme === "system"
        ? (window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light")
        : state.theme;
    root.setAttribute("data-theme", resolved);
    root.setAttribute("data-accent", state.accent);

    const isDark = resolved !== "light";
    $("#themeToggle svg use").setAttribute("href", isDark ? "#i-moon" : "#i-sun");
    $("#themeToggle").setAttribute(
      "data-tip",
      `Switch to ${isDark ? "light" : "OLED black"}`
    );

    $$(".theme-option").forEach((el) =>
      el.classList.toggle("is-on", el.dataset.themeValue === state.theme)
    );
    $$(".accent-swatch").forEach((el) =>
      el.classList.toggle("is-on", el.dataset.accent === state.accent)
    );

    const themeLabel = (THEMES.find((t) => t.id === state.theme) || THEMES[0]).label;
    const accentLabel = (ACCENTS.find((a) => a.id === state.accent) || ACCENTS[0]).label;
    $("#sumTheme").textContent = `${themeLabel} · ${accentLabel}`;
  }

  function cycleTheme() {
    // the rail button is a one-click day/night flip
    const dark = ["oled", "dark"];
    const current = state.theme === "system"
      ? (window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light")
      : state.theme;
    state.theme = dark.includes(current) ? "light" : "oled";
    applyTheme();
    toast(`Theme: ${(THEMES.find((t) => t.id === state.theme) || {}).label}`, "info");
  }

  function buildAccentPicker(target, onPick) {
    target.innerHTML = ACCENTS.map(
      (a) =>
        `<button class="accent-swatch${a.id === state.accent ? " is-on" : ""}" ` +
        `data-accent="${a.id}" style="background:linear-gradient(140deg,${a.color},${a.color}cc)" ` +
        `title="${a.label}" aria-label="${a.label}"></button>`
    ).join("");
    $$(".accent-swatch", target).forEach((el) =>
      el.addEventListener("click", () => {
        state.accent = el.dataset.accent;
        applyTheme();
        onPick && onPick();
      })
    );
  }

  function buildThemePicker(target) {
    target.innerHTML = THEMES.map(
      (t) =>
        `<button class="theme-option${t.id === state.theme ? " is-on" : ""}" data-theme-value="${t.id}">` +
        `<div class="theme-option__label">${t.label} <span class="theme-option__tag">${t.tag}</span></div>` +
        `</button>`
    ).join("");
    $$(".theme-option", target).forEach((el) =>
      el.addEventListener("click", () => {
        state.theme = el.dataset.themeValue;
        applyTheme();
      })
    );
  }

  /* ------------------------------------------------------------ pages ---- */
  function goto(page, opts) {
    opts = opts || {};
    const target = $("#page-" + page);
    if (!target) return;
    $$(".page").forEach((p) => p.classList.remove("is-active"));
    target.classList.add("is-active");
    $$(".rail__btn").forEach((b) =>
      b.setAttribute("aria-current", b.dataset.goto === page ? "page" : "false")
    );
    $("#content").scrollTop = 0;
    if (opts.instanceId) selectInstance(opts.instanceId);
  }

  /* -------------------------------------------------------- instances ---- */
  function instanceCard(inst) {
    const gc = (D.GC.find((g) => g.id === inst.gc) || D.GC[0]).label;
    return `
      <article class="inst" data-instance="${inst.id}">
        <div class="inst__banner">
          <svg class="inst__icon" viewBox="0 0 64 64"><use href="#${inst.art}"/></svg>
        </div>
        <button class="btn btn--ghost btn--icon btn--sm inst__menu" data-toast="Instance actions" aria-label="Instance actions">
          <svg><use href="#i-more"/></svg>
        </button>
        <div class="inst__body">
          <h3 class="inst__name">${inst.name}</h3>
          <p class="inst__sub">${inst.played}</p>
          <div class="inst__badges">
            <span class="badge badge--accent">${inst.loader}</span>
            <span class="badge">${inst.version}</span>
            <span class="badge">${inst.mods} mods</span>
            <span class="badge">${D.fmtMB(inst.ram)}</span>
            <span class="badge badge--info">${gc}</span>
          </div>
        </div>
        <button class="inst__play" data-launch="${inst.id}" aria-label="Launch ${inst.name}">
          <svg><use href="#i-play"/></svg>
        </button>
      </article>`;
  }

  function renderInstances() {
    $("#homeInstances").innerHTML = D.INSTANCES.slice(0, 3).map(instanceCard).join("");
    $("#allInstances").innerHTML = D.INSTANCES.map(instanceCard).join("");
    $$(".inst").forEach((card) => {
      card.addEventListener("click", (e) => {
        if (e.target.closest("[data-launch]")) return;
        selectInstance(card.dataset.instance);
        goto("instance");
      });
    });
    $$("[data-launch]").forEach((btn) =>
      btn.addEventListener("click", (e) => {
        e.stopPropagation();
        const inst = D.INSTANCES.find((i) => i.id === btn.dataset.launch);
        launch(inst);
      })
    );
  }

  function selectInstance(id) {
    const inst = D.INSTANCES.find((i) => i.id === id) || D.INSTANCES[0];
    state.activeInstance = inst.id;
    $$(".inst").forEach((c) => c.classList.toggle("is-selected", c.dataset.instance === inst.id));

    $("#instTitle").textContent = inst.name;
    $("#instSub").textContent = `Minecraft ${inst.version} · ${inst.loader}`;
    $("#instName").textContent = inst.name;
    $("#instModCount").textContent = `${inst.mods} mods`;
    $("#miniRam").textContent = `${inst.ram} MB`;
    $("#miniGc").textContent = (D.GC.find((g) => g.id === inst.gc) || D.GC[0]).label;
    $("#sumRam").textContent = `${inst.ram} MB`;
    $("#sumGc").textContent = (D.GC.find((g) => g.id === inst.gc) || D.GC[0]).label;
    $(".launch-strip__icon use").setAttribute("href", "#" + inst.art);

    state.ram = inst.ram;
    state.gc = inst.gc;
    syncRamUI();
    syncGcUI();
    renderMods();
  }

  function renderMods() {
    $("#modTable").innerHTML = D.MODS.map(
      (m) => `
      <tr>
        <td>${m.name}</td>
        <td class="mono muted">${m.version}</td>
        <td>${m.source}</td>
        <td>${m.star ? '<span class="badge badge--accent">Star Menu</span>' : '<span class="muted">—</span>'}</td>
        <td class="right"><button class="btn btn--ghost btn--sm" data-toast="${m.name} ${m.star ? "disabled" : "enabled"}">${m.star ? "Disable" : "Enable"}</button></td>
      </tr>`
    ).join("");
  }

  /* --------------------------------------------------------- browse ------ */
  function renderBrowse() {
    $("#browseGrid").innerHTML = D.BROWSE.map(
      (b) => `
      <article class="card is-interactive">
        <div class="card__head" style="margin-bottom:var(--sc-space-4)">
          <div class="mod-row__icon"><svg><use href="#${b.icon}"/></svg></div>
          <span class="badge">${b.tag}</span>
        </div>
        <h3 class="card__title">${b.name}</h3>
        <p class="card__desc" style="margin:var(--sc-space-3) 0 var(--sc-space-5)">${b.desc}</p>
        <div class="inline">
          <span class="muted mono">${b.downloads}</span>
          <button class="btn btn--primary btn--sm right" data-toast="Installing ${b.name}">Install</button>
        </div>
      </article>`
    ).join("");
  }

  /* ------------------------------------------------------- range paint --- */
  /** Fills the travelled part of a range track (CSS cannot read :value). */
  function paintRange(el) {
    if (!el) return;
    const min = Number(el.min || 0);
    const max = Number(el.max || 100);
    const pct = max === min ? 0 : ((Number(el.value) - min) / (max - min)) * 100;
    el.style.setProperty("--sc-range-fill", pct.toFixed(2) + "%");
  }

  function paintAllRanges() {
    $$('input[type="range"]').forEach(paintRange);
  }

  /* ------------------------------------------------------ RAM control ---- */
  function syncRamUI() {
    const mb = state.ram;
    const inst = D.INSTANCES.find((i) => i.id === state.activeInstance) || D.INSTANCES[0];

    ["#ram", "#quickRam", "#wizardRam"].forEach((sel) => {
      const el = $(sel);
      if (el) {
        el.value = mb;
        paintRange(el);
      }
    });
    ["#ramValue", "#quickRamValue", "#wizardRamValue"].forEach((sel) => {
      const el = $(sel);
      if (el) el.textContent = mb;
    });
    ["#ramNumber", "#quickRamNumber"].forEach((sel) => {
      const el = $(sel);
      if (el) el.value = mb;
    });
    $("#heroRam").textContent = mb + " MB";

    $$(".chip[data-ram]").forEach((chip) =>
      chip.classList.toggle("is-on", Number(chip.dataset.ram) === mb)
    );

    renderArgsPreview();
    const verdict = D.ramVerdict(mb, inst.mods);
    const notice = $("#ramNotice");
    notice.className = "banner banner--" + verdict.state;
    $("#ramNoticeBody").innerHTML = `<b>${verdict.title}</b> ${verdict.body}`;
    $("#ramNotice svg use").setAttribute(
      "href",
      verdict.state === "success" ? "#i-check-circle" : "#i-info"
    );

    $("#sumMemory").textContent = mb + " MB";
    $("#instModCount").textContent = `${inst.mods} mods`;
  }

  function setRam(value, silent) {
    const v = Math.max(512, Math.min(32768, Number(value) || 512));
    state.ram = Math.round(v / 256) * 256;
    syncRamUI();
    if (!silent) {
      const inst = D.INSTANCES.find((i) => i.id === state.activeInstance) || D.INSTANCES[0];
      inst.ram = state.ram;
      $("#miniRam").textContent = state.ram + " MB";
    }
  }

  function autoRam() {
    const inst = D.INSTANCES.find((i) => i.id === state.activeInstance) || D.INSTANCES[0];
    // the real launcher sizes from mod count, render distance and physical RAM
    const base = 2048 + inst.mods * 48;
    const capped = Math.min(base, Math.floor(D.PHYSICAL_MB * 0.5));
    setRam(Math.round(capped / 256) * 256);
    toast(`Auto-tuned to ${state.ram} MB for ${inst.mods} mods`, "success");
  }

  /* ------------------------------------------------------------- GC ------ */
  function gcCard(gc, checked) {
    return `
      <label class="radio-card${checked ? " is-on" : ""}" data-gc="${gc.id}">
        <input type="radio" name="gc" ${checked ? "checked" : ""}>
        <span class="radio-card__dot"></span>
        <span class="radio-card__body">
          <span class="radio-card__name">${gc.label} ${gc.tag ? `<span class="badge badge--accent">${gc.tag}</span>` : ""}</span>
          <span class="radio-card__desc">${gc.desc}</span>
        </span>
      </label>`;
  }

  /** Renders the flags the launcher would actually pass for the current GC. */
  function renderArgsPreview() {
    const gc = D.GC.find((g) => g.id === state.gc) || D.GC[0];
    const preview = $("#gcArgsPreview");
    if (!preview) return;
    const flags = gc.args
      .split(" ")
      .map((f) => `<b>${f}</b>`)
      .join(" ");
    // -Xmx / -Xms are always present, so show them for a realistic command line
    preview.innerHTML = `-Xms${Math.round(state.ram / 2)}M -Xmx${state.ram}M\n${flags}`;
    const badge = $("#gcJavaBadge");
    if (badge) badge.textContent = "Java " + gc.minJava + "+";
    const legacy = $("#jvmArgsExtra");
    if (legacy) legacy.textContent = $("#jvmArgs") ? $("#jvmArgs").value.trim() : "";
  }

  function renderGc(targets) {
    targets.forEach((sel) => {
      const el = $(sel);
      if (!el) return;
      el.innerHTML = D.GC.map((g) => gcCard(g, g.id === state.gc)).join("");
      $$(".radio-card", el).forEach((card) =>
        card.addEventListener("click", () => {
          state.gc = card.dataset.gc;
          const inst = D.INSTANCES.find((i) => i.id === state.activeInstance);
          if (inst) inst.gc = state.gc;
          syncGcUI();
        })
      );
    });
  }

  const GC_HINTS = {
    g1gc: "Best all-round choice for 4-8 GB heaps.",
    zgc: "Needs Java 17+ and 8 GB+ to beat G1GC.",
    "zgc-gen": "Java 21+ only. Lowest pauses at high frame rates.",
    shenandoah: "Good when ZGC stutters on smaller heaps.",
    serial: "Smallest footprint, best for vanilla and low-spec machines.",
    parallel: "Highest throughput but longer pauses.",
  };

  function syncGcUI() {
    const gc = D.GC.find((g) => g.id === state.gc) || D.GC[0];
    $$(".radio-card[data-gc]").forEach((c) => c.classList.toggle("is-on", c.dataset.gc === state.gc));
    $("#miniGc").textContent = gc.label;
    $("#heroGc").textContent = gc.label;
    $("#sumGc").textContent = gc.label;
    $("#sumCollector").textContent = gc.label;
    $("#quickGcHint").textContent = GC_HINTS[state.gc] || "";
    const select = $("#quickGc");
    if (select && select.value !== state.gc) select.value = state.gc;
    renderArgsPreview();
  }

  /* ------------------------------------------------------------ tabs ----- */
  function initTabs() {
    $$(".tab").forEach((tab) =>
      tab.addEventListener("click", () => {
        $$(".tab").forEach((t) => t.classList.toggle("is-on", t === tab));
        $$(".tabpanel").forEach((p) =>
          p.classList.toggle("is-active", p.dataset.panel === tab.dataset.tab)
        );
      })
    );

    $$(".settings__link").forEach((link) =>
      link.addEventListener("click", () => {
        $$(".settings__link").forEach((l) => l.classList.toggle("is-on", l === link));
        $$(".settings__panel").forEach((p) =>
          p.classList.toggle("is-active", p.dataset.panel === link.dataset.panel)
        );
      })
    );
  }

  /* --------------------------------------------------------- launching --- */
  function launch(inst) {
    if (!inst) inst = D.INSTANCES.find((i) => i.id === state.activeInstance) || D.INSTANCES[0];
    toast(`Launching ${inst.name} · ${inst.ram} MB · ${(D.GC.find((g) => g.id === inst.gc) || D.GC[0]).label}`, "success");
    selectInstance(inst.id);
    goto("instance");
    // simulate the console filling in
    const panel = $('.tab[data-tab="logs"]');
    const log = $("#logView");
    if (log && panel) {
      panel.click();
      log.innerHTML = "";
      streamLog(inst);
    }
  }

  const LOG_LINES = [
    ["i", "Star Client 21.1.0 starting"],
    ["t", "resolving instance 'Star Client'"],
    ["i", "java: Eclipse Temurin 21.0.4+7 LTS (~/.local/share/StarClient/java/temurin-21)"],
    ["i", "collector preset: G1GC (18 flags applied)"],
    ["s", "classpath built · 87 mods · 214 jars"],
    ["t", "launching net.fabricmc.loader.impl.launch.knot.KnotClient"],
    ["i", "[main/INFO] Loading Minecraft 1.21.1 with Fabric Loader 0.16.9"],
    ["s", "[Render thread/INFO] Backend: OpenGL 4.6 (independent)"],
    ["i", "[main/INFO] Applied Star Menu presets: 6 enabled, 0 skipped"],
    ["w", "[main/WARN] Connection watchdog skipped: server reports anti-cheat"],
    ["s", "[Render thread/INFO] Loaded 87 mods in 6.482 s"],
    ["i", "[main/INFO] Time to load chunks: 1.204 s"],
    ["s", "Game ready · 412 FPS average (last 60 s)"],
  ];

  function streamLog(inst) {
    const log = $("#logView");
    let i = 0;
    const timer = setInterval(() => {
      if (i >= LOG_LINES.length) {
        clearInterval(timer);
        return;
      }
      const [kind, text] = LOG_LINES[i++];
      const stamp = new Date().toLocaleTimeString("en-GB", { hour12: false });
      log.innerHTML += `<div class="${kind}"><span class="t">[${stamp}]</span> ${text}</div>`;
      log.scrollTop = log.scrollHeight;
    }, 150);
  }

  /* ---------------------------------------------------------- toasts ----- */
  const TOAST_ICON = {
    success: "i-check-circle",
    info: "i-info",
    warning: "i-alert",
    danger: "i-alert",
  };

  function toast(message, kind) {
    kind = kind || "info";
    const el = document.createElement("div");
    el.className = "toast toast--" + kind;
    el.innerHTML = `<svg><use href="#${TOAST_ICON[kind] || "i-info"}"/></svg><span>${message}</span>`;
    $("#toasts").appendChild(el);
    setTimeout(() => {
      el.style.transition = "opacity .25s, transform .25s";
      el.style.opacity = "0";
      el.style.transform = "translateX(16px)";
      setTimeout(() => el.remove(), 260);
    }, 2600);
  }

  /* ---------------------------------------------------------- wizard ----- */
  function wizardShow(step) {
    state.wizardStep = Math.max(1, Math.min(5, step));
    $$(".wpanel").forEach((p) =>
      p.classList.toggle("is-active", Number(p.dataset.wpanel) === state.wizardStep)
    );
    $$(".wstep").forEach((s) => {
      const n = Number(s.dataset.wstep);
      s.classList.toggle("is-on", n === state.wizardStep);
      s.classList.toggle("is-done", n < state.wizardStep);
      if (n < state.wizardStep) s.querySelector(".wstep__num").innerHTML = '<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><use href="#i-check"/></svg>';
      else s.querySelector(".wstep__num").textContent = n;
    });
    $("#wizardBack").disabled = state.wizardStep === 1;
    $("#wizardNext").innerHTML =
      state.wizardStep === 5
        ? 'Finish<svg><use href="#i-check"/></svg>'
        : 'Continue<svg><use href="#i-chevron-r"/></svg>';
    paintAllRanges();
    renderArgsPreview();
    if (state.wizardStep === 5) {
      $("#sumMemory").textContent = state.ram + " MB";
      $("#sumCollector").textContent = (D.GC.find((g) => g.id === state.gc) || D.GC[0]).label;
      const themeLabel = (THEMES.find((t) => t.id === state.theme) || THEMES[0]).label;
      const accentLabel = (ACCENTS.find((a) => a.id === state.accent) || ACCENTS[0]).label;
      $("#sumTheme").textContent = `${themeLabel} · ${accentLabel}`;
    }
  }

  function openWizard() {
    $("#wizardOverlay").classList.add("is-open");
    wizardShow(1);
  }

  /* ------------------------------------------------------------- init ---- */
  function init() {
    applyTheme();
    renderInstances();
    renderBrowse();
    renderMods();
    initTabs();
    buildAccentPicker($("#accentPicker"));
    buildAccentPicker($("#wizardAccents"));
    buildThemePicker($("#wizardThemes"));
    renderGc(["#gcCards", "#wizardGc"]);
    selectInstance("star");
    paintAllRanges();

    // rail navigation + anything else that declares a destination
    $$("[data-goto]").forEach((el) =>
      el.addEventListener("click", () => goto(el.dataset.goto))
    );

    // theme controls
    $("#themeToggle").addEventListener("click", cycleTheme);
    $$(".theme-option[data-theme-value]").forEach((el) =>
      el.addEventListener("click", () => {
        state.theme = el.dataset.themeValue;
        applyTheme();
      })
    );

    // RAM controls (every slider/number/chip drives the same state)
    ["#ram", "#quickRam", "#wizardRam"].forEach((sel) => {
      const el = $(sel);
      if (el)
        el.addEventListener("input", (e) => {
          paintRange(e.target);
          setRam(e.target.value);
        });
    });
    // anything else with a range (custom sliders) still gets a filled track
    $$('input[type="range"]').forEach((el) =>
      el.addEventListener("input", () => paintRange(el))
    );
    ["#ramNumber", "#quickRamNumber"].forEach((sel) => {
      const el = $(sel);
      if (el) el.addEventListener("change", (e) => setRam(e.target.value));
    });
    $$(".chip[data-ram]").forEach((chip) =>
      chip.addEventListener("click", () => setRam(chip.dataset.ram))
    );
    $("#ramAuto").addEventListener("click", autoRam);
    $("#ramAuto2").addEventListener("click", autoRam);
    $("#wizardAuto").addEventListener("click", () => {
      autoRam();
      toast("Auto-tuned from system memory and mod count", "success");
    });

    $("#quickGc").addEventListener("change", (e) => {
      state.gc = e.target.value;
      const inst = D.INSTANCES.find((i) => i.id === state.activeInstance);
      if (inst) inst.gc = state.gc;
      syncGcUI();
    });

    // reset the collector choice back to the tuned default
    const gcReset = $("#gcReset");
    if (gcReset)
      gcReset.addEventListener("click", () => {
        state.gc = "g1gc";
        const inst = D.INSTANCES.find((i) => i.id === state.activeInstance);
        if (inst) inst.gc = "g1gc";
        syncGcUI();
        toast("Garbage collector reset to G1GC (default)", "success");
      });

    const copyArgs2 = $("#copyArgs2");
    if (copyArgs2)
      copyArgs2.addEventListener("click", () => {
        const gc = D.GC.find((g) => g.id === state.gc) || D.GC[0];
        const extra = $("#jvmArgs") ? $("#jvmArgs").value.trim() : "";
        navigator.clipboard &&
          navigator.clipboard.writeText(`-Xmx${state.ram}M ${gc.args} ${extra}`.trim());
      });

    // launch buttons
    $("#heroLaunch").addEventListener("click", () => launch());
    $("#instLaunch").addEventListener("click", () => launch());

    // Star Menu master switch dims the preset rows
    $("#starMenuMaster").addEventListener("change", (e) => {
      state.starMenu = e.target.checked;
      $$(".mod-row").forEach((row) => row.classList.toggle("is-off", !state.starMenu));
      $$(".smMod").forEach((sw) => (sw.disabled = !state.starMenu));
      toast(`Star Menu ${state.starMenu ? "enabled" : "disabled"} for this instance`, state.starMenu ? "success" : "warning");
    });

    // wizard
    $("#quickStartChip").addEventListener("click", openWizard);
    $("#rerunWizard").addEventListener("click", openWizard);
    $("#wizardNext").addEventListener("click", () => {
      if (state.wizardStep === 5) {
        $("#wizardOverlay").classList.remove("is-open");
        toast("Setup complete. Have fun!", "success");
        return;
      }
      wizardShow(state.wizardStep + 1);
    });
    $("#wizardBack").addEventListener("click", () => wizardShow(state.wizardStep - 1));
    $("#wizardSkip").addEventListener("click", () => {
      $("#wizardOverlay").classList.remove("is-open");
      toast("You can re-run setup from Settings → Launcher", "info");
    });
    $("#wizardName").addEventListener("input", (e) => {
      $("#sumAccount").textContent = e.target.value ? `Offline · ${e.target.value}` : "Online (Microsoft)";
    });

    // modals
    $$("[data-close-modal]").forEach((el) =>
      el.addEventListener("click", () => $("#modalOverlay").classList.remove("is-open"))
    );
    $$(".overlay").forEach((ov) =>
      ov.addEventListener("click", (e) => {
        if (e.target === ov && ov.id !== "wizardOverlay") ov.classList.remove("is-open");
      })
    );
    document.addEventListener("keydown", (e) => {
      if (e.key === "Escape") {
        $("#modalOverlay").classList.remove("is-open");
        $("#wizardOverlay").classList.remove("is-open");
      }
    });

    // generic data-toast handlers
    document.addEventListener("click", (e) => {
      const t = e.target.closest("[data-toast]");
      if (t) toast(t.dataset.toast, "success");
    });

    // copy resolved JVM args
    const copyBtn = $("#copyArgs");
    if (copyBtn) copyBtn.addEventListener("click", () => {
      const gc = D.GC.find((g) => g.id === state.gc) || D.GC[0];
      const extra = $("#jvmArgs").value.trim();
      navigator.clipboard && navigator.clipboard.writeText(`${gc.args} ${extra}`.trim());
    });

    // compact density toggle -> data-density drives the spacing rules in CSS
    $("#compactToggle").addEventListener("change", (e) => {
      if (e.target.checked) document.documentElement.setAttribute("data-density", "compact");
      else document.documentElement.removeAttribute("data-density");
    });

    // react to OS theme changes while in "match system"
    window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => {
      if (state.theme === "system") applyTheme();
    });

    // first-run wizard. sessionStorage throws on some file:// setups, so the
    // whole check is optional - the wizard is always reachable from the
    // Quick Start chip and from Settings.
    let seen = false;
    try {
      seen = sessionStorage.getItem("starclient.visited") === "1";
      sessionStorage.setItem("starclient.visited", "1");
    } catch (err) {
      seen = true; // storage unavailable: don't nag on every load
    }
    if (!seen) setTimeout(openWizard, 450);
  }

  document.addEventListener("DOMContentLoaded", init);
})();
