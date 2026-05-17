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
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background-color: #f2f2f7; margin: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; overflow: hidden; }
    .card { background: white; border-radius: 28px; padding: 35px 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.08); text-align: center; width: 85%; max-width: 360px; }

    /* 顶部标题与电池排版 */
    .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 35px; }
    .title { font-size: 24px; font-weight: 700; color: #1c1c1e; margin: 0; letter-spacing: 0.5px; }

    /* 高颜值电池徽章 */
    .battery-badge { display: flex; align-items: center; background: #f2f2f7; padding: 6px 12px; border-radius: 20px; font-size: 14px; font-weight: 600; color: #34c759; cursor: pointer; transition: background 0.2s; }
    .battery-badge:active { background: #e5e5ea; }
    .battery-icon { margin-right: 4px; font-size: 16px; }
    .battery-low { color: #ff3b30; } /* 低电量变红 */
    .battery-mid { color: #ff9500; } /* 中电量变橙 */

    /* 高颜值拟物态 Toggle 按键 */
    .toggle-btn { width: 130px; height: 130px; border-radius: 50%; border: none; background: #e5e5ea; color: #8e8e93; font-size: 18px; font-weight: 600; cursor: pointer; transition: all 0.4s cubic-bezier(0.25, 0.8, 0.25, 1); box-shadow: inset 0 0 15px rgba(0,0,0,0.05); display: flex; flex-direction: column; align-items: center; justify-content: center; margin: 0 auto; -webkit-tap-highlight-color: transparent; outline: none; }
    .toggle-btn.active { background: #ff9500; color: white; box-shadow: 0 15px 35px rgba(255, 149, 0, 0.4); }
    .power-icon { font-size: 38px; margin-bottom: 8px; }

    /* 底部设备管理入口 */
    .settings-link { margin-top: 40px; color: #007aff; font-size: 15px; font-weight: 500; cursor: pointer; padding: 10px; border-radius: 12px; transition: background 0.2s; }
    .settings-link:active { background: #e5e5ea; }

    /* 全屏毛玻璃弹窗 (扫描面板) */
    .modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.4); backdrop-filter: blur(5px); -webkit-backdrop-filter: blur(5px); align-items: center; justify-content: center; z-index: 999; opacity: 0; transition: opacity 0.3s; }
    .modal.show { display: flex; opacity: 1; }
    .modal-content { background: white; border-radius: 24px; padding: 25px; width: 85%; max-width: 360px; max-height: 80vh; overflow-y: auto; box-shadow: 0 20px 40px rgba(0,0,0,0.2); }
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

    <!-- 开关区域 -->
    <button class="toggle-btn" id="mainToggle" onclick="toggleLight()">
      <div class="power-icon">⏻</div>
      <span id="btnText">已关闭</span>
    </button>

    <div class="settings-link" onclick="openModal()">⚙️ 周边设备雷达扫描</div>
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

      if (!isNaN(batteryVal)) {
        text.innerText = batteryVal + '%';

        // 苹果风色彩反馈
        badge.classList.remove('battery-low', 'battery-mid');
        if (batteryVal <= 20) {
          badge.classList.add('battery-low');
        } else if (batteryVal <= 50) {
          badge.classList.add('battery-mid');
        }
      }
    }

    // 页面加载 1 秒后主动拉取一次兜底
    setTimeout(fetchBattery, 1000);


    // ================== 开关控制逻辑 ==================
    let isLightOn = false;

    function toggleLight() {
      const btn = document.getElementById('mainToggle');
      const text = document.getElementById('btnText');
      if (navigator.vibrate) navigator.vibrate(50);

      isLightOn = !isLightOn;
      const action = isLightOn ? 1 : 0;

      if (isLightOn) {
        btn.classList.add('active');
        text.innerText = '已开启';
      } else {
        btn.classList.remove('active');
        text.innerText = '已关闭';
      }

      fetch('/control?action=' + action)
        .then(response => {
          if (!response.ok) throw new Error('网关拒绝');
        }).catch(err => {
          alert('⚠️ 指令发送失败，请检查网络');
          isLightOn = !isLightOn;
          btn.classList.toggle('active');
          text.innerText = isLightOn ? '已开启' : '已关闭';
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

      fetch('/scan')
        .then(response => response.json())
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
          alert('🚨 扫描超时！请确保网关正常。');
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
          } else {
            alert('⚠️ 绑定失败');
          }
        });
    }
  </script>
</body>
</html>
)rawliteral";

#endif
