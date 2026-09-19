/* ==========================================================================
 * Star Client - prototype data
 * --------------------------------------------------------------------------
 * Mock content for the UI prototype. The garbage-collector table and the
 * memory limits mirror theme/tokens.json (performance.garbage-collectors and
 * performance.ram) so the prototype and the launcher agree on the numbers.
 * ========================================================================== */
(function (global) {
  "use strict";

  const PHYSICAL_MB = 16384; // pretend machine, so the RAM warnings can be demoed

  const GC = [
    {
      id: "g1gc",
      label: "G1GC",
      tag: "Default",
      desc: "Balanced default. Best compatibility and steady frame pacing on 4-8 GB heaps.",
      minJava: 8,
      args: "-XX:+UseG1GC -XX:MaxGCPauseMillis=200 -XX:G1HeapRegionSize=8M -XX:G1ReservePercent=20 -XX:InitiatingHeapOccupancyPercent=15 -XX:MaxTenuringThreshold=1",
    },
    {
      id: "zgc",
      label: "ZGC",
      tag: "",
      desc: "Ultra-low pause times. Needs Java 17+ and 8 GB+ of RAM to beat G1GC.",
      minJava: 17,
      args: "-XX:+UseZGC -XX:ZCollectionInterval=120 -XX:ZAllocationSpikeTolerance=5 -XX:+DisableExplicitGC",
    },
    {
      id: "zgc-gen",
      label: "Generational ZGC",
      tag: "New",
      desc: "Java 21+ only. Low pauses with better throughput than classic ZGC.",
      minJava: 21,
      args: "-XX:+UseZGC -XX:+ZGenerational -XX:+DisableExplicitGC",
    },
    {
      id: "shenandoah",
      label: "Shenandoah",
      tag: "",
      desc: "Low-pause alternative to ZGC. Handy when ZGC stutters on small heaps.",
      minJava: 12,
      args: "-XX:+UseShenandoahGC -XX:ShenandoahGCMode=iu -XX:+DisableExplicitGC",
    },
    {
      id: "serial",
      label: "Serial GC",
      tag: "Low spec",
      desc: "Tiny footprint for low-spec machines and very small heaps.",
      minJava: 8,
      args: "-XX:+UseSerialGC -XX:+DisableExplicitGC",
    },
    {
      id: "parallel",
      label: "Parallel GC",
      tag: "",
      desc: "Highest raw throughput, longer pauses. Good for servers, not for PvP.",
      minJava: 8,
      args: "-XX:+UseParallelGC -XX:ParallelGCThreads=4 -XX:+DisableExplicitGC",
    },
  ];

  const INSTANCES = [
    {
      id: "star",
      name: "Star Client",
      sub: "Minecraft 1.21.1 · Fabric",
      art: "art-tech",
      loader: "Fabric",
      version: "1.21.1",
      mods: 87,
      ram: 6144,
      gc: "g1gc",
      played: "2 hours ago",
      lastBoot: 2 * 60 * 60 * 1000,
    },
    {
      id: "skyblock",
      name: "Skyblock Islands",
      sub: "Minecraft 1.21.1 · Fabric",
      art: "art-sky",
      loader: "Fabric",
      version: "1.21.1",
      mods: 42,
      ram: 4096,
      gc: "g1gc",
      played: "yesterday",
      lastBoot: 26 * 60 * 60 * 1000,
    },
    {
      id: "vanilla",
      name: "Vanilla 1.21.1",
      sub: "Minecraft 1.21.1 · Vanilla",
      art: "art-grass",
      loader: "Vanilla",
      version: "1.21.1",
      mods: 0,
      ram: 2048,
      gc: "serial",
      played: "4 days ago",
      lastBoot: 4 * 24 * 60 * 60 * 1000,
    },
    {
      id: "netherite",
      name: "Netherite Forge",
      sub: "Minecraft 1.20.1 · Forge",
      art: "art-nether",
      loader: "Forge",
      version: "1.20.1",
      mods: 213,
      ram: 10240,
      gc: "zgc",
      played: "1 week ago",
      lastBoot: 7 * 24 * 60 * 60 * 1000,
    },
  ];

  const MODS = [
    { name: "Sodium-class renderer", version: "0.6.0", source: "Modrinth", star: true },
    { name: "Lithium-class optimiser", version: "0.14.2", source: "Modrinth", star: true },
    { name: "Zoom & waypoints", version: "5.4.1", source: "Modrinth", star: true },
    { name: "Performance HUD", version: "3.12.4", source: "CurseForge", star: true },
    { name: "Player list extras", version: "1.4.0", source: "Modrinth", star: true },
    { name: "Screenshot tools", version: "2.1.7", source: "Modrinth", star: true },
    { name: "Connection watchdog", version: "0.9.3", source: "CurseForge", star: true },
    { name: "Recipe book plus", version: "1.2.0", source: "Modrinth", star: false },
    { name: "Shulker preview", version: "2.0.4", source: "Modrinth", star: false },
    { name: "Better statistics", version: "1.1.2", source: "CurseForge", star: false },
  ];

  const BROWSE = [
    { name: "Sodium-class renderer", desc: "Rendering pipeline rebuilt for high frame rates.", icon: "i-spark", downloads: "48.2M", tag: "Optimisation" },
    { name: "Lithium-class optimiser", desc: "Server and client logic optimisations without behaviour changes.", icon: "i-cpu", downloads: "36.9M", tag: "Optimisation" },
    { name: "Zoom & waypoints", desc: "Scroll-to-zoom with persistent minimap markers.", icon: "i-compass", downloads: "22.4M", tag: "Utility" },
    { name: "Performance HUD", desc: "Frame time, memory and chunk graph overlay.", icon: "i-memory", downloads: "19.1M", tag: "Utility" },
    { name: "Shader loader", desc: "Post-processing shader support for modern renderers.", icon: "i-layers", downloads: "31.7M", tag: "Graphics" },
    { name: "Screenshot tools", desc: "Instant gallery, clipboard copy, auto naming.", icon: "i-wrench", downloads: "8.3M", tag: "Utility" },
    { name: "Backup keeper", desc: "Rolling world backups before every launch.", icon: "i-shield", downloads: "5.6M", tag: "Safety" },
    { name: "Map bookmarks", desc: "Share waypoints with your server friends.", icon: "i-globe", downloads: "4.2M", tag: "Utility" },
  ];

  function fmtMB(mb) {
    if (mb >= 1024) {
      const gb = mb / 1024;
      return (gb >= 10 ? gb.toFixed(1) : gb.toFixed(gb % 1 === 0 ? 0 : 1)) + " GB";
    }
    return mb + " MB";
  }

  /** Mirrors the safe-band logic the launcher will use in RAM control. */
  function ramVerdict(mb, modCount) {
    const safeMax = PHYSICAL_MB * 0.75;
    if (mb > safeMax) {
      return { state: "danger", title: "Too much memory", body: `That leaves the operating system less than a quarter of your ${fmtMB(PHYSICAL_MB)}. Star Client caps the slider at ${fmtMB(safeMax)} for safety.` };
    }
    if (mb < 1024) {
      return { state: "danger", title: "Almost certainly too little", body: "Below 1 GB the game will stall on chunk generation and most mod loaders will run out of memory during startup." };
    }
    if (mb < 2048) {
      return { state: "warning", title: "Tight fit", body: `Fine for vanilla, risky with ${modCount || "many"} mods. 4 GB is a safer floor for modded play.` };
    }
    if (mb > 12288 && modCount < 150) {
      return { state: "warning", title: "More than you need", body: "Heaps beyond 12 GB usually measure slower than a well-tuned 8 GB heap, because full collections take longer." };
    }
    if (mb >= 4096 && mb <= 8192) {
      return { state: "success", title: "Comfortable", body: `${mb} MB is inside the recommended band for a ${fmtMB(PHYSICAL_MB)} machine with ${modCount || 0} mods in this instance.` };
    }
    return { state: "info", title: "Workable", body: `${mb} MB will run, but the sweet spot for modded 1.21 is 4-8 GB.` };
  }

  global.StarData = { PHYSICAL_MB, GC, INSTANCES, MODS, BROWSE, fmtMB, ramVerdict };
})(window);
