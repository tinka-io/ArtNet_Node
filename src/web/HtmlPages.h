#pragma once

// HTML for the home page
const char homePage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tinkas ArtNet Node</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 700px; margin: 50px auto; padding: 20px; background-color: #f0f0f0; }
        .container { background-color: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; margin-bottom: 10px; }
        .version { color: #666; font-size: 0.9em; margin: 0 0 30px 0; text-align: center; }
        .info-box { background-color: #e8f4f8; padding: 20px; border-radius: 10px; margin: 20px 0; border-left: 4px solid #0066cc; }
        .info-box h2 { margin-top: 0; color: #333; font-size: 1.2em; }
        .info-box p { margin: 8px 0; }
        .mode-section { background-color: #fff3cd; padding: 25px; border-radius: 10px; margin: 20px 0; border-left: 4px solid #ffc107; }
        .mode-section h2 { margin-top: 0; color: #333; font-size: 1.2em; }
        .mode-display { font-size: 1.3em; font-weight: bold; color: #0066cc; margin: 15px 0; }
        .toggle-switch { position: relative; display: inline-block; width: 60px; height: 34px; margin: 0 15px; vertical-align: middle; }
        .toggle-switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #0066cc; transition: .4s; border-radius: 34px; }
        .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
        input:checked + .slider { background-color: #28a745; }
        input:checked + .slider:before { transform: translateX(26px); }
        .mode-toggle-container { display: flex; align-items: center; justify-content: center; margin: 20px 0; font-size: 1.1em; }
        .btn { display: inline-block; background-color: #0066cc; color: white; padding: 15px 30px; margin: 10px; text-decoration: none; border-radius: 5px; text-align: center; }
        .btn:hover { background-color: #0052a3; }
        .btn-group { display: flex; justify-content: space-around; margin-top: 30px; flex-wrap: wrap; }
        .message { padding: 15px; margin-top: 15px; border-radius: 5px; display: none; text-align: center; }
        .success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Tinkas ArtNet Node</h1>
        <div class="version">Firmware: %FIRMWARE_VERSION%</div>

        <div class="mode-section">
            <h2>Operation Mode</h2>
            <div class="mode-display" id="currentMode">%MODE%</div>
            <div class="mode-toggle-container">
                <span>ArtNet Node</span>
                <label class="toggle-switch">
                    <input type="checkbox" id="modeToggle" %MODE_CHECKED%>
                    <span class="slider"></span>
                </label>
                <span>Control Panel</span>
            </div>
            <div class="message" id="message"></div>
        </div>

        <div class="info-box">
            <h2>Network Information</h2>
            <p><strong>Connection:</strong> %CONNECTION%</p>
            <p><strong>IP Mode:</strong> %IP_MODE%</p>
            <p><strong>IP Address:</strong> %IP%</p>
            <p><strong>Gateway:</strong> %GATEWAY%</p>
            <p><strong>Subnet:</strong> %SUBNET%</p>
        </div>

        <div class="info-box">
            <h2>ArtNet & DMX</h2>
            <p><strong>ArtNet Universe:</strong> %ARTNET_UNIVERSE%</p>
            <p><strong>DMX Refresh Rate:</strong> %DMX_REFRESH% ms (%DMX_HZ% Hz)</p>
        </div>

        <div class="btn-group">
            <a href="/settings" class="btn">ArtNet/DMX Settings</a>
            <a href="/logs" class="btn">System Logs</a>
            <a href="/config" class="btn">Network Config</a>
            <a href="/update" class="btn">OTA Update</a>
        </div>
    </div>

    <script>
        const toggle = document.getElementById('modeToggle');
        const modeDisplay = document.getElementById('currentMode');
        const message = document.getElementById('message');

        toggle.addEventListener('change', async () => {
            const newMode = toggle.checked;
            const modeName = newMode ? 'Control Panel' : 'ArtNet Node';

            message.style.display = 'none';

            try {
                const response = await fetch('/api/setMode', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ useControlPanel: newMode })
                });

                const result = await response.json();

                if (result.success) {
                    modeDisplay.textContent = modeName;
                    message.className = 'message success';
                    message.textContent = 'Mode changed to ' + modeName + ' successfully!';
                    message.style.display = 'block';

                    // Hide success message after 3 seconds
                    setTimeout(() => {
                        message.style.display = 'none';
                    }, 3000);
                } else {
                    toggle.checked = !newMode;
                    message.className = 'message error';
                    message.textContent = 'Error: ' + result.message;
                    message.style.display = 'block';
                }
            } catch (error) {
                toggle.checked = !newMode;
                message.className = 'message error';
                message.textContent = 'Error changing mode: ' + error.message;
                message.style.display = 'block';
            }
        });
    </script>
</body>
</html>
)rawliteral";

// HTML for the configuration page
const char configPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Network Configuration</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; background-color: #f0f0f0; }
        .container { background-color: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .info-box { background-color: #e8f4f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; border-left: 4px solid #0066cc; }
        .info-box p { margin: 5px 0; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; color: #555; }
        input[type="text"], select { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        .radio-group { margin: 15px 0; }
        .radio-group label { display: inline; font-weight: normal; margin-left: 5px; }
        input[type="radio"] { margin-right: 5px; }
        .static-fields { display: none; background-color: #f9f9f9; padding: 15px; border-radius: 5px; margin-top: 10px; }
        button { background-color: #0066cc; color: white; padding: 12px 30px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; width: 100%; margin-top: 10px; }
        button:hover { background-color: #0052a3; }
        .message { padding: 10px; margin-top: 15px; border-radius: 5px; display: none; }
        .success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .home-link { text-align: center; margin-top: 20px; }
        .home-link a { color: #0066cc; text-decoration: none; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Network Configuration</h1>
        <div class="info-box">
            <h3 style="margin-top:0;">Current Network Status</h3>
            <p><strong>Mode:</strong> <span id="currentMode">%MODE%</span></p>
            <p><strong>IP Address:</strong> <span id="currentIP">%IP%</span></p>
            <p><strong>Gateway:</strong> <span id="currentGateway">%GATEWAY%</span></p>
            <p><strong>Subnet:</strong> <span id="currentSubnet">%SUBNET%</span></p>
        </div>
        <form id="configForm">
            <div class="radio-group">
                <label style="display:block; font-weight:bold; margin-bottom:10px;">IP Configuration:</label>
                <div><input type="radio" id="dhcp" name="ipMode" value="dhcp" %DHCP_CHECKED%><label for="dhcp">DHCP (Automatic)</label></div>
                <div><input type="radio" id="static" name="ipMode" value="static" %STATIC_CHECKED%><label for="static">Static IP</label></div>
            </div>
            <div class="static-fields" id="staticFields">
                <div class="form-group"><label for="staticIP">IP Address:</label><input type="text" id="staticIP" name="staticIP" value="%STATIC_IP%" placeholder="192.168.1.100"></div>
                <div class="form-group"><label for="gateway">Gateway:</label><input type="text" id="gateway" name="gateway" value="%GATEWAY_IP%" placeholder="192.168.1.1"></div>
                <div class="form-group"><label for="subnet">Subnet Mask:</label><input type="text" id="subnet" name="subnet" value="%SUBNET_MASK%" placeholder="255.255.255.0"></div>
                <div class="form-group"><label for="dns1">Primary DNS:</label><input type="text" id="dns1" name="dns1" value="%DNS1%" placeholder="8.8.8.8"></div>
                <div class="form-group"><label for="dns2">Secondary DNS:</label><input type="text" id="dns2" name="dns2" value="%DNS2%" placeholder="8.8.4.4"></div>
            </div>
            <button type="submit">Save Configuration</button>
        </form>
        <div class="message" id="message"></div>
        <div class="home-link"><a href="/">Back to Home</a></div>
    </div>
    <script>
        function toggleStaticFields() {
            const staticRadio = document.getElementById('static');
            const staticFields = document.getElementById('staticFields');
            staticFields.style.display = staticRadio.checked ? 'block' : 'none';
        }
        document.querySelectorAll('input[name="ipMode"]').forEach(radio => { radio.addEventListener('change', toggleStaticFields); });
        toggleStaticFields();
        document.getElementById('configForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData(e.target);
            const data = {};
            for (let [key, value] of formData.entries()) { data[key] = value; }
            const message = document.getElementById('message');
            try {
                const response = await fetch('/saveConfig', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data) });
                const result = await response.json();
                message.className = 'message ' + (result.success ? 'success' : 'error');
                message.textContent = result.message;
                message.style.display = 'block';
                if (result.success) {
                    setTimeout(() => {
                        message.textContent = 'Restarting device... Please wait and reconnect.';
                        setTimeout(() => { location.reload(); }, 5000);
                    }, 2000);
                }
            } catch (error) {
                message.className = 'message error';
                message.textContent = 'Error saving configuration: ' + error.message;
                message.style.display = 'block';
            }
        });
    </script>
</body>
</html>
)rawliteral";

// HTML for the logs page
const char logsPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Logs</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }
        .container { background-color: white; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); max-width: 1200px; margin: 0 auto; }
        .header { background-color: #0066cc; color: white; padding: 20px; border-radius: 10px 10px 0 0; display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; }
        h1 { margin: 0; font-size: 1.5em; }
        .controls { display: flex; gap: 10px; flex-wrap: wrap; }
        .btn { background-color: white; color: #0066cc; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; font-size: 14px; text-decoration: none; display: inline-block; }
        .btn:hover { background-color: #f0f0f0; }
        .btn-clear { background-color: #dc3545; color: white; }
        .btn-clear:hover { background-color: #c82333; }
        .log-container { padding: 20px; height: 70vh; overflow-y: auto; background-color: #1e1e1e; color: #d4d4d4; font-family: 'Courier New', monospace; font-size: 13px; border-radius: 0 0 10px 10px; }
        .log-entry { padding: 4px 0; border-bottom: 1px solid #333; word-wrap: break-word; }
        .log-entry:hover { background-color: #2d2d2d; }
        .timestamp { color: #4ec9b0; font-weight: bold; }
        .status { padding: 10px 20px; background-color: #e8f4f8; font-size: 12px; color: #666; }
        .status.connected { background-color: #d4edda; color: #155724; }
        .auto-scroll-toggle { display: flex; align-items: center; gap: 5px; color: white; font-size: 14px; }
        .auto-scroll-toggle input { cursor: pointer; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>System Logs</h1>
            <div class="controls">
                <label class="auto-scroll-toggle">
                    <input type="checkbox" id="autoScroll" checked>
                    Auto-scroll
                </label>
                <button class="btn" onclick="loadLogs()">Refresh</button>
                <button class="btn btn-clear" onclick="clearLogs()">Clear</button>
                <a href="/" class="btn">Home</a>
            </div>
        </div>
        <div class="status" id="status">Loading logs...</div>
        <div class="log-container" id="logContainer">
            <div style="color: #888;">Waiting for logs...</div>
        </div>
    </div>

    <script>
        const logContainer = document.getElementById('logContainer');
        const statusDiv = document.getElementById('status');
        const autoScrollCheckbox = document.getElementById('autoScroll');
        let lastLogCount = 0;

        function scrollToBottom() {
            if (autoScrollCheckbox.checked) {
                logContainer.scrollTop = logContainer.scrollHeight;
            }
        }

        function displayLogs(logs) {
            if (logs.length === 0) {
                logContainer.innerHTML = '<div style="color: #888;">No logs available</div>';
                return;
            }

            let html = '';
            logs.forEach(log => {
                const match = log.match(/\[(.*?)\](.*)/);
                if (match) {
                    html += `<div class="log-entry"><span class="timestamp">${match[1]}</span>${escapeHtml(match[2])}</div>`;
                } else {
                    html += `<div class="log-entry">${escapeHtml(log)}</div>`;
                }
            });
            logContainer.innerHTML = html;
            scrollToBottom();
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }

        function loadLogs() {
            fetch('/api/logs')
                .then(response => response.json())
                .then(data => {
                    displayLogs(data.logs);
                    lastLogCount = data.logs.length;
                    statusDiv.textContent = 'Loaded ' + data.logs.length + ' log entries';
                    statusDiv.className = 'status';
                })
                .catch(error => {
                    statusDiv.textContent = 'Error loading logs: ' + error.message;
                    statusDiv.className = 'status';
                });
        }

        function clearLogs() {
            if (confirm('Clear all logs?')) {
                fetch('/api/logs/clear', { method: 'POST' })
                    .then(response => response.json())
                    .then(() => {
                        logContainer.innerHTML = '<div style="color: #888;">Logs cleared</div>';
                        lastLogCount = 0;
                        statusDiv.textContent = 'Logs cleared';
                    })
                    .catch(error => {
                        statusDiv.textContent = 'Error clearing logs: ' + error.message;
                    });
            }
        }

        function startPolling() {
            setInterval(() => {
                fetch('/api/logs')
                    .then(response => response.json())
                    .then(data => {
                        if (data.logs.length !== lastLogCount) {
                            displayLogs(data.logs);
                            lastLogCount = data.logs.length;
                            statusDiv.textContent = 'Live - ' + data.logs.length + ' entries (last update: ' + new Date().toLocaleTimeString() + ')';
                            statusDiv.className = 'status connected';
                        }
                    })
                    .catch(error => {
                        console.error('Polling error:', error);
                    });
            }, 1000);
        }

        loadLogs();
        startPolling();
    </script>
</body>
</html>
)rawliteral";

// HTML for the settings page
const char settingsPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ArtNet & DMX Settings</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; background-color: #f0f0f0; }
        .container { background-color: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .info-box { background-color: #e8f4f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; border-left: 4px solid #0066cc; }
        .info-box h3 { margin-top: 0; color: #333; }
        .info-box p { margin: 5px 0; }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; color: #555; }
        input[type="number"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; font-size: 16px; }
        .help-text { font-size: 12px; color: #666; margin-top: 5px; }
        .refresh-presets { display: flex; gap: 5px; margin-top: 5px; flex-wrap: wrap; }
        .preset-btn { padding: 5px 10px; background-color: #f0f0f0; border: 1px solid #ddd; border-radius: 3px; cursor: pointer; font-size: 12px; }
        .preset-btn:hover { background-color: #e0e0e0; }
        button { background-color: #0066cc; color: white; padding: 12px 30px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; width: 100%; margin-top: 10px; }
        button:hover { background-color: #0052a3; }
        .message { padding: 10px; margin-top: 15px; border-radius: 5px; display: none; }
        .success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .btn-group { display: flex; gap: 10px; margin-top: 20px; }
        .btn-secondary { background-color: #6c757d; }
        .btn-secondary:hover { background-color: #5a6268; }
        .home-link { text-align: center; margin-top: 20px; }
        .home-link a { color: #0066cc; text-decoration: none; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ArtNet & DMX Settings</h1>

        <div class="info-box">
            <h3>Current Settings</h3>
            <p><strong>Mode:</strong> <span id="currentMode">%MODE%</span></p>
            <p><strong>ArtNet Universe:</strong> <span id="currentUniverse">%ARTNET_UNIVERSE%</span></p>
            <p><strong>DMX Refresh Rate:</strong> <span id="currentRefresh">%DMX_REFRESH%</span> ms (<span id="currentHz">%DMX_HZ%</span> Hz)</p>
        </div>

        <form id="settingsForm">
            <div class="form-group">
                <label for="artnetUniverse">ArtNet Universe (0-255)</label>
                <input type="number" id="artnetUniverse" name="artnetUniverse" min="0" max="255" value="%ARTNET_UNIVERSE%" required>
                <div class="help-text">Select which ArtNet universe to listen to for DMX data</div>
            </div>

            <div class="form-group">
                <label for="dmxRefresh">DMX Refresh Rate (ms)</label>
                <input type="number" id="dmxRefresh" name="dmxRefresh" min="10" max="100" value="%DMX_REFRESH%" required>
                <div class="help-text">
                    How often to send DMX data (10-100 ms). Lower = faster updates, higher CPU usage.
                    <div id="refreshHz" style="margin-top: 5px; font-weight: bold;"></div>
                </div>
                <div class="refresh-presets">
                    <button type="button" class="preset-btn" onclick="setRefresh(25)">40 Hz (25ms)</button>
                    <button type="button" class="preset-btn" onclick="setRefresh(33)">30 Hz (33ms)</button>
                    <button type="button" class="preset-btn" onclick="setRefresh(40)">25 Hz (40ms)</button>
                    <button type="button" class="preset-btn" onclick="setRefresh(50)">20 Hz (50ms)</button>
                </div>
            </div>

            <div class="btn-group">
                <button type="submit">Save Settings</button>
            </div>
        </form>

        <div class="message" id="message"></div>
        <div class="home-link"><a href="/">Back to Home</a></div>
    </div>

    <script>
        const universeInput = document.getElementById('artnetUniverse');
        const refreshInput = document.getElementById('dmxRefresh');
        const refreshHzDiv = document.getElementById('refreshHz');
        const message = document.getElementById('message');

        function updateRefreshRate() {
            const ms = parseInt(refreshInput.value) || 33;
            const hz = (1000 / ms).toFixed(1);
            refreshHzDiv.textContent = 'Approximately ' + hz + ' Hz';
        }

        function setRefresh(ms) {
            refreshInput.value = ms;
            updateRefreshRate();
        }

        refreshInput.addEventListener('input', updateRefreshRate);
        updateRefreshRate();

        document.getElementById('settingsForm').addEventListener('submit', async (e) => {
            e.preventDefault();

            const data = {
                artnetUniverse: parseInt(universeInput.value),
                dmxRefreshRate: parseInt(refreshInput.value)
            };

            message.style.display = 'none';

            try {
                const response = await fetch('/api/settings', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(data)
                });

                const result = await response.json();

                if (result.success) {
                    message.className = 'message success';
                    message.textContent = 'Settings saved successfully! Changes will take effect immediately.';
                    message.style.display = 'block';

                    // Update current settings display
                    document.getElementById('currentUniverse').textContent = data.artnetUniverse;
                    document.getElementById('currentRefresh').textContent = data.dmxRefreshRate;
                    document.getElementById('currentHz').textContent = (1000 / data.dmxRefreshRate).toFixed(1);

                    // Hide message after 3 seconds
                    setTimeout(() => {
                        message.style.display = 'none';
                    }, 3000);
                } else {
                    message.className = 'message error';
                    message.textContent = 'Error: ' + result.message;
                    message.style.display = 'block';
                }
            } catch (error) {
                message.className = 'message error';
                message.textContent = 'Error saving settings: ' + error.message;
                message.style.display = 'block';
            }
        });
    </script>
</body>
</html>
)rawliteral";
