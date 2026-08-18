#include "network_manager.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ctype.h>
#include <esp_wifi.h>

#include "config.h"
#include "osc_manager.h"

namespace {

enum class NetworkState : uint8_t {
  CONNECTING,
  CONNECTED,
  AP_MODE,
};

WebServer server(80);
DNSServer dnsServer;
NetworkState networkState = NetworkState::CONNECTING;
unsigned long restartAtMs = 0;
bool restartScheduled = false;
bool mdnsRunning = false;
bool routesRegistered = false;
bool webServerStarted = false;
String savedSsid;
String savedPassword;

const char PAGE_STYLE[] PROGMEM = R"CSS(
body{font-family:system-ui,sans-serif;margin:0;background:#f3f5f8;color:#172033}
main{max-width:680px;margin:32px auto;padding:0 16px}
.card{background:#fff;border-radius:14px;padding:22px;box-shadow:0 4px 18px #0001}
h1{font-size:1.6rem;margin-top:0}label{display:block;font-weight:650;margin-top:14px}
input{box-sizing:border-box;width:100%;padding:11px;border:1px solid #aeb7c5;border-radius:8px;font-size:1rem}
button{width:100%;margin-top:18px;padding:12px;border:0;border-radius:8px;background:#2962e2;color:#fff;font-size:1rem;font-weight:650}
.danger{background:#c9343a}.note{color:#586274;line-height:1.55}.status{padding:10px 12px;background:#edf3ff;border-radius:8px}
code{overflow-wrap:anywhere}
)CSS";

String pageStart(const char* title) {
  String html;
  html.reserve(2200);
  html += F("<!doctype html><html lang='en'><head><meta charset='utf-8'>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>");
  html += title;
  html += F("</title><style>");
  html += FPSTR(PAGE_STYLE);
  html += F("</style></head><body><main><div class='card'>");
  return html;
}

void sendPage(String html) {
  html += F("</div></main></body></html>");
  server.send(200, "text/html; charset=utf-8", html);
}

String htmlEscape(const String& value) {
  String escaped;
  escaped.reserve(value.length());
  for (size_t index = 0; index < value.length(); ++index) {
    switch (value[index]) {
      case '&': escaped += F("&amp;"); break;
      case '<': escaped += F("&lt;"); break;
      case '>': escaped += F("&gt;"); break;
      case '"': escaped += F("&quot;"); break;
      case '\'': escaped += F("&#39;"); break;
      default: escaped += value[index]; break;
    }
  }
  return escaped;
}

void sendProvisioningPage(const String& message = String()) {
  String html = pageStart("ChainOSCmini Wi-Fi Setup");
  html += F("<h1>ChainOSCmini Wi-Fi Setup</h1>");
  html += F("<p class='note'>Enter the Wi-Fi network used by your OSC device. ");
  html += F("Wi-Fiは2.4 GHz帯のネットワークを指定してください。</p>");
  if (message.length() > 0) {
    html += F("<p class='status'>");
    html += message;
    html += F("</p>");
  }
  html += F("<form method='post' action='/save-wifi'>");
  html += F("<label for='ssid'>Wi-Fi SSID</label>");
  html += F("<input id='ssid' name='ssid' maxlength='32' required autocomplete='off'>");
  html += F("<label for='password'>Wi-Fi Password</label>");
  html += F("<input id='password' name='password' type='password' maxlength='64' autocomplete='off'>");
  html += F("<p class='note'>Use 8–63 characters, or a 64-digit hexadecimal PSK. Leave blank only for an open network.</p>");
  html += F("<button type='submit'>Save Wi-Fi and Restart</button></form>");
  sendPage(html);
}

void sendStatusPage(const String& message = String()) {
  String html = pageStart("ChainOSCmini");
  html += F("<h1>ChainOSCmini</h1><p class='status'>Wi-Fi connected / Wi-Fi接続済み</p>");
  if (!message.isEmpty()) {
    html += F("<p class='status'>");
    html += htmlEscape(message);
    html += F("</p>");
  }
  html += F("<p><strong>Version:</strong> ");
  html += APP_VERSION;
  html += F("</p><p><strong>IP address:</strong> <code>");
  html += WiFi.localIP().toString();
  html += F("</code></p><p><strong>mDNS:</strong> <code>http://");
  html += WIFI_MDNS_HOST;
  html += F(".local/</code></p>");
  html += F("<hr><h2>OSC Target / OSC送信先</h2>");
  html += F("<form method='post' action='/save-osc'>");
  html += F("<label for='osc_host'>Host or IP address / ホスト名またはIPアドレス</label>");
  html += F("<input id='osc_host' name='osc_host' maxlength='253' required value='");
  html += htmlEscape(oscTargetHost());
  html += F("'>");
  html += F("<label for='osc_port'>UDP Port / UDPポート</label>");
  html += F("<input id='osc_port' name='osc_port' type='number' min='1' max='65535' required value='");
  html += oscTargetPort();
  html += F("'><button type='submit'>Save OSC Target / OSC送信先を保存</button></form>");
  html += F("<p class='note'>DualKey: <code>/chainoscmini/dualkey/key1</code>, <code>/chainoscmini/dualkey/key2</code><br>Chain Key: <code>/chainoscmini/chain/key/&lt;UID&gt;</code><br>Pressed = 1, Released = 0</p>");
  html += F("<form method='post' action='/forget-wifi' onsubmit=\"return confirm('Delete saved Wi-Fi settings?')\">");
  html += F("<button class='danger' type='submit'>Forget Wi-Fi Settings</button></form>");
  sendPage(html);
}

void handleSaveOsc() {
  String host = server.arg("osc_host");
  String portText = server.arg("osc_port");
  host.trim();
  portText.trim();
  bool numericPort = !portText.isEmpty();
  for (size_t index = 0; numericPort && index < portText.length(); ++index) {
    numericPort = isdigit(static_cast<unsigned char>(portText[index]));
  }
  const unsigned long portValue = numericPort ? portText.toInt() : 0;
  if (host.isEmpty() || host.length() > 253 || portValue < 1 ||
      portValue > 65535 ||
      !oscSaveTarget(host, static_cast<uint16_t>(portValue))) {
    sendStatusPage("Could not save OSC target. Check Host and Port. / OSC送信先を保存できませんでした。");
    return;
  }
  sendStatusPage("OSC target saved. / OSC送信先を保存しました。");
}

void handleRoot() {
  if (networkState == NetworkState::AP_MODE) {
    sendProvisioningPage();
  } else {
    sendStatusPage();
  }
}

void scheduleRestart() {
  restartScheduled = true;
  restartAtMs = millis() + NETWORK_RESTART_DELAY_MS;
}

bool validWifiInput(const String& ssid, const String& password,
                    String& error) {
  const size_t ssidBytes = ssid.length();
  const size_t passwordBytes = password.length();
  if (ssidBytes == 0 || ssidBytes > 32) {
    error = "SSID must be 1–32 bytes. / SSIDは1～32バイトで入力してください。";
    return false;
  }
  bool valid64DigitPsk = passwordBytes == 64;
  for (size_t index = 0; valid64DigitPsk && index < passwordBytes; ++index) {
    valid64DigitPsk = isxdigit(static_cast<unsigned char>(password[index]));
  }
  if (passwordBytes != 0 &&
      (passwordBytes < 8 || (passwordBytes > 63 && !valid64DigitPsk))) {
    error = "Password must be blank, 8–63 bytes, or a 64-digit hexadecimal PSK. / パスワードは空欄、8～63バイト、または64桁の16進数PSKで入力してください。";
    return false;
  }
  return true;
}

void handleSaveWifi() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String error;
  if (!validWifiInput(ssid, password, error)) {
    sendProvisioningPage(error);
    return;
  }

  Preferences preferences;
  if (!preferences.begin(WIFI_PREFS_NAMESPACE, false)) {
    sendProvisioningPage("Could not open settings storage. / 設定ストレージを開けませんでした。");
    return;
  }
  const size_t ssidWritten = preferences.putString("ssid", ssid);
  const size_t passwordWritten = preferences.putString("password", password);
  preferences.end();
  if (ssidWritten == 0 || (password.length() > 0 && passwordWritten == 0)) {
    sendProvisioningPage("Could not save Wi-Fi settings. / Wi-Fi設定を保存できませんでした。");
    return;
  }

  Serial.printf("[ChainOSCmini][NET] credentials_saved ssid_bytes=%u password_bytes=%u\n",
                static_cast<unsigned int>(ssid.length()),
                static_cast<unsigned int>(password.length()));
  String html = pageStart("Wi-Fi Saved");
  html += F("<h1>Wi-Fi settings saved</h1>");
  html += F("<p class='status'>Restarting ChainOSCmini… / ChainOSCminiを再起動します…</p>");
  sendPage(html);
  scheduleRestart();
}

void handleForgetWifi() {
  Preferences preferences;
  bool cleared = false;
  if (preferences.begin(WIFI_PREFS_NAMESPACE, false)) {
    const bool ssidRemoved =
        !preferences.isKey("ssid") || preferences.remove("ssid");
    const bool passwordRemoved =
        !preferences.isKey("password") || preferences.remove("password");
    cleared = ssidRemoved && passwordRemoved;
    preferences.end();
  }
  Serial.printf("[ChainOSCmini][NET] credentials_cleared=%s\n",
                cleared ? "true" : "false");
  String html = pageStart("Wi-Fi Settings Deleted");
  html += F("<h1>Wi-Fi settings deleted</h1>");
  html += F("<p class='status'>Restarting in setup mode… / 設定モードで再起動します…</p>");
  sendPage(html);
  scheduleRestart();
}

void registerRoutes() {
  if (routesRegistered) {
    return;
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save-wifi", HTTP_POST, handleSaveWifi);
  server.on("/forget-wifi", HTTP_POST, handleForgetWifi);
  server.on("/save-osc", HTTP_POST, handleSaveOsc);
  server.on("/generate_204", HTTP_ANY, handleRoot);
  server.on("/hotspot-detect.html", HTTP_ANY, handleRoot);
  server.on("/ncsi.txt", HTTP_ANY, handleRoot);
  server.on("/connecttest.txt", HTTP_ANY, handleRoot);
  server.on("/fwlink", HTTP_ANY, handleRoot);
  server.on("/redirect", HTTP_ANY, handleRoot);
  server.on("/canonical.html", HTTP_ANY, handleRoot);
  server.on("/success.txt", HTTP_ANY, handleRoot);
  server.onNotFound(handleRoot);
  routesRegistered = true;
}

void startAccessPoint(const char* reason) {
  if (mdnsRunning) {
    MDNS.end();
    mdnsRunning = false;
  }
  WiFi.mode(WIFI_AP);
  const IPAddress apIp(192, 168, 4, 1);
  WiFi.softAPConfig(apIp, apIp, IPAddress(255, 255, 255, 0));
  const bool started = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  const esp_err_t txPowerResult =
      esp_wifi_set_max_tx_power(WIFI_TX_POWER_QDBM);
  delay(500);
  dnsServer.start(CAPTIVE_DNS_PORT, "*", apIp);
  registerRoutes();
  if (!webServerStarted) {
    server.begin();
    webServerStarted = true;
  }
  networkState = NetworkState::AP_MODE;
  Serial.printf(
      "[ChainOSCmini][NET] state=AP_MODE reason=%s started=%s ssid=%s "
      "ip=%s tx_power_qdbm=%d tx_power_result=%d\n",
      reason, started ? "true" : "false", WIFI_AP_SSID,
      apIp.toString().c_str(), static_cast<int>(WIFI_TX_POWER_QDBM),
      static_cast<int>(txPowerResult));
}

void startStationConnection() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
  const esp_err_t txPowerResult =
      esp_wifi_set_max_tx_power(WIFI_TX_POWER_QDBM);
  networkState = NetworkState::CONNECTING;
  Serial.printf("[ChainOSCmini][NET] state=CONNECTING ssid_bytes=%u timeout_ms=%lu tx_power_qdbm=%d tx_power_result=%d\n",
                static_cast<unsigned int>(savedSsid.length()),
                WIFI_CONNECT_TIMEOUT_MS,
                static_cast<int>(WIFI_TX_POWER_QDBM),
                static_cast<int>(txPowerResult));
}

void handleConnected() {
  networkState = NetworkState::CONNECTED;
  const bool mdnsStarted = MDNS.begin(WIFI_MDNS_HOST);
  mdnsRunning = mdnsStarted;
  if (mdnsStarted) {
    MDNS.addService("http", "tcp", 80);
  }
  registerRoutes();
  if (!webServerStarted) {
    server.begin();
    webServerStarted = true;
  }
  Serial.printf(
      "[ChainOSCmini][NET] state=CONNECTED ip=%s mdns=%s.local "
      "mdns_started=%s rssi=%d\n",
      WiFi.localIP().toString().c_str(), WIFI_MDNS_HOST,
      mdnsStarted ? "true" : "false", WiFi.RSSI());
}

}  // namespace

void networkSetup() {
  Serial.println("[ChainOSCmini][NET] setup_begin=true");

  Preferences preferences;
  if (preferences.begin(WIFI_PREFS_NAMESPACE, true)) {
    savedSsid = preferences.getString("ssid", "");
    savedPassword = preferences.getString("password", "");
    preferences.end();
  }

  if (savedSsid.length() == 0) {
    startAccessPoint("no_saved_credentials");
  } else {
    startStationConnection();
    const unsigned long startedAtMs = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startedAtMs < WIFI_CONNECT_TIMEOUT_MS) {
      delay(300);
    }
    if (WiFi.status() == WL_CONNECTED) {
      handleConnected();
    } else {
      startAccessPoint("connect_timeout");
    }
  }

  Serial.printf("[ChainOSCmini][NET] web_server_started=%s\n",
                webServerStarted ? "true" : "false");
}

void networkUpdate() {
  if (webServerStarted) {
    server.handleClient();
  }

  if (networkState == NetworkState::AP_MODE) {
    dnsServer.processNextRequest();
  } else if (networkState == NetworkState::CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) handleConnected();
  } else if (networkState == NetworkState::CONNECTED &&
             WiFi.status() != WL_CONNECTED) {
    if (mdnsRunning) {
      MDNS.end();
      mdnsRunning = false;
    }
    WiFi.reconnect();
    networkState = NetworkState::CONNECTING;
    Serial.println("[ChainOSCmini][NET] state=RECONNECTING");
  }

  if (restartScheduled &&
      static_cast<long>(millis() - restartAtMs) >= 0) {
    Serial.println("[ChainOSCmini][NET] restarting=true");
    delay(20);
    ESP.restart();
  }
}
