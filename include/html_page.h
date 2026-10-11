#ifndef HTML_PAGE_H
#define HTML_PAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>CCJ Gateway</title>
  <style>
    /* 基础与苹果风排版 */
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background-color: #f2f2f7; margin: 0; padding: 24px 0; box-sizing: border-box; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; }
    .card { box-sizing: border-box; background: white; border-radius: 28px; padding: 32px 24px; box-shadow: 0 10px 30px rgba(0,0,0,0.08); text-align: center; width: calc(100% - 32px); max-width: 420px; }

    /* 顶部标题与电池排版 */
    .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 35px; }
    .title { font-size: 24px; font-weight: 700; color: #1c1c1e; margin: 0; letter-spacing: 0.5px; }

    /* 高颜值电池徽章 */
    .battery-badge { display: flex; align-items: center; background: #f2f2f7; padding: 6px 12px; border-radius: 20px; font-size: 14px; font-weight: 600; color: #34c759; cursor: pointer; transition: background 0.2s; }
    .battery-badge:active { background: #e5e5ea; }
    .battery-icon { margin-right: 4px; font-size: 16px; }
    .battery-low { color: #ff3b30; } /* 低电量变红 */
    .battery-mid { color: #ff9500; } /* 中电量变橙 */

    /* 独立的开灯与关灯指令按钮（不再假设网页知道灯的真实状态） */
    .control-buttons { display: flex; justify-content: center; gap: 18px; }
    .control-btn { flex: 1; max-width: 125px; aspect-ratio: 1; border-radius: 50%; border: none; font-size: 17px; font-weight: 600; cursor: pointer; transition: transform 0.15s, box-shadow 0.2s, opacity 0.2s; display: flex; flex-direction: column; align-items: center; justify-content: center; -webkit-tap-highlight-color: transparent; outline: none; }
    .control-btn:active { transform: scale(0.95); }
    .control-btn:disabled { cursor: default; opacity: 0.55; }
    .on-btn { background: #ff9500; color: white; box-shadow: 0 12px 28px rgba(255, 149, 0, 0.32); }
    .off-btn { background: #e5e5ea; color: #3a3a3c; box-shadow: inset 0 0 15px rgba(0,0,0,0.05); }
    .control-icon { font-size: 34px; margin-bottom: 8px; }
    .control-status { min-height: 20px; margin-top: 18px; color: #8e8e93; font-size: 14px; font-weight: 500; }
    .control-status.success { color: #34c759; }
    .control-status.error { color: #ff3b30; }

    /* 底部设备管理入口 */
    .settings-link { margin-top: 24px; color: #007aff; font-size: 15px; font-weight: 500; cursor: pointer; padding: 10px; border-radius: 12px; transition: background 0.2s; border: 0; background: transparent; }
    .settings-link:active { background: #e5e5ea; }
    button:disabled { opacity: 0.5; cursor: default; }
    .maintenance-panel { margin-top: 24px; padding-top: 20px; border-top: 1px solid #e5e5ea; text-align: left; }
    .maintenance-title { font-size: 16px; font-weight: 600; color: #1c1c1e; }
    .maintenance-copy { font-size: 13px; line-height: 1.6; color: #636366; margin: 8px 0; }
    .connection-status { font-size: 14px; line-height: 1.5; color: #636366; margin: 0 0 16px; }

    /* 全屏毛玻璃弹窗 (扫描面板) */
    .modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.4); backdrop-filter: blur(5px); -webkit-backdrop-filter: blur(5px); align-items: center; justify-content: center; z-index: 999; opacity: 0; transition: opacity 0.3s; }
    .modal.show { display: flex; opacity: 1; }
    .modal-content { box-sizing: border-box; background: white; border-radius: 24px; padding: 25px; width: calc(100% - 32px); max-width: 410px; max-height: 80vh; overflow-y: auto; box-shadow: 0 20px 40px rgba(0,0,0,0.2); }
    .modal-title { font-size: 20px; font-weight: 700; margin-bottom: 20px; color: #1c1c1e; }

    /* 设备列表样式 */
    .device-item { display: flex; justify-content: space-between; align-items: center; padding: 15px 10px; border-bottom: 1px solid #f2f2f7; cursor: pointer; border-radius: 12px; }
    .device-item:active { background: #f2f2f7; }
    .device-info { display: flex; flex-direction: column; text-align: left; }
    .device-name { font-weight: 600; color: #1c1c1e; font-size: 16px; margin-bottom: 4px; }
    .device-mac { font-size: 12px; color: #8e8e93; font-family: monospace; }
    .device-rssi { font-size: 12px; color: #34c759; font-weight: 500; margin-top: 2px; }

    /* 弹窗按钮 */
    .btn-primary { background: #007aff; color: white; border: none; padding: 14px; border-radius: 14px; width: 100%; font-size: 16px; font-weight: 600; margin-top: 20px; cursor: pointer; -webkit-tap-highlight-color: transparent; }
    .btn-primary:active { background: #0062cc; }
    .btn-secondary { background: #f2f2f7; color: #007aff; border: none; padding: 14px; border-radius: 14px; width: 100%; font-size: 16px; font-weight: 600; margin-top: 10px; cursor: pointer; -webkit-tap-highlight-color: transparent; }
    .btn-secondary:active { background: #e5e5ea; }

    /* 扫描动画 */
    .loader { display: none; margin: 15px auto; border: 3px solid #f2f2f7; border-top: 3px solid #007aff; border-radius: 50%; width: 28px; height: 28px; animation: spin 1s linear infinite; }
    @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
  </style>
</head>
<body>

  <!-- 主控面板 -->
  <div class="card">
    <!-- 顶部标题与电池区域 -->
    <div class="header">
      <div class="title">ZZK 中枢</div>
      <div class="battery-badge" id="batteryBadge" onclick="fetchBattery()" title="点击刷新电量">
        <span class="battery-icon">🔋</span>
        <span id="batteryText">--%</span>
      </div>
    </div>

    <!-- 独立的开灯与关灯指令区域 -->
    <div class="connection-status" id="connectionStatus" role="status">正在读取连接状态…</div>
    <div class="control-buttons">
      <button class="control-btn on-btn" onclick="sendLightCommand(1)" aria-label="开灯" disabled>
        <span class="control-icon">☀</span>
        <span>开灯</span>
      </button>
      <button class="control-btn off-btn" onclick="sendLightCommand(0)" aria-label="关灯" disabled>
        <span class="control-icon">☾</span>
        <span>关灯</span>
      </button>
    </div>
    <div class="control-status" id="controlStatus">请选择操作</div>

    <div class="maintenance-panel">
      <div class="maintenance-title">蓝牙固件升级</div>
      <p class="maintenance-copy">临时释放连接 180 秒，让附近的电脑或手机连接开关烧录。绑定会保留，到时自动恢复。</p>
      <p class="maintenance-copy" id="maintenanceStatus" role="status">正在读取状态…</p>
      <button class="btn-secondary" id="releaseBtn" onclick="setMaintenance('release')" disabled>临时释放连接（180 秒）</button>
      <button class="btn-primary" id="resumeBtn" onclick="setMaintenance('resume')" hidden>立即恢复连接</button>
    </div>
    <button class="settings-link" id="settingsBtn" onclick="openModal()" disabled>⚙️ 周边设备雷达扫描</button>
  </div>

  <!-- 雷达扫描与绑定弹窗 (保持不变) -->
  <div class="modal" id="scanModal">
    <div class="modal-content">
      <div class="modal-title">发现附近 ZZK 开关</div>
      <button class="btn-primary" id="scanBtn" onclick="scanDevices()">📡 开启蓝牙雷达</button>
      <div class="loader" id="loader"></div>
      <div id="deviceList" style="margin-top: 15px;"></div>
      <button class="btn-primary" id="bindBtn" style="display: none;" onclick="bindDevice()">🔗 保存专属绑定</button>
      <button class="btn-secondary" onclick="closeModal()">返回</button>
    </div>
  </div>

  <script>
    let gatewayStatus = null;
    let statusAvailable = false;
    let maintenanceRequestPending = false;
    let statusRequestPending = false;
    let statusRevision = 0;
    let isSendingCommand = false;

    function updateButtons() {
      const blocked = !statusAvailable || !gatewayStatus || gatewayStatus.maintenance || maintenanceRequestPending;
      document.querySelectorAll('.control-btn').forEach(button => {
        button.disabled = blocked || !gatewayStatus.connected || isSendingCommand;
      });
      ['settingsBtn', 'scanBtn', 'bindBtn'].forEach(id => document.getElementById(id).disabled = blocked);
      document.getElementById('releaseBtn').disabled = !statusAvailable || !gatewayStatus ||
        !gatewayStatus.bound_mac || maintenanceRequestPending ||
        ['releasing', 'resuming'].includes(gatewayStatus.maintenance_state);
      document.getElementById('resumeBtn').disabled = !statusAvailable || maintenanceRequestPending ||
        !gatewayStatus || ['releasing', 'resuming'].includes(gatewayStatus.maintenance_state);
    }

    function renderStatus(data) {
      gatewayStatus = data;
      statusAvailable = true;
      const phase = data.maintenance_state;
      document.getElementById('connectionStatus').textContent = data.maintenance ? '蓝牙维护中 · 灯控已暂停' :
        data.connected ? '开关已连接' : data.bound_mac ? '正在连接已绑定开关…' : '尚未绑定开关';
      document.getElementById('maintenanceStatus').textContent = phase === 'releasing' ? '正在释放连接，请等待倒计时开始后再烧录…' :
        phase === 'active' ? '连接已释放，可开始烧录 · ' + data.remaining_seconds + ' 秒后自动恢复' :
        phase === 'resuming' ? '正在恢复连接…' : data.bound_mac ? '正常连接模式' : '请先扫描并绑定开关';
      document.getElementById('releaseBtn').textContent = phase === 'active' ? '延长至 180 秒' : '临时释放连接（180 秒）';
      document.getElementById('resumeBtn').hidden = !data.maintenance;
      updateBatteryUI(data.battery);
      updateButtons();
    }

    async function fetchStatus() {
      if (statusRequestPending || maintenanceRequestPending) return;
      statusRequestPending = true;
      const revision = statusRevision;
      try {
        const response = await fetch('/status', { cache: 'no-store' });
        if (!response.ok) throw new Error('状态读取失败');
        const data = await response.json();
        if (revision === statusRevision) renderStatus(data);
      } catch (err) {
        if (revision === statusRevision) {
          statusAvailable = false;
          document.getElementById('connectionStatus').textContent = '无法连接网关，正在重试…';
          document.getElementById('maintenanceStatus').textContent = '状态暂不可用，恢复网络后自动同步';
          updateButtons();
        }
      } finally { statusRequestPending = false; }
    }

    async function setMaintenance(action) {
      if (maintenanceRequestPending) return;
      maintenanceRequestPending = true;
      ++statusRevision; // Ignore a status poll started before this request.
      updateButtons();
      try {
        const response = await fetch('/maintenance', {
          method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'action=' + encodeURIComponent(action), cache: 'no-store'
        });
        if (!response.ok) throw new Error(response.status === 409 ? '请先绑定开关' : '操作失败，请重试');
        renderStatus(await response.json());
        document.getElementById('controlStatus').textContent = '请选择操作';
        document.getElementById('controlStatus').className = 'control-status';
      } catch (err) {
        // A lost response may still have changed the gateway: wait for status.
        statusAvailable = false;
        document.getElementById('maintenanceStatus').textContent = err.message + '，正在同步状态…';
      } finally {
        maintenanceRequestPending = false;
        updateButtons();
        fetchStatus();
      }
    }

    // ================== 电量读取逻辑 ==================
    // 1. 手动点击刷新（加入时间戳，彻底粉碎浏览器缓存！）
    function fetchBattery() {
      // 在网址后面加个随机时间戳 ?t=...，骗过浏览器，强迫它每次都去问 ESP32 拿最新数据
      fetch('/battery?t=' + new Date().getTime())
        .then(response => response.text())
        .then(val => {
          updateBatteryUI(val);
        })
        .catch(err => console.log('电量抓取失败，请检查网关接口'));
    }

    // 2. 高级玩法：建立 SSE 实时监听通道
    // 只要网关蓝牙一收到 ZZK 的推送，网页瞬间就能自动刷新，0 延迟！
    if (!!window.EventSource) {
      const source = new EventSource('/events');

      // 专门监听名为 "battery" 的事件
      source.addEventListener('battery', function(e) {
        console.log("🚀 触发 SSE 实时电量推送: ", e.data);
        updateBatteryUI(e.data);
      }, false);
    }

    // 3. 统一的 UI 渲染函数
    function updateBatteryUI(val) {
      const batteryVal = parseInt(val);
      const badge = document.getElementById('batteryBadge');
      const text = document.getElementById('batteryText');

      if (!isNaN(batteryVal) && batteryVal >= 0 && batteryVal <= 100) {
        text.innerText = batteryVal + '%';

        // 苹果风色彩反馈
        badge.classList.remove('battery-low', 'battery-mid');
        if (batteryVal <= 20) {
          badge.classList.add('battery-low');
        } else if (batteryVal <= 50) {
          badge.classList.add('battery-mid');
        }
      } else {
        text.innerText = '--%';
        badge.classList.remove('battery-low', 'battery-mid');
      }
    }

    // 页面加载 1 秒后主动拉取一次兜底
    setTimeout(fetchBattery, 1000);


    // ================== 开灯/关灯指令逻辑 ==================
    // 页面不保存或猜测灯的状态，每个按钮只发送一个明确动作。
    function sendLightCommand(action) {
      if (isSendingCommand || !statusAvailable || !gatewayStatus || !gatewayStatus.connected ||
          gatewayStatus.maintenance || maintenanceRequestPending) return;

      const buttons = document.querySelectorAll('.control-btn');
      const status = document.getElementById('controlStatus');
      if (navigator.vibrate) navigator.vibrate(50);

      isSendingCommand = true;
      buttons.forEach(button => button.disabled = true);
      status.className = 'control-status';
      status.innerText = action === 1 ? '正在发送开灯指令…' : '正在发送关灯指令…';

      fetch('/control?action=' + action, { cache: 'no-store' })
        .then(response => {
          if (!response.ok) throw new Error(response.status === 409 ? '维护期间无法控制开关' :
            response.status === 503 ? '开关尚未连接' : '指令发送失败');
          status.className = 'control-status success';
          status.innerText = action === 1 ? '✓ 开灯指令已发送至网关' : '✓ 关灯指令已发送至网关';
        })
        .catch(err => {
          status.className = 'control-status error';
          status.innerText = '⚠️ ' + err.message;
          fetchStatus();
        })
        .finally(() => {
          isSendingCommand = false;
          updateButtons();
        });
    }

    // ================== 弹窗与雷达管理逻辑 ==================
    const modal = document.getElementById('scanModal');
    function openModal() { modal.classList.add('show'); }
    function closeModal() {
      modal.classList.remove('show');
      document.getElementById('deviceList').innerHTML = '';
      document.getElementById('bindBtn').style.display = 'none';
      document.getElementById('scanBtn').style.display = 'block';
    }

    function scanDevices() {
      document.getElementById('scanBtn').style.display = 'none';
      document.getElementById('loader').style.display = 'block';
      document.getElementById('deviceList').innerHTML = '';
      document.getElementById('bindBtn').style.display = 'none';

      fetch('/scan', { cache: 'no-store' })
        .then(response => {
          if (!response.ok) throw new Error(response.status === 409 ? '蓝牙正在连接或维护中，请稍后重试' : '扫描失败');
          return response.json();
        })
        .then(data => {
          document.getElementById('loader').style.display = 'none';
          document.getElementById('scanBtn').style.display = 'block';
          document.getElementById('scanBtn').innerText = '🔄 重新扫描';

          const list = document.getElementById('deviceList');
          if (data.length === 0) {
            const empty = document.createElement('div');
            empty.style.cssText = 'text-align:center; color:#8e8e93; font-size:14px; padding:20px 0;';
            empty.textContent = '空气中极其安静，未发现设备';
            list.appendChild(empty);
            return;
          }

          data.forEach((dev, index) => {
            let rssiColor = dev.rssi > -70 ? '#34c759' : (dev.rssi > -85 ? '#ff9500' : '#ff3b30');

            const item = document.createElement('label');
            item.className = 'device-item';

            const info = document.createElement('div');
            info.className = 'device-info';

            const name = document.createElement('span');
            name.className = 'device-name';
            name.textContent = dev.name || 'ZZK_Switch (未命名)';

            const mac = document.createElement('span');
            mac.className = 'device-mac';
            mac.textContent = dev.mac || '';

            const rssi = document.createElement('span');
            rssi.className = 'device-rssi';
            rssi.style.color = rssiColor;
            rssi.textContent = '信号: ' + dev.rssi + ' dBm';

            const input = document.createElement('input');
            input.type = 'radio';
            input.name = 'macSelect';
            input.value = dev.mac || '';
            input.checked = index === 0;
            input.style.cssText = 'width:22px; height:22px; accent-color: #007aff;';

            info.appendChild(name);
            info.appendChild(mac);
            info.appendChild(rssi);
            item.appendChild(info);
            item.appendChild(input);
            list.appendChild(item);
          });
          document.getElementById('bindBtn').style.display = 'block';
        })
        .catch(err => {
          document.getElementById('loader').style.display = 'none';
          document.getElementById('scanBtn').style.display = 'block';
          alert(err.message);
        });
    }

    function bindDevice() {
      const selected = document.querySelector('input[name="macSelect"]:checked');
      if (!selected) return;
      fetch('/bind?mac=' + encodeURIComponent(selected.value))
        .then(response => {
          if (response.ok) {
            alert('🎉 网关已永久记忆该设备的 MAC 地址。');
            closeModal();
            fetchStatus();
          } else {
            alert(response.status === 409 ? '蓝牙正在连接或维护中，请稍后重试' : '⚠️ 绑定失败');
          }
        }).catch(() => alert('无法连接网关，请重试'));
    }
    fetchStatus();
    setInterval(fetchStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

#endif
