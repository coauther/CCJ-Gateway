// html_page.h
#ifndef HTML_PAGE_H
#define HTML_PAGE_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>CCJ Gateway</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, sans-serif; background-color: #F2F2F7; margin: 0; display: flex; flex-direction: column; align-items: center; height: 100vh; -webkit-tap-highlight-color: transparent; }
        .header { margin-top: 60px; text-align: center; }
        .header h1 { font-size: 28px; color: #1C1C1E; margin: 0; }
        .header p { font-size: 14px; color: #8E8E93; margin-top: 5px; }
        .card { background: #FFFFFF; border-radius: 24px; width: 85%; max-width: 350px; padding: 40px 20px; margin-top: 40px; box-shadow: 0 10px 30px rgba(0,0,0,0.06); display: flex; flex-direction: column; align-items: center; }
        .status-dot { width: 10px; height: 10px; border-radius: 50%; background-color: #34C759; margin-bottom: 25px; box-shadow: 0 0 10px rgba(52,199,89,0.4); }
        .title { margin-top: 0; margin-bottom: 30px; color: #1C1C1E; font-size: 22px; font-weight: 600; }
        .toggle-btn { background-color: #E5E5EA; color: #8E8E93; border: none; border-radius: 20px; padding: 20px 0; width: 100%; font-size: 18px; font-weight: bold; cursor: pointer; transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1); display: flex; justify-content: center; align-items: center; gap: 10px; }
        .toggle-btn:active { transform: scale(0.95); }
        .toggle-btn.active { background-color: #FF9500; color: #FFFFFF; box-shadow: 0 8px 20px rgba(255, 149, 0, 0.3); }
        .power-icon { width: 18px; height: 18px; border: 2px solid currentColor; border-radius: 50%; border-top-color: transparent; position: relative; transform: rotate(45deg); }
        .power-icon::before { content: ''; position: absolute; width: 2px; height: 10px; background-color: currentColor; top: -4px; left: 6px; transform: rotate(-45deg); }

        /* 电池 UI 样式 */
        .battery-container {
            position: absolute;
            top: 20px;
            right: 20px;
            font-size: 16px;
            font-weight: bold;
            color: #333;
            background: white;
            padding: 8px 12px;
            border-radius: 20px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            display: flex;
            align-items: center;
            gap: 5px;
        }
    </style>
</head>
<body>

    <div class="header">
        <h1>CCJ Gateway</h1>
        <p>局域网直连控制中心</p>
    </div>

    <div class="battery-container">
    🔋 <span id="bat-level">--</span>%
    </div>

    <div class="card">
        <div class="status-dot"></div>
        <h2 class="title">客厅主照明</h2>
        <button class="toggle-btn" id="lightBtn" onclick="toggleSwitch()">
            <div class="power-icon"></div>
            <span id="btnText">灯光已关闭</span>
        </button>
    </div>
    <script>
        let isLightOn = false;
        function toggleSwitch() {
            const btn = document.getElementById('lightBtn');
            const btnText = document.getElementById('btnText');
            const action = isLightOn ? 0 : 1;

            if (navigator.vibrate) navigator.vibrate(50);

            fetch('/control?action=' + action)
                .then(response => {
                    if (response.ok) {
                        isLightOn = !isLightOn;
                        if (isLightOn) {
                            btn.classList.add('active');
                            btnText.innerText = '灯光已开启';
                        } else {
                            btn.classList.remove('active');
                            btnText.innerText = '灯光已关闭';
                        }
                    } else alert("网关执行失败！");
                }).catch(err => alert("网络请求失败，请检查 WiFi 连接！"));
        }

        // 🚀 核心新增：监听 ESP32 推送的电池事件
        if (!!window.EventSource) {
        var source = new EventSource('/events');
        source.addEventListener('battery', function(e) {
            console.log("收到电量更新: ", e.data);
            document.getElementById('bat-level').innerText = e.data;
        }, false);
        }
    </script>
</body>
</html>
)rawliteral";

#endif