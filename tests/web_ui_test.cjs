// Run with Node.js and Playwright installed: node tests/web_ui_test.cjs
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { chromium } = require('playwright');

(async () => {
  const html = fs.readFileSync(path.join(__dirname, '../include/html_page.h'), 'utf8')
    .split('R"rawliteral(')[1].split(')rawliteral"')[0];
  const browser = await chromium.launch({ headless: true, channel: process.env.PLAYWRIGHT_CHANNEL || undefined });
  try {
    const page = await browser.newPage({ viewport: { width: 390, height: 844 } });
    const errors = [];
    const actions = [];
    page.on('pageerror', error => errors.push(error.message));
    let offline = false;
    let data = { bound_mac: '', connected: false, battery: -1, maintenance: false,
      maintenance_state: 'normal', remaining_seconds: 0 };
    await page.route('http://gateway.test/**', async route => {
      const url = new URL(route.request().url());
      if (url.pathname === '/') return route.fulfill({ contentType: 'text/html', body: html });
      if (url.pathname === '/status') {
        if (offline) return route.abort();
        return route.fulfill({ json: data });
      }
      if (url.pathname === '/maintenance') {
        assert.equal(route.request().method(), 'POST');
        const action = new URLSearchParams(route.request().postData()).get('action');
        actions.push(action);
        data = { ...data, maintenance: true, remaining_seconds: 0,
          maintenance_state: action === 'release' ? 'releasing' : 'resuming' };
        return route.fulfill({ status: 202, json: data });
      }
      if (url.pathname === '/battery') return route.fulfill({ body: String(data.battery) });
      if (url.pathname === '/events') return route.fulfill({ contentType: 'text/event-stream', body: '' });
      if (url.pathname === '/control') return route.fulfill({ body: 'OK' });
      return route.fulfill({ status: 404, body: '' });
    });
    const sync = async () => {
      await page.waitForFunction(() => !statusRequestPending && !maintenanceRequestPending);
      await page.evaluate(() => fetchStatus());
    };
    const disabled = selector => page.locator(selector).isDisabled();
    await page.goto('http://gateway.test/');
    await sync();
    assert(await disabled('#releaseBtn'));
    assert(await disabled('.on-btn'));
    assert.equal(await page.locator('#batteryText').innerText(), '--%');

    data = { ...data, bound_mac: 'c1:23:45:67:89:ab', connected: true, battery: 72 };
    await sync();
    assert(!(await disabled('.on-btn')));
    assert(!(await disabled('#releaseBtn')));
    await page.locator('#releaseBtn').click();
    await page.waitForFunction(() => gatewayStatus.maintenance_state === 'releasing');
    assert(await disabled('.on-btn'));
    assert(await disabled('#scanBtn'));
    assert(await disabled('#bindBtn'));
    assert(await disabled('#resumeBtn'));

    data = { ...data, connected: false, battery: -1, maintenance_state: 'active', remaining_seconds: 180 };
    await sync();
    assert((await page.locator('#maintenanceStatus').innerText()).includes('180 秒'));
    assert(!(await disabled('#resumeBtn')));
    assert.equal(await page.locator('#batteryText').innerText(), '--%');
    // Refresh must retain the gateway's maintenance state and countdown.
    data.remaining_seconds = 95;
    await page.reload();
    await sync();
    assert((await page.locator('#maintenanceStatus').innerText()).includes('95 秒'));
    assert(await disabled('.off-btn'));
    assert.equal(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), true);
    if (process.env.CCJ_UI_SCREENSHOT) await page.screenshot({ path: process.env.CCJ_UI_SCREENSHOT, fullPage: true });
    await page.setViewportSize({ width: 320, height: 568 });
    assert.equal(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), true);
    await page.setViewportSize({ width: 390, height: 844 });

    await page.locator('#releaseBtn').click();
    await page.waitForFunction(() => gatewayStatus.maintenance_state === 'releasing');
    data = { ...data, maintenance_state: 'active', remaining_seconds: 180 };
    await sync();
    await page.locator('#resumeBtn').click();
    await page.waitForFunction(() => gatewayStatus.maintenance_state === 'resuming');
    assert(await disabled('.on-btn'));
    data = { ...data, maintenance: false, maintenance_state: 'normal', remaining_seconds: 0 };
    await sync();
    assert(await disabled('.on-btn')); // Reconnecting is not yet ready.
    data.connected = true;
    await sync();
    assert(!(await disabled('.on-btn')));

    offline = true;
    await sync();
    assert(await disabled('.on-btn'));
    assert(await disabled('#releaseBtn'));
    offline = false;
    await sync();
    assert(!(await disabled('.on-btn')));
    assert.deepEqual(actions, ['release', 'release', 'resume']);
    assert.deepEqual(errors, []);
    console.log('Web UI: release, extend, reload, resume, offline recovery and mobile layout passed');
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
