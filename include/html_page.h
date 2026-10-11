#ifndef HTML_PAGE_H
#define HTML_PAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <meta name="color-scheme" content="light dark">
  <meta name="theme-color" content="#e9eff2">
  <title>CCJ 中枢 · 灯光控制</title>
  <style>
    :root {
      color-scheme: light;
      --ink: #24374a; --muted: #526878; --blue: #2268b2; --line: rgba(255,255,255,.75);
      --glass: rgba(255,255,255,.38); --inset: rgba(255,255,255,.33);
      --shadow: 0 20px 60px -22px rgba(44,78,101,.28), 0 3px 8px rgba(44,78,101,.035);
      --green: #2c8068; --danger: #b8363f; --amber: #9a570b;
    }
    * { box-sizing: border-box; }
    [hidden] { display: none !important; }
    body {
      margin: 0; min-height: 100vh; min-height: 100svh; color: var(--ink);
      padding: max(24px, env(safe-area-inset-top)) 20px max(20px, env(safe-area-inset-bottom));
      font: 15px/1.5 -apple-system, BlinkMacSystemFont, "SF Pro Display", "Segoe UI", "PingFang SC", "Microsoft YaHei", sans-serif;
      background: #e9eff2; -webkit-font-smoothing: antialiased;
    }
    button, input { font: inherit; }
    button { color: inherit; cursor: pointer; -webkit-tap-highlight-color: transparent; }
    button:disabled { cursor: default; }
    button:focus-visible, input:focus-visible { outline: 3px solid var(--blue); outline-offset: 5px; }
    button { border: 0; }
    .icon { width: 22px; height: 22px; fill: none; stroke: currentColor; stroke-width: 1.65; stroke-linecap: round; stroke-linejoin: round; flex-shrink: 0; }
    .symbols { position: absolute; width: 0; height: 0; overflow: hidden; }
    .wallpaper { position: fixed; inset: 0; z-index: -1; overflow: hidden; pointer-events: none;
      background: radial-gradient(ellipse at 20% 0%, #fbf6ef 0, transparent 55%), linear-gradient(145deg, #e9f2f6, #e6ecee 48%, #e8edeb); }
    .orb { position: absolute; border-radius: 50%; }
    .orb-blue { width: 570px; height: 570px; top: -140px; left: calc(50% - 10px);
      background: radial-gradient(circle at 30% 30%, #e1faff 0, #bddbe9 28%, #88b5d1 58%, #c8e1e9 78%);
      box-shadow: inset 2px 3px 5px rgba(255,255,255,.85), inset -22px -24px 70px rgba(69,119,164,.12), 0 30px 90px rgba(89,143,174,.13); }
    .orb-peach { width: 370px; height: 370px; left: calc(50% - 420px); top: 280px;
      background: radial-gradient(circle at 30% 26%, #fff5de, #f8d4b9 40%, #e7bbaa 76%);
      box-shadow: inset 2px 3px 6px #fff8, inset -18px -20px 40px #cd947422; }
    .orb-mint { width: 620px; height: 620px; left: calc(50% - 150px); top: 590px;
      background: radial-gradient(circle at 37% 28%, #e0f4ed, #bedad1 45%, #9abcba 78%); box-shadow: inset 3px 4px 8px #fff9; }
    .wallpaper::after { content: ""; position: absolute; inset: 0; opacity: .26;
      background-image: url("data:image/svg+xml,%3Csvg viewBox='0 0 180 180' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='.8' numOctaves='3' stitchTiles='stitch'/%3E%3C/filter%3E%3Cpath fill='%238899aa' opacity='.15' filter='url(%23n)' d='M0 0h180v180H0z'/%3E%3C/svg%3E"); }
    .app { width: 100%; max-width: 460px; margin: 0 auto; }
    .brand { display: flex; align-items: center; justify-content: space-between; padding: 0 9px; margin: 0 0 18px; }
    .wordmark { font-size: 19px; font-weight: 750; letter-spacing: -.7px; }
    .wordmark span { font-weight: 450; margin-left: 9px; font-size: 14px; letter-spacing: 1.5px; }
    .brand-detail { display: flex; align-items: center; gap: 5px; font-size: 12px; color: var(--muted); }
    .brand-detail .icon { width: 16px; height: 16px; }
    .glass { position: relative; isolation: isolate; border: 1px solid var(--line); background: var(--glass); border-radius: 32px;
      box-shadow: var(--shadow), inset 0 1px 0 rgba(255,255,255,.72), inset 0 -1px 0 rgba(255,255,255,.3);
      backdrop-filter: blur(28px) saturate(150%); -webkit-backdrop-filter: blur(28px) saturate(150%); }
    .glass::before { content: ""; position: absolute; inset: 0; z-index: -1; border-radius: inherit; pointer-events: none;
      background: linear-gradient(135deg, rgba(255,255,255,.32), transparent 36%, transparent 70%, rgba(255,255,255,.18)); }
    .light-card { padding: 21px 26px 20px; text-align: center; }
    .device-bar { display: flex; align-items: center; justify-content: space-between; gap: 10px; }
    .connection { display: flex; align-items: center; gap: 7px; min-width: 0; font-size: 12px; color: var(--muted); text-align: left; }
    .dot { width: 6px; height: 6px; background: #7c8e9a; border-radius: 50%; flex-shrink: 0; }
    [data-connection="connected"] .dot { background: var(--green); box-shadow: 0 0 0 4px rgba(44,128,104,.08); }
    [data-connection="maintenance"] .dot { background: #c38631; box-shadow: 0 0 0 4px rgba(195,134,49,.09); }
    [data-connection="offline"] .dot { background: var(--danger); }
    .battery-badge { display: flex; align-items: center; gap: 6px; padding: 6px 10px; flex-shrink: 0;
      border-radius: 30px; background: rgba(255,255,255,.35); color: var(--green); font-size: 12px; font-weight: 600;
      border: 1px solid rgba(255,255,255,.55); }
    .battery-badge .icon { width: 20px; height: 16px; }
    .battery-fill { fill: currentColor; stroke: none; transition: width .35s ease; }
    .battery-low { color: var(--danger); } .battery-mid { color: var(--amber); }
    .battery-unknown { color: var(--muted); }
    .light-art { display: grid; place-items: center; position: relative; width: 80px; height: 80px; margin: 19px auto 12px;
      border-radius: 28px; color: #527e9b; background: linear-gradient(135deg, rgba(255,255,255,.7), rgba(236,247,250,.12));
      border: 1px solid rgba(255,255,255,.84); box-shadow: 0 12px 30px -15px #577c9866, inset 0 2px 1px #fff9;
      transition: color .3s, box-shadow .3s, background .3s; }
    .light-art::before { content: ""; position: absolute; inset: -22px; z-index: -1; border-radius: 50%;
      background: radial-gradient(circle, #ffffff70, transparent 70%); }
    .light-art .icon { width: 40px; height: 40px; stroke-width: 1.15; }
    .light-card[data-choice="1"] .light-art { color: #ac701f; background: linear-gradient(135deg, #fff8dbb3, #ffe9b330);
      box-shadow: 0 10px 34px -8px #eeb85944, inset 0 2px 1px #fffc; }
    h1 { margin: 0; font-size: 28px; font-weight: 650; letter-spacing: -.8px; line-height: 1.3; }
    .device-caption { margin: 7px 0 20px; font-size: 12px; color: var(--muted); letter-spacing: .2px; }
    .light-slider { display: flex; position: relative; padding: 5px; border-radius: 25px; height: 72px; isolation: isolate;
      background: rgba(91,119,139,.085); border: 1px solid rgba(83,112,134,.075);
      box-shadow: inset 0 2px 5px rgba(66,97,120,.07), 0 1px 0 #ffffff8c; touch-action: pan-y; user-select: none; }
    .slider-thumb { position: absolute; left: 5px; top: 5px; bottom: 5px; width: calc(50% - 5px); border-radius: 20px;
      z-index: -1; opacity: 0; transform: translateX(0); pointer-events: none;
      background: linear-gradient(140deg, rgba(255,255,255,.88), rgba(255,255,255,.42));
      border: 1px solid rgba(255,255,255,.94); box-shadow: 0 5px 12px rgba(57,81,96,.12), inset 0 1px 1px #fff;
      backdrop-filter: blur(12px); -webkit-backdrop-filter: blur(12px); transition: transform .32s cubic-bezier(.2,.9,.3,1), opacity .2s; }
    .light-slider[data-choice="0"] .slider-thumb, .light-slider[data-choice="1"] .slider-thumb { opacity: 1; }
    .light-slider[data-choice="1"] .slider-thumb { transform: translateX(100%); background: linear-gradient(140deg, #fffbedec, #ffefd878); }
    .light-slider.dragging .slider-thumb { opacity: 1; transition: none; }
    .control-btn { display: flex; align-items: center; justify-content: center; gap: 10px; flex: 1; border-radius: 20px;
      background: transparent; font-size: 16px; font-weight: 580; color: var(--muted); }
    .control-btn[aria-pressed="true"] { color: var(--ink); }
    .on-btn[aria-pressed="true"] { color: #93591e; }
    .control-btn:disabled { opacity: .45; }
    .light-slider[aria-busy="true"] { opacity: .68; }
    .control-status { min-height: 21px; margin: 15px 0 3px; font-size: 13px; color: var(--muted); }
    .control-status.success { color: var(--green); } .control-status.error { color: var(--danger); }
    .state-note { margin: 0; color: var(--muted); font-size: 11px; opacity: .85; }
    .maintenance-card { margin-top: 14px; padding: 21px 24px 17px; border-radius: 27px; }
    .section-heading { display: flex; align-items: center; gap: 10px; margin-bottom: 6px; }
    .section-heading .icon { color: var(--blue); width: 18px; height: 18px; }
    h2 { margin: 0; font-size: 16px; font-weight: 650; letter-spacing: -.2px; }
    .section-copy { font-size: 12px; color: var(--muted); margin: 0 0 16px; line-height: 1.7; }
    .release-btn { display: flex; align-items: center; justify-content: space-between; position: relative; width: 100%;
      min-height: 72px; padding: 14px 17px; border-radius: 20px; overflow: hidden; isolation: isolate; text-align: left;
      color: var(--blue); background: rgba(218,234,247,.43); border: 1px solid rgba(255,255,255,.68);
      box-shadow: inset 0 1px 0 #fffa, 0 3px 12px #466f9810; transition: box-shadow .2s, background .2s; }
    .release-btn:not(:disabled):hover { box-shadow: inset 0 1px 0 #fff, 0 5px 20px #466f9820; }
    .release-btn:disabled { opacity: .58; }
    .release-progress { position: absolute; inset: 0; z-index: -1; transform: scaleX(0); transform-origin: left center;
      background: linear-gradient(100deg, rgba(144,190,222,.27), rgba(116,174,214,.4));
      border-right: 1px solid #ffffffb3; transition: transform .26s linear; pointer-events: none; }
    .release-btn[data-phase="active"] { color: #235874; background: rgba(216,234,244,.3); }
    .release-title { display: block; font-size: 14px; font-weight: 650; }
    .release-hint { display: block; font-size: 11px; margin-top: 3px; color: var(--muted); }
    .release-time { flex-shrink: 0; font-size: 23px; font-weight: 400; font-variant-numeric: tabular-nums; letter-spacing: -.6px; margin-left: 12px; }
    .maintenance-status { color: var(--muted); font-size: 11px; margin: 11px 2px 0; min-height: 17px; }
    .resume-btn { display: flex; align-items: center; justify-content: center; gap: 6px; min-height: 39px;
      margin: 8px auto -5px; padding: 4px 12px; color: var(--blue); font-size: 12px; border-radius: 12px; background: transparent; }
    .resume-btn .icon { width: 14px; height: 14px; }
    .resume-btn:hover { background: var(--inset); } .resume-btn:disabled { opacity: .45; }
    .settings-link { display: flex; align-items: center; gap: 12px; width: 100%; margin-top: 17px; padding: 13px 17px;
      border-radius: 21px; text-align: left; background: rgba(255,255,255,.22); border: 1px solid rgba(255,255,255,.45);
      backdrop-filter: blur(16px); -webkit-backdrop-filter: blur(16px); }
    .settings-link > .icon:first-child { width: 21px; height: 21px; color: var(--muted); }
    .settings-label { flex: 1; font-size: 13px; font-weight: 600; }
    .settings-label small { display: block; margin-top: 1px; font-size: 11px; color: var(--muted); font-weight: 400; }
    .settings-link > .icon:last-child { width: 16px; height: 16px; color: var(--muted); }
    .settings-link:disabled { opacity: .5; }
    .page-footer { margin: 12px 0 0; text-align: center; font-size: 10px; color: var(--muted); letter-spacing: 1.4px; opacity: .65; }
    dialog { color: var(--ink); width: calc(100% - 32px); max-width: 420px; max-height: 85svh; margin: auto;
      padding: 25px; overflow-y: auto; }
    dialog.glass { background: rgba(240,246,249,.86); border-radius: 29px; }
    dialog::backdrop { background: rgba(32,54,74,.2); backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px); }
    .modal-header { display: flex; align-items: center; justify-content: space-between; gap: 12px; margin-bottom: 8px; }
    .modal-header h2 { font-size: 20px; }
    .close-btn { display: grid; place-items: center; width: 36px; height: 36px; border-radius: 50%; background: var(--inset); }
    .close-btn .icon { width: 18px; height: 18px; }
    .btn-primary { width: 100%; padding: 13px 16px; margin-top: 12px; background: #286da8; color: white; border-radius: 16px; font-weight: 600; font-size: 14px; }
    .btn-primary:disabled { opacity: .5; }
    .device-list { margin-top: 14px; }
    .device-item { display: flex; justify-content: space-between; align-items: center; gap: 14px; padding: 14px;
      margin-top: 8px; border-radius: 17px; background: var(--inset); border: 1px solid var(--line); cursor: pointer; }
    .device-info { min-width: 0; display: flex; flex-direction: column; gap: 3px; }
    .device-name { font-size: 14px; font-weight: 600; overflow-wrap: anywhere; }
    .device-mac { font-size: 11px; color: var(--muted); font-variant-numeric: tabular-nums; }
    .device-rssi { font-size: 11px; }
    .device-item input { accent-color: var(--blue); width: 20px; height: 20px; flex-shrink: 0; }
    .modal-status { color: var(--muted); font-size: 12px; margin: 12px 0 0; min-height: 18px; }
    .loader { width: 25px; height: 25px; margin: 20px auto; border: 2px solid rgba(70,110,145,.15); border-top-color: var(--blue); border-radius: 50%; animation: spin 1s linear infinite; }
    @keyframes spin { to { transform: rotate(360deg); } }
    @media (min-width: 700px) { body { padding-top: 40px; } .app { margin-top: max(0px, calc((100svh - 890px) / 2)); } }
    @media (max-width: 360px) { body { padding: 20px 14px; } .light-card { padding: 19px 18px; } .maintenance-card { padding: 20px 18px 16px; } .connection { font-size: 11px; } .release-time { font-size: 21px; } }
    @media (prefers-color-scheme: dark) {
      :root { color-scheme: dark; --ink: #e8f0f8; --muted: #b4c5d2; --blue: #9bd0ff; --line: rgba(224,239,255,.24);
        --glass: rgba(31,49,65,.51); --inset: rgba(174,204,234,.11); --green: #8dd6b9; --danger: #ff9fa4; --amber: #edc38a;
        --shadow: 0 22px 60px -15px rgba(0,8,20,.4); }
      body { background: #142535; }
      .wallpaper { background: linear-gradient(135deg, #223443, #152534 60%, #193735); }
      .orb-blue { background: radial-gradient(circle at 32% 24%, #6296b3, #305876 50%, #1f3f59 78%); box-shadow: inset 2px 3px 5px #cbebff40; }
      .orb-peach { background: radial-gradient(circle at 30% 25%, #a38c74, #695e55 50%, #493f3d); box-shadow: inset 2px 3px 5px #ffe1b630; }
      .orb-mint { background: radial-gradient(circle at 37% 28%, #558d85, #315f5b 45%, #203f40 78%); box-shadow: inset 2px 3px 5px #cbfff230; }
      .glass { box-shadow: var(--shadow), inset 0 1px 0 #edf7ff30; }
      .glass::before { opacity: .22; }
      .battery-badge { background: #d1ecff10; border-color: #d1ecff20; }
      .light-art { color: #cee6f8; background: linear-gradient(135deg, #dfedff24, #d1ecff07); border-color: #d1ecff55; box-shadow: inset 0 1px 1px #fff4, 0 10px 25px #0002; }
      .light-art::before { opacity: .12; }
      .light-card[data-choice="1"] .light-art { color: #ffe0a3; background: linear-gradient(135deg, #f6d79d40, #ffdd8410); box-shadow: inset 0 1px 1px #ffefcf66, 0 10px 28px #eeb85916; }
      .light-slider { background: #09182733; box-shadow: inset 0 2px 5px #0002, 0 1px 0 #fff2; }
      .slider-thumb { background: linear-gradient(140deg, #cde9ff3d, #9db0c629); border-color: #eff7ff5c; box-shadow: 0 5px 12px #07162944, inset 0 1px 1px #fff4; }
      .light-slider[data-choice="1"] .slider-thumb { background: linear-gradient(140deg, #ffe8b044, #e4b36520); }
      .on-btn[aria-pressed="true"] { color: #ffdfaa; }
      .release-btn { background: #aed9ff14; border-color: #d1ecff38; box-shadow: inset 0 1px 0 #fff2; }
      .release-btn[data-phase="active"] { color: #d0eaff; background: #aed9ff14; }
      .release-progress { background: linear-gradient(100deg, #77b5e025, #7dc7fc55); border-right-color: #a9ddff80; }
      .settings-link { background: #d1ecff09; border-color: #d1ecff20; }
      dialog.glass { background: rgba(29,49,67,.93); }
    }
    @supports not ((backdrop-filter: blur(1px)) or (-webkit-backdrop-filter: blur(1px))) {
      .glass { background: #eaf0f4; }
      @media (prefers-color-scheme: dark) { .glass { background: #233a4b; } }
    }
    @media (prefers-reduced-motion: reduce) { *, *::before, *::after { animation: none !important; transition: none !important; } }
    @media (prefers-reduced-transparency: reduce) {
      .glass { background: #eaf0f4; backdrop-filter: none; -webkit-backdrop-filter: none; }
      @media (prefers-color-scheme: dark) { .glass { background: #233a4b; } }
    }
  </style>
</head>
<body>
  <svg class="symbols" aria-hidden="true" xmlns="http://www.w3.org/2000/svg"><defs>
    <symbol id="i-bluetooth" viewBox="0 0 24 24"><path d="m7 7 10 10-5 5V2l5 5L7 17"/></symbol>
    <symbol id="i-battery" viewBox="0 0 24 24"><rect x="2" y="6" width="17" height="12" rx="3"/><path d="M22 10v4"/></symbol>
    <symbol id="i-bulb" viewBox="0 0 24 24"><path d="M9 18v-2c0-2-3-3-3-7a6 6 0 0 1 12 0c0 4-3 5-3 7v2ZM9 21h6M10 18h4M10 10l2 2 2-2m-2 2v5"/></symbol>
    <symbol id="i-sun" viewBox="0 0 24 24"><circle cx="12" cy="12" r="4"/><path d="M12 2v2m0 16v2M2 12h2m16 0h2M5 5l1.5 1.5m11 11L19 19M5 19l1.5-1.5m11-11L19 5"/></symbol>
    <symbol id="i-moon" viewBox="0 0 24 24"><path d="M20 14.2A8.5 8.5 0 0 1 9.8 4 8.5 8.5 0 1 0 20 14.2Z"/></symbol>
    <symbol id="i-restore" viewBox="0 0 24 24"><path d="M3 11a9 9 0 1 1 2 7M3 4v7h7"/></symbol>
    <symbol id="i-radar" viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><circle cx="12" cy="12" r="5"/><path d="m12 12 6-6"/><circle cx="12" cy="12" r="1"/></symbol>
    <symbol id="i-arrow" viewBox="0 0 24 24"><path d="m9 5 7 7-7 7"/></symbol>
    <symbol id="i-close" viewBox="0 0 24 24"><path d="m6 6 12 12M6 18 18 6"/></symbol>
  </defs></svg>
  <div class="wallpaper" aria-hidden="true"><div class="orb orb-blue"></div><div class="orb orb-peach"></div><div class="orb orb-mint"></div></div>
  <main class="app">
    <header class="brand"><div class="wordmark">CCJ<span>中枢</span></div><div class="brand-detail"><svg class="icon"><use href="#i-bluetooth"/></svg>蓝牙网关</div></header>
    <section class="glass light-card" id="lightCard" aria-labelledby="lightTitle">
      <div class="device-bar">
        <div class="connection" id="connectionIndicator"><span class="dot" aria-hidden="true"></span><span id="connectionStatus" role="status">正在连接网关…</span></div>
        <button class="battery-badge battery-unknown" id="batteryBadge" onclick="fetchBattery()" title="刷新电量" aria-label="刷新开关电量"><svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><use href="#i-battery"/><rect class="battery-fill" id="batteryFill" x="4.5" y="8.5" width="0" height="7" rx="1"/><path id="batteryUnknown" d="M8.5 12h4"/></svg><span id="batteryText">--%</span></button>
      </div>
      <div class="light-art" aria-hidden="true"><svg class="icon"><use href="#i-bulb"/></svg></div>
      <h1 id="lightTitle">灯光控制</h1>
      <p class="device-caption" id="deviceCaption">连接后即可控制开关</p>
      <div class="light-slider" id="lightSlider" role="group" aria-label="灯光控制：向左关灯，向右开灯" aria-describedby="stateNote" aria-busy="false">
        <span class="slider-thumb" id="sliderThumb" aria-hidden="true"></span>
        <button class="control-btn off-btn" onclick="sendLightCommand(0)" aria-pressed="false" disabled><svg class="icon" aria-hidden="true"><use href="#i-moon"/></svg>关灯</button>
        <button class="control-btn on-btn" onclick="sendLightCommand(1)" aria-pressed="false" disabled><svg class="icon" aria-hidden="true"><use href="#i-sun"/></svg>开灯</button>
      </div>
      <p class="control-status" id="controlStatus" role="status">点击或滑动，选择开灯 / 关灯</p>
      <p class="state-note" id="stateNote">滑块位置表示最近操作，不代表实际灯态</p>
    </section>
    <section class="glass maintenance-card" aria-labelledby="maintenanceTitle">
      <div class="section-heading"><svg class="icon" aria-hidden="true"><use href="#i-bluetooth"/></svg><h2 id="maintenanceTitle">蓝牙固件升级</h2></div>
      <p class="section-copy">临时让出蓝牙连接，为开关更新固件。</p>
      <button class="release-btn" id="releaseBtn" onclick="setMaintenance('release')" disabled>
        <span class="release-progress" id="releaseProgress" aria-hidden="true"></span>
        <span><span class="release-title" id="releaseTitle">释放蓝牙连接</span><span class="release-hint" id="releaseHint">180 秒后自动恢复</span></span>
        <span class="release-time" id="releaseTime" aria-live="off">03:00</span>
      </button>
      <p class="maintenance-status" id="maintenanceStatus" role="status">正在读取状态…</p>
      <button class="resume-btn" id="resumeBtn" onclick="setMaintenance('resume')" hidden><svg class="icon" aria-hidden="true"><use href="#i-restore"/></svg>立即恢复连接</button>
    </section>
    <button class="settings-link" id="settingsBtn" onclick="openModal()" disabled><svg class="icon" aria-hidden="true"><use href="#i-radar"/></svg><span class="settings-label">设备管理<small>扫描与绑定开关</small></span><svg class="icon" aria-hidden="true"><use href="#i-arrow"/></svg></button>
    <p class="page-footer">CCJ GATEWAY</p>
  </main>
  <dialog class="glass" id="scanModal" aria-labelledby="scanTitle">
    <div class="modal-header"><h2 id="scanTitle">发现附近开关</h2><button class="close-btn" onclick="closeModal()" aria-label="关闭设备管理"><svg class="icon" aria-hidden="true"><use href="#i-close"/></svg></button></div>
    <p class="section-copy">选择你的 ZZK 开关，网关会记住这台设备。</p>
    <button class="btn-primary" id="scanBtn" onclick="scanDevices()">扫描附近设备</button>
    <div class="loader" id="loader" hidden aria-label="正在扫描"></div>
    <div class="device-list" id="deviceList"></div>
    <p class="modal-status" id="modalStatus" role="status"></p>
    <button class="btn-primary" id="bindBtn" onclick="bindDevice()" hidden>绑定所选开关</button>
  </dialog>
  <script>
    const $ = id => document.getElementById(id);
    const RELEASE_SECONDS = 180;
    let gatewayStatus = null, statusAvailable = false;
    let maintenanceRequestPending = false, statusRequestPending = false, statusRevision = 0;
    let isSendingCommand = false, lastAction = null, controlRevision = 0;
    let maintenanceDeadline = 0, scanning = false, binding = false, scanRevision = 0;

    async function request(url, options = {}, timeout = 8000) {
      const controller = new AbortController();
      const timer = setTimeout(() => controller.abort(), timeout);
      try {
        const response = await fetch(url, { cache: 'no-store', ...options, signal: controller.signal });
        // Read the body before clearing the deadline; a stalled body must not lock the UI.
        const text = await response.text();
        return { ok: response.ok, status: response.status, text, json: () => JSON.parse(text) };
      } finally { clearTimeout(timer); }
    }
    function blocked() { return !statusAvailable || !gatewayStatus || gatewayStatus.maintenance || maintenanceRequestPending; }
    function canControl() { return !blocked() && gatewayStatus.connected && !isSendingCommand; }
    function chooseAction(action) {
      lastAction = action;
      $('lightSlider').dataset.choice = $('lightCard').dataset.choice = action === null ? '' : String(action);
      document.querySelector('.off-btn').setAttribute('aria-pressed', String(action === 0));
      document.querySelector('.on-btn').setAttribute('aria-pressed', String(action === 1));
    }
    function clearAction(message = '点击或滑动，选择开灯 / 关灯') {
      ++controlRevision; chooseAction(null);
      $('controlStatus').className = 'control-status'; $('controlStatus').textContent = message;
    }
    function updateButtons() {
      document.querySelectorAll('.control-btn').forEach(button => { button.disabled = !canControl(); });
      $('lightSlider').setAttribute('aria-busy', String(isSendingCommand));
      $('settingsBtn').disabled = blocked() || scanning || binding;
      $('scanBtn').disabled = blocked() || scanning || binding;
      $('bindBtn').disabled = blocked() || scanning || binding;
      const transitional = gatewayStatus && ['releasing', 'resuming'].includes(gatewayStatus.maintenance_state);
      $('releaseBtn').disabled = !statusAvailable || !gatewayStatus || !gatewayStatus.bound_mac || maintenanceRequestPending || transitional || isSendingCommand || binding || scanning;
      if (gatewayStatus && gatewayStatus.maintenance_state === 'active' && maintenanceDeadline <= performance.now()) $('releaseBtn').disabled = true;
      $('resumeBtn').disabled = !statusAvailable || maintenanceRequestPending || !gatewayStatus || transitional;
    }
    function renderCountdown() {
      const phase = gatewayStatus ? gatewayStatus.maintenance_state : 'normal';
      $('releaseBtn').dataset.phase = phase;
      let title = '释放蓝牙连接', hint = '180 秒后自动恢复', time = '03:00', fraction = 0;
      if (!statusAvailable) { title = '等待网关连接'; hint = '连接恢复后自动同步'; time = '--:--'; }
      else if (phase === 'active') {
        const remaining = Math.max(0, Math.min(RELEASE_SECONDS, (maintenanceDeadline - performance.now()) / 1000));
        const seconds = Math.ceil(remaining);
        fraction = remaining / RELEASE_SECONDS;
        time = String(Math.floor(seconds / 60)).padStart(2, '0') + ':' + String(seconds % 60).padStart(2, '0');
        title = seconds > 0 ? '连接已释放' : '正在恢复连接';
        hint = seconds > 0 ? '轻点延长至 180 秒' : '等待网关确认恢复';
        $('releaseBtn').setAttribute('aria-label', '连接已释放，剩余 ' + seconds + ' 秒，点击延长至 180 秒');
      } else if (phase === 'releasing' || maintenanceRequestPending) { title = '正在处理…'; hint = '请等待网关确认'; time = '···'; }
      else if (phase === 'resuming') { title = '正在恢复连接'; hint = '即将返回灯光控制'; time = '···'; }
      if (phase !== 'active' || !statusAvailable) $('releaseBtn').setAttribute('aria-label', title + '，' + hint);
      $('releaseTitle').textContent = title; $('releaseHint').textContent = hint; $('releaseTime').textContent = time;
      $('releaseProgress').style.transform = 'scaleX(' + fraction + ')';
      updateButtons();
    }
    function renderStatus(data) {
      if (!data || typeof data.connected !== 'boolean' || typeof data.maintenance !== 'boolean' ||
          !['normal', 'releasing', 'active', 'resuming'].includes(data.maintenance_state)) throw new Error('无效状态');
      if (!gatewayStatus || data.bound_mac !== gatewayStatus.bound_mac || data.connected !== gatewayStatus.connected ||
          data.maintenance !== gatewayStatus.maintenance) clearAction(data.maintenance ? '蓝牙维护期间，灯光控制已暂停' :
            data.connected ? '点击或滑动，选择开灯 / 关灯' : '连接开关后可开始操作');
      gatewayStatus = data; statusAvailable = true;
      const phase = data.maintenance_state;
      maintenanceDeadline = phase === 'active' ? performance.now() + Math.max(0, Math.min(RELEASE_SECONDS, Number(data.remaining_seconds) || 0)) * 1000 : 0;
      $('connectionIndicator').dataset.connection = data.maintenance ? 'maintenance' : data.connected ? 'connected' : 'waiting';
      $('connectionStatus').textContent = data.maintenance ? '蓝牙维护中' : data.connected ? '开关已连接' : data.bound_mac ? '正在连接开关…' : '尚未绑定开关';
      $('deviceCaption').textContent = data.bound_mac ? '已绑定开关 · ' + data.bound_mac.slice(-5).toUpperCase() : '绑定开关后，即可开始控制';
      $('maintenanceStatus').textContent = phase === 'releasing' ? '正在断开蓝牙，请等倒计时开始后再烧录。' :
        phase === 'active' ? '可在附近电脑上烧录；到时自动恢复连接。' :
        phase === 'resuming' ? '正在退出维护，网关将自动重新连接。' : data.bound_mac ? '绑定会保留，到时自动重连。' : '请先在设备管理中绑定开关。';
      $('resumeBtn').hidden = !data.maintenance;
      if (data.maintenance) { ++scanRevision; $('bindBtn').hidden = true; }
      updateBatteryUI(data.battery); renderCountdown();
    }
    function showOffline() {
      statusAvailable = false; clearAction('网关暂时离线，正在重新连接…');
      $('connectionIndicator').dataset.connection = 'offline';
      $('connectionStatus').textContent = '网关暂时离线';
      $('maintenanceStatus').textContent = '状态暂不可用，网络恢复后自动同步。';
      updateBatteryUI(-1); renderCountdown();
    }
    async function fetchStatus() {
      if (statusRequestPending || maintenanceRequestPending || binding) return;
      statusRequestPending = true;
      const revision = statusRevision;
      try {
        const response = await request('/status');
        if (!response.ok) throw new Error('状态读取失败');
        if (revision === statusRevision) renderStatus(response.json());
      } catch (error) { if (revision === statusRevision) showOffline(); }
      finally { statusRequestPending = false; }
    }
    async function setMaintenance(action) {
      if (!['release', 'resume'].includes(action) || $(action === 'release' ? 'releaseBtn' : 'resumeBtn').disabled) return;
      maintenanceRequestPending = true; ++statusRevision; ++scanRevision;
      clearAction('正在切换蓝牙连接模式…'); renderCountdown();
      try {
        const response = await request('/maintenance', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: 'action=' + action });
        if (!response.ok) throw new Error(response.status === 409 ? '请先绑定开关' : '操作未完成');
        renderStatus(response.json());
      } catch (error) { showOffline(); $('maintenanceStatus').textContent = '操作结果待确认，正在同步网关状态…'; }
      finally { maintenanceRequestPending = false; renderCountdown(); fetchStatus(); }
    }
    async function fetchBattery() {
      await fetchStatus();
    }
    function updateBatteryUI(value) {
      const level = typeof value === 'number' ? value : typeof value === 'string' && value.trim() ? Number(value) : NaN;
      const valid = Number.isInteger(level) && level >= 0 && level <= 100;
      $('batteryText').textContent = valid ? level + '%' : '--%';
      $('batteryFill').style.width = (valid ? 12 * level / 100 : 0) + 'px';
      $('batteryUnknown').toggleAttribute('hidden', valid);
      $('batteryBadge').classList.toggle('battery-unknown', !valid);
      $('batteryBadge').classList.toggle('battery-low', valid && level <= 20);
      $('batteryBadge').classList.toggle('battery-mid', valid && level > 20 && level <= 50);
      const label = (valid ? '开关电量 ' + level + '%' : '电量暂不可用') + '，点击刷新';
      $('batteryBadge').setAttribute('aria-label', label);
      $('batteryBadge').title = label;
    }
    if (window.EventSource) {
      const source = new EventSource('/events');
      source.addEventListener('battery', event => { if (statusAvailable && gatewayStatus && gatewayStatus.connected && !gatewayStatus.maintenance) updateBatteryUI(event.data); });
    }
    async function sendLightCommand(action) {
      if ((action !== 0 && action !== 1) || !canControl()) return;
      const revision = controlRevision;
      isSendingCommand = true; updateButtons();
      $('controlStatus').className = 'control-status';
      $('controlStatus').textContent = action === 1 ? '正在发送开灯指令…' : '正在发送关灯指令…';
      try {
        const response = await request('/control?action=' + action);
        if (!response.ok) throw new Error(response.status === 409 ? '维护期间无法控制开关' : response.status === 503 ? '开关尚未连接' : '指令发送失败，请重试');
        if (revision !== controlRevision || blocked() || !gatewayStatus.connected) return;
        chooseAction(action);
        $('controlStatus').className = 'control-status success';
        $('controlStatus').textContent = action === 1 ? '开灯指令已交给网关' : '关灯指令已交给网关';
      } catch (error) {
        if (revision === controlRevision) { chooseAction(null); $('controlStatus').className = 'control-status error'; $('controlStatus').textContent = error.name === 'AbortError' ? '指令结果待确认，请检查灯光后重试' : error.message; }
        fetchStatus();
      } finally { isSendingCommand = false; updateButtons(); }
    }
    // Both segments remain explicit commands. A drag or arrow key selects one;
    // no light state is guessed or stored across page loads.
    const slider = $('lightSlider');
    let gesture = null, suppressClickUntil = 0;
    slider.addEventListener('pointerdown', event => {
      if (!canControl() || !event.isPrimary || event.button !== 0) return;
      gesture = { id: event.pointerId, x: event.clientX, y: event.clientY, dragging: false, action: null };
    });
    slider.addEventListener('pointermove', event => {
      if (!gesture || event.pointerId !== gesture.id) return;
      const dx = event.clientX - gesture.x, dy = event.clientY - gesture.y;
      if (!gesture.dragging && Math.abs(dy) > 10 && Math.abs(dy) > Math.abs(dx)) { gesture = null; return; }
      if (!gesture.dragging && Math.abs(dx) > 10 && Math.abs(dx) > Math.abs(dy)) {
        gesture.dragging = true; slider.setPointerCapture(event.pointerId); slider.classList.add('dragging');
      }
      if (!gesture.dragging) return;
      const rect = slider.getBoundingClientRect();
      const position = Math.max(0, Math.min(1, (event.clientX - rect.left - rect.width / 4) / (rect.width / 2)));
      gesture.action = position >= .5 ? 1 : 0;
      $('sliderThumb').style.transform = 'translateX(' + position * 100 + '%)';
    });
    function endGesture(event, cancelled) {
      if (!gesture || event.pointerId !== gesture.id) return;
      const completed = gesture; gesture = null;
      slider.classList.remove('dragging'); $('sliderThumb').style.transform = '';
      if (slider.hasPointerCapture(event.pointerId)) slider.releasePointerCapture(event.pointerId);
      if (completed.dragging) {
        suppressClickUntil = performance.now() + 400;
        if (!cancelled && canControl()) sendLightCommand(completed.action);
      }
    }
    slider.addEventListener('pointerup', event => endGesture(event, false));
    slider.addEventListener('pointercancel', event => endGesture(event, true));
    slider.addEventListener('lostpointercapture', event => {
      // Touch begins with implicit capture on a child button. Its capture-loss
      // event bubbles when the track takes over; that is not a cancelled drag.
      if (event.target === slider) endGesture(event, true);
    });
    slider.addEventListener('click', event => { if (performance.now() < suppressClickUntil) { event.preventDefault(); event.stopPropagation(); } }, true);
    slider.addEventListener('keydown', event => {
      if (!['ArrowLeft', 'ArrowRight'].includes(event.key) || event.repeat || !canControl()) return;
      event.preventDefault(); const action = event.key === 'ArrowRight' ? 1 : 0;
      document.querySelector(action ? '.on-btn' : '.off-btn').focus(); sendLightCommand(action);
    });

    const modal = $('scanModal');
    function openModal() {
      if (blocked()) return;
      modal.showModal(); $('scanBtn').focus();
    }
    function closeModal() {
      ++scanRevision; modal.close(); $('deviceList').replaceChildren(); $('bindBtn').hidden = true;
      $('modalStatus').textContent = ''; $('settingsBtn').focus();
    }
    modal.addEventListener('cancel', event => { event.preventDefault(); closeModal(); });
    modal.addEventListener('click', event => {
      const rect = modal.getBoundingClientRect();
      if (event.target === modal && (event.clientX < rect.left || event.clientX > rect.right || event.clientY < rect.top || event.clientY > rect.bottom)) closeModal();
    });
    async function scanDevices() {
      if (blocked() || scanning || binding) return;
      scanning = true; const revision = ++scanRevision;
      $('loader').hidden = false; $('deviceList').replaceChildren(); $('bindBtn').hidden = true;
      $('modalStatus').textContent = '正在寻找附近的开关…'; updateButtons();
      try {
        const response = await request('/scan', {}, 12000);
        if (!response.ok) throw new Error(response.status === 409 ? '蓝牙正忙，请稍后重试' : '扫描失败，请重试');
        const devices = response.json();
        if (revision !== scanRevision || blocked() || !modal.open) return;
        if (!Array.isArray(devices)) throw new Error('扫描结果无效');
        $('modalStatus').textContent = devices.length ? '选择设备后，点击下方绑定。' : '暂未发现开关，请靠近后重新扫描。';
        devices.forEach((device, index) => {
          const item = document.createElement('label'); item.className = 'device-item';
          const info = document.createElement('div'); info.className = 'device-info';
          const name = document.createElement('span'); name.className = 'device-name'; name.textContent = device.name || '未命名 ZZK 开关';
          const mac = document.createElement('span'); mac.className = 'device-mac'; mac.textContent = device.mac || '';
          const signal = document.createElement('span'); signal.className = 'device-rssi'; signal.textContent = '信号 ' + device.rssi + ' dBm'; signal.style.color = device.rssi > -70 ? 'var(--green)' : device.rssi > -85 ? 'var(--amber)' : 'var(--danger)';
          const input = document.createElement('input'); input.type = 'radio'; input.name = 'macSelect'; input.value = device.mac || ''; input.checked = index === 0;
          info.append(name, mac, signal); item.append(info, input); $('deviceList').append(item);
        });
        $('bindBtn').hidden = !devices.length;
      } catch (error) { if (revision === scanRevision) $('modalStatus').textContent = error.name === 'AbortError' ? '扫描超时，请重试' : error.message; }
      finally { scanning = false; $('loader').hidden = true; $('scanBtn').textContent = '重新扫描'; updateButtons(); }
    }
    async function bindDevice() {
      const selected = document.querySelector('input[name="macSelect"]:checked');
      if (!selected || blocked() || scanning || binding) return;
      binding = true; ++statusRevision; updateButtons();
      try {
        const response = await request('/bind?mac=' + encodeURIComponent(selected.value));
        if (!response.ok) throw new Error(response.status === 409 ? '蓝牙正忙，请稍后重试' : '绑定失败，请重试');
        clearAction(); closeModal();
      } catch (error) { $('modalStatus').textContent = error.name === 'AbortError' ? '绑定结果待确认，请刷新状态' : error.message; }
      finally { binding = false; updateButtons(); fetchStatus(); }
    }
    document.addEventListener('visibilitychange', () => { if (!document.hidden) fetchStatus(); });
    fetchStatus(); setInterval(fetchStatus, 1000); setInterval(renderCountdown, 250);
  </script>
</body>
</html>

)rawliteral";

#endif
