"use strict";
(() => {
  const el = id => document.getElementById(id);
  const canvas = el("canvas"), status = el("status"), storage = el("storage");
  const log = message => {
    el("log").textContent = (el("log").textContent + String(message) + "\n").slice(-16000);
    console.log(message);
  };
  let game, started = false, dirty = 0, saved = 0, syncing = false, timer;
  function fail(error) {
    log(error?.stack || error);
    status.textContent = "The game could not start. " + (error?.message || error);
    el("launch").hidden = false;
    el("start").disabled = el("watch").disabled = true;
  }
  function syncSaves() {
    clearTimeout(timer);
    if (!game || syncing || dirty === saved) return;
    syncing = true;
    const revision = dirty;
    storage.textContent = "Saving to this browser…";
    game.FS.syncfs(false, error => {
      syncing = false;
      if (error) {
        storage.textContent = "Browser save failed — keep this tab open and retry.";
        el("retry-save").hidden = false;
        log("IndexedDB save failed: " + error);
        return;
      }
      saved = revision;
      el("retry-save").hidden = true;
      storage.textContent = "Saved to this browser";
      if (dirty !== saved) syncSaves();
    });
  }
  function queueSave() {
    ++dirty;
    storage.textContent = "Save pending…";
    clearTimeout(timer);
    timer = setTimeout(syncSaves, 100);
  }
  el("retry-save").onclick = syncSaves;
  window.addEventListener("beforeunload", event => {
    if (dirty !== saved) { event.preventDefault(); event.returnValue = ""; }
  });
  document.addEventListener("visibilitychange", () => {
    if (document.hidden) syncSaves();
  });
  canvas.addEventListener("contextmenu", event => event.preventDefault());
  canvas.addEventListener("keydown", event => {
    if (["ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight", "Space", "Tab"].includes(event.code)) event.preventDefault();
  });
  el("fullscreen").onclick = async () => {
    try {
      if (document.fullscreenElement) await document.exitFullscreen();
      else await el("stage").requestFullscreen();
      canvas.focus();
    } catch (error) { log("Fullscreen unavailable: " + error.message); }
  };
  function start(watch) {
    if (!game || started) return;
    started = true;
    document.body.classList.add("playing");
    el("launch").hidden = true;
    el("fullscreen").disabled = false;
    canvas.focus();
    // main() is invoked directly inside this click, enabling browser audio.
    try { game.callMain(["--data", "/data", "--audio-data", "/audio", watch ? "--cpu-teams" : "--front-end"]); }
    catch (error) { fail(error); }
  }
  el("start").onclick = () => start(false);
  el("watch").onclick = () => start(true);
  async function prepare() {
    if (typeof createEuro96 !== "function") throw new Error("The browser build is missing. Run tools/build_web.ps1 first.");
    game = await createEuro96({canvas, noInitialRun: true, print: log, printErr: log,
      queueSave, onAbort: reason => fail(new Error(String(reason))),
      onExit: code => {
        syncSaves();
        el("launch").hidden = false;
        status.textContent = code ? "The game stopped with an error. See the technical log." : "Game closed. Reload this page to play again.";
        el("start").disabled = el("watch").disabled = true;
      }});
    // Exposed only by this proof-of-concept shell for browser QA and diagnostics.
    window.euro96 = game;
    game.FS.mkdirTree("/saves");
    game.FS.mount(game.FS.filesystems.IDBFS, {}, "/saves");
    await new Promise((resolve, reject) => game.FS.syncfs(true, error => error ? reject(new Error("Browser storage is unavailable: " + error)) : resolve()));
    storage.textContent = "Browser saves ready";
    const response = await fetch("assets.json", {cache: "no-cache"});
    if (!response.ok) throw new Error("Asset manifest is missing (HTTP " + response.status + ").");
    const manifest = await response.json();
    const total = manifest.files.reduce((sum, file) => sum + file.bytes, 0);
    let completed = 0, index = 0;
    // Limit concurrent temporary buffers; originals are mounted in the Wasm FS.
    await Promise.all(Array.from({length: 3}, async () => {
      while (index < manifest.files.length) {
        const file = manifest.files[index++];
        const response = await fetch(file.url);
        if (!response.ok) throw new Error(file.path + " failed to load (HTTP " + response.status + ").");
        const bytes = new Uint8Array(await response.arrayBuffer());
        if (bytes.length !== file.bytes) throw new Error("Incomplete asset: " + file.path);
        const hash = Array.from(new Uint8Array(await crypto.subtle.digest("SHA-256", bytes)), b => b.toString(16).padStart(2, "0")).join("");
        if (hash !== file.sha256) throw new Error("Asset checksum mismatch: " + file.path);
        game.FS.mkdirTree(file.path.slice(0, file.path.lastIndexOf("/")));
        game.FS.writeFile(file.path, bytes, {canOwn: true});
        completed += bytes.length;
        el("progress").value = completed / total;
        status.textContent = `Loading game assets · ${Math.round(completed / 1048576)} / ${Math.round(total / 1048576)} MiB`;
      }
    }));
    status.textContent = "Ready. Choose Play to enable sound and open the original menus.";
    el("start").disabled = el("watch").disabled = false;
    log("Browser assets verified: " + manifest.files.length + " files, " + total + " bytes.");
  }
  prepare().catch(fail);
})();
