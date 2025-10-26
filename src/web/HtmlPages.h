#pragma once

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
