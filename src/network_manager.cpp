#include "network_manager.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ctype.h>
#include <esp_wifi.h>
#include <math.h>

#include "config.h"
#include "key_settings.h"
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
enum class UiLanguage : uint8_t { ENGLISH = 0, JAPANESE = 1 };
UiLanguage uiLanguage = UiLanguage::ENGLISH;
bool uiLanguageConfigured = false;

bool isJapaneseUi() { return uiLanguage == UiLanguage::JAPANESE; }
const char* tr(const char* english, const char* japanese) {
  return isJapaneseUi() ? japanese : english;
}

void saveUiLanguage() {
  Preferences preferences;
  if (preferences.begin("ui", false)) {
    preferences.putUChar("language", static_cast<uint8_t>(uiLanguage));
    preferences.end();
    uiLanguageConfigured = true;
  }
}

void applyBrowserLanguageOnFirstVisit() {
  if (uiLanguageConfigured) return;
  String accepted = server.header("Accept-Language");
  accepted.toLowerCase();
  if (!accepted.isEmpty()) {
    uiLanguage = accepted.startsWith("ja") ? UiLanguage::JAPANESE
                                            : UiLanguage::ENGLISH;
    saveUiLanguage();
  }
}

const char PAGE_STYLE[] PROGMEM = R"CSS(
body{font-family:sans-serif;margin:16px;background:#f5f5f5;color:#18212f}
main{max-width:1100px;margin:0 auto}
.card{background:#fff;padding:16px;border-radius:10px;margin-bottom:16px;box-shadow:0 2px 5px rgba(0,0,0,.1)}
h1{font-size:1.4em;margin:0 0 16px}h2{margin:0;font-size:1.1em}
label{display:block;margin-top:10px;font-weight:bold;font-size:.9em}
input,select{width:100%;padding:8px;margin-top:4px;box-sizing:border-box;border:1px solid #9aa3ad;border-radius:2px;font-size:1em}
input.invalid,select.invalid{border:2px solid #c73c4a;background:#fff8f8}.osc-row small,.address-field small{display:flex;justify-content:space-between;min-height:17px;color:#697586}.osc-row .err,.address-field .err{color:#c73c4a}
button{width:100%;padding:12px;background:#28a745;color:#fff;border:none;border-radius:6px;font-size:16px;margin-top:12px;cursor:pointer}
.primary{background:#3267e3}.danger{background:#dc3545}.note{color:#888;font-size:.9em;line-height:1.5}.meta{color:#666;font-size:.85em}
.status{padding:10px 12px;background:#edf3ff;color:#244da7;border:1px solid #cddbf8;border-radius:8px}
.system-grid,.key-grid{display:grid;grid-template-columns:1fr 1fr;gap:14px;align-items:start}.system-grid label,.key-grid label{margin-top:0}
.system-item{padding:10px;background:#f8f9fa;border-radius:6px}.system-item strong{display:block;margin-bottom:5px;font-size:.9em}.system-item code{word-break:break-all}
.section-title{margin:24px 2px 10px}.saved-settings{margin-top:28px}
.device{position:relative;border-left:5px solid #6f42c1}.device-head{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:10px}.device-head h2{display:flex;align-items:center;gap:4px;flex-wrap:wrap}
.collapse-button{width:30px;height:30px;margin:0;padding:0;background:#f1f4f8;color:#42516a;border:1px solid #dce2ea;border-radius:7px;font-size:16px;line-height:1;transition:transform .15s}.collapse-button.collapsed{transform:rotate(-90deg)}.device-body[hidden]{display:none}
.uid{font-family:monospace;background:#eee;padding:6px 10px;border-radius:4px;word-break:break-all;font-size:.85em;margin-bottom:10px}
.badge{display:inline-block;padding:2px 8px;border-radius:10px;font-size:.75em;margin-right:4px}.badge-on{background:#d4edda;color:#155724}.badge-off{background:#f8d7da;color:#721c24}.badge-type{background:#e7e7ff;color:#333}
.mode-box{background:#f8f9fa;padding:10px;border-radius:6px;margin-top:10px}.press{border-left:5px solid #dc3545;padding-left:10px}.release{border-left:5px solid #007bff;padding-left:10px}
.usage{display:flex;justify-content:space-between;align-items:center;margin:14px 0;padding:11px 13px;border:1px solid #cddbf8;border-radius:9px;background:#edf3ff;color:#244da7}
.event-tabs{display:flex;gap:4px;padding:4px;background:#edf0f4;border-radius:9px}.event-tab{margin:0;background:transparent;color:#697586}.event-tab.active{background:#fff;color:#18212f;box-shadow:0 1px 4px #bbb}
.event-panel{margin-top:12px}.osc-list{display:grid;gap:10px}.osc-row{display:grid;grid-template-columns:62px minmax(180px,1fr) 115px minmax(100px,.55fr) 68px;gap:9px;align-items:start;padding:12px;border:1px solid #dce2ea;border-radius:10px;background:#fbfcfe}.osc-row label{margin-top:0}.order{display:flex;gap:3px;align-self:center}.mv{width:auto;margin:0;padding:7px;background:#fff;color:#526075;border:1px solid #dce2ea}.remove-msg{width:auto;margin-top:22px;padding:9px;background:#fff3f4;color:#c73c4a;border:1px solid #efc6cb}.add-msg{background:#f7faff;color:#3267e3;border:1px dashed #9db6ef}.add-msg:disabled{background:#eee;color:#888}.empty{display:none;padding:18px;text-align:center;color:#697586;border:1px dashed #dce2ea;border-radius:9px}.osc-list:empty+.empty{display:block}
.sequence-card{margin-top:12px;padding:15px;border:1px solid #dce2ea;border-radius:10px;background:#fbfcfe}.sequence-card h3{margin-top:0}.seq-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:10px}.seq-address{grid-column:1/-1}
.save-bar{position:sticky;z-index:15;bottom:8px;display:flex;align-items:center;gap:12px;padding:10px 12px;margin:16px 0 28px;background:rgba(255,255,255,.96);border:1px solid #dce2ea;border-radius:10px;box-shadow:0 5px 18px rgba(0,0,0,.14)}.save-bar button{flex:1;margin:0;background:#28a745}.dirty-status{color:#b45f06;font-weight:bold;white-space:nowrap}.saved-device-card h2{display:flex;align-items:center;gap:4px;flex-wrap:wrap}.btn-warning{background:#ff9800}.toast{position:fixed;z-index:30;left:50%;bottom:78px;transform:translateX(-50%);padding:11px 18px;border-radius:8px;background:#17324d;color:#fff;box-shadow:0 4px 16px rgba(0,0,0,.25)}.wifi-actions{margin-top:28px}.wifi-actions form{margin:0}
.language-row{display:flex;align-items:center;justify-content:space-between;gap:12px}.language-row h2{margin:0}.language-row form{margin:0;min-width:150px}.language-row select{margin:0}
@media(max-width:720px){.system-grid,.key-grid,.seq-grid{grid-template-columns:1fr}.seq-address{grid-column:1}.osc-row{grid-template-columns:52px 1fr}.osc-row .field,.remove-msg{grid-column:2}}
)CSS";

const char PAGE_SCRIPT[] PROGMEM = R"JS(
const JA=__JA__;const tx=(en,ja)=>JA?ja:en;const MAX_MSG=8;
const enc=new TextEncoder();function bytes(value){return enc.encode(value).length}function limitBytes(input,max){while(bytes(input.value)>max)input.value=input.value.slice(0,-1)}
function validateInput(input){const address=input.classList.contains('msg-address')||input.classList.contains('osc-address'),max=address?192:128,b=bytes(input.value);let error='';if(address){if(!input.value)error=tx('Required','必須です');else if(input.value[0]!=='/')error=tx('Start with /','「/」から始めてください');else if(/[\s#*,?\[\]{}]/.test(input.value))error=tx('Invalid character','使用できない文字があります')}else{const row=input.closest('.osc-row'),type=row?row.querySelector('.type').value:'2';if(type==='0'&&(!input.value.trim()||!Number.isFinite(Number(input.value))))error=tx('Invalid float','Float値が正しくありません');if(type==='1'&&!/^[+-]?\d+$/.test(input.value.trim()))error=tx('Invalid integer','Int値が正しくありません')}if(b>max)error=tx('Too long','長すぎます');input.classList.toggle('invalid',!!error);const small=input.parentNode.querySelector('small');if(small){small.querySelector('.err').textContent=error;small.querySelector('.bytes').textContent=b+' / '+max+' bytes'}return !error}
function limitAndValidate(input,max){limitBytes(input,max);validateInput(input)}function validateSettingsForm(form){let valid=true;form.querySelectorAll('.msg-address,.msg-value,.osc-address').forEach(input=>{if(!validateInput(input))valid=false});if(!valid){const bad=form.querySelector('.invalid');if(bad)bad.focus();alert(tx('Please correct the highlighted OSC fields.','赤く表示されたOSC設定項目を修正してください。'))}return valid}
function markDirty(){const status=document.getElementById('dirty-status');if(status)status.hidden=false}
function showToast(message){const toast=document.getElementById('save-toast');toast.textContent=message;toast.hidden=false;clearTimeout(window.toastTimer);window.toastTimer=setTimeout(()=>toast.hidden=true,3000)}
async function saveSettings(event){const form=event.currentTarget;if(!validateSettingsForm(form))return;const button=form.querySelector('.save-bar button');button.disabled=true;try{const response=await fetch('/save-all?ajax=1',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(new FormData(form))});const message=await response.text();if(!response.ok)throw new Error(message);document.getElementById('dirty-status').hidden=true;showToast(message)}catch(error){alert(error.message||tx('Could not save settings.','設定を保存できませんでした。'))}finally{button.disabled=false}}
async function deleteSavedDevice(event,form){event.preventDefault();if(!confirm(tx('Delete settings for this device?','このデバイスの設定を削除しますか？')))return;try{const response=await fetch('/delete_device?ajax=1',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(new FormData(form))});const message=await response.text();if(!response.ok)throw new Error(message);form.closest('.saved-device-card').remove();showToast(message)}catch(error){alert(error.message||tx('Could not delete device settings.','デバイス設定を削除できませんでした。'))}}
function toggleDevice(index,key){const body=document.getElementById('device-body-'+index);const button=document.getElementById('collapse-'+index);const collapsed=!body.hidden;body.hidden=collapsed;button.classList.toggle('collapsed',collapsed);button.setAttribute('aria-expanded',collapsed?'false':'true');sessionStorage.setItem('chainoscmini-collapse-'+key,collapsed?'1':'0')}
function toggleKeyMode(prId,sqId,select){document.getElementById(prId).style.display=select.value==='1'?'none':'block';document.getElementById(sqId).style.display=select.value==='1'?'block':'none'}
function showEvent(group,event,button){document.querySelectorAll('.event-panel[data-group="'+group+'"]').forEach(panel=>panel.style.display=panel.dataset.event===event?'block':'none');button.parentNode.querySelectorAll('.event-tab').forEach(tab=>tab.classList.remove('active'));button.classList.add('active')}
function rows(group){return document.querySelectorAll('.osc-row[data-group="'+group+'"]')}
function renumber(group){const all=rows(group);['press','release'].forEach(event=>{document.querySelectorAll('.osc-row[data-group="'+group+'"][data-event="'+event+'"]').forEach((row,index)=>{const prefix=event==='press'?'p':'r';row.querySelector('.msg-address').name=prefix+'_address_'+group+'_'+index;row.querySelector('.type').name=prefix+'_type_'+group+'_'+index;row.querySelector('.msg-value').name=prefix+'_value_'+group+'_'+index})});document.getElementById('count-'+group).textContent=all.length;document.getElementById('p-count-'+group).value=document.querySelectorAll('.osc-row[data-group="'+group+'"][data-event="press"]').length;document.getElementById('r-count-'+group).value=document.querySelectorAll('.osc-row[data-group="'+group+'"][data-event="release"]').length;document.querySelectorAll('.add-msg[data-group="'+group+'"]').forEach(button=>button.disabled=all.length>=MAX_MSG)}
function moveMsg(button,direction){const row=button.closest('.osc-row'),sibling=direction<0?row.previousElementSibling:row.nextElementSibling;if(!sibling)return;direction<0?row.parentNode.insertBefore(row,sibling):row.parentNode.insertBefore(sibling,row);renumber(row.dataset.group);markDirty()}
function removeMsg(button){const row=button.closest('.osc-row'),group=row.dataset.group;row.remove();renumber(group);markDirty()}
function addMsg(button){const group=button.dataset.group,event=button.dataset.event;if(rows(group).length>=MAX_MSG)return;const list=document.getElementById('list-'+event+'-'+group),row=document.createElement('div');row.className='osc-row';row.dataset.group=group;row.dataset.event=event;row.innerHTML='<div class="order"><button type="button" class="mv" onclick="moveMsg(this,-1)">&uarr;</button><button type="button" class="mv" onclick="moveMsg(this,1)">&darr;</button></div><div class="field"><label>'+tx('OSC Address','OSCアドレス')+'</label><input class="msg-address" maxlength="192" required oninput="limitAndValidate(this,192)"><small><span class="err"></span><span class="bytes"></span></small></div><div class="field"><label>'+tx('Type','型')+'</label><select class="type" onchange="validateInput(this.closest(\'.osc-row\').querySelector(\'.msg-value\'))"><option value="0" selected>Float</option><option value="1">Int</option><option value="2">String</option></select><small></small></div><div class="field"><label>'+tx('Value','値')+'</label><input class="msg-value" maxlength="128" value="1.0" oninput="limitAndValidate(this,128)"><small><span class="err"></span><span class="bytes"></span></small></div><button type="button" class="remove-msg" onclick="removeMsg(this)">'+tx('Delete','削除')+'</button>';list.appendChild(row);renumber(group);markDirty();row.querySelector('.msg-address').focus()}
document.addEventListener('DOMContentLoaded',()=>{const groups=new Set;document.querySelectorAll('.add-msg[data-group]').forEach(button=>groups.add(button.dataset.group));groups.forEach(renumber);document.querySelectorAll('.msg-address,.msg-value,.osc-address').forEach(validateInput);const form=document.getElementById('settings-form');if(form){form.addEventListener('input',markDirty);form.addEventListener('change',markDirty)}document.querySelectorAll('.device[data-collapse-key]').forEach(card=>{const key=card.dataset.collapseKey,index=card.dataset.deviceIndex;if(sessionStorage.getItem('chainoscmini-collapse-'+key)==='1'){const body=document.getElementById('device-body-'+index),button=document.getElementById('collapse-'+index);body.hidden=true;button.classList.add('collapsed');button.setAttribute('aria-expanded','false')}})})
)JS";

String pageStart(const char* title) {
  String html;
  html.reserve(2200);
  html += F("<!doctype html><html lang='");
  html += isJapaneseUi() ? F("ja") : F("en");
  html += F("'><head><meta charset='utf-8'>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>");
  html += title;
  html += F("</title><style>");
  html += FPSTR(PAGE_STYLE);
  html += F("</style><script>");
  html += FPSTR(PAGE_SCRIPT);
  html += F("</script></head><body><main>");
  html.replace("__JA__", isJapaneseUi() ? "true" : "false");
  return html;
}

void sendPage(String html) {
  html += F("</main></body></html>");
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
  html += F("<h1>Chain OSC Setting</h1><div class='card language-row'><h2>");
  html += tr("Language", "言語");
  html += F("</h2><form action='/set_language' method='post'><select name='language' onchange='this.form.submit()'><option value='en'");
  if (!isJapaneseUi()) html += F(" selected");
  html += F(">English</option><option value='ja'");
  if (isJapaneseUi()) html += F(" selected");
  html += F(">日本語</option></select></form></div><div class='card'><h2>");
  html += tr("ChainOSCmini Wi-Fi Setup", "ChainOSCmini Wi-Fi設定");
  html += F("</h2><p class='note'>");
  html += tr("Enter the Wi-Fi network used by your OSC device. Select a 2.4 GHz network.",
             "OSC送信先と同じWi-Fiを入力してください。2.4 GHz帯のネットワークを指定してください。");
  html += F("</p>");
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
  html += F("<p class='note'>");
  html += tr("Use 8–63 characters, or a 64-digit hexadecimal PSK. Leave blank only for an open network.",
             "8～63文字、または64桁の16進数PSKを入力してください。オープンネットワークの場合のみ空欄にします。");
  html += F("</p><button type='submit'>");
  html += tr("Save Wi-Fi and Restart", "Wi-Fiを保存して再起動");
  html += F("</button></form></div>");
  sendPage(html);
}

String typeSelectHtml(const String& name, ValueType current,
                      bool validateValue = false) {
  String html = "<select class='type' name='" + name + "'";
  if (validateValue)
    html += " onchange=\"validateInput(this.closest('.osc-row').querySelector('.msg-value'))\"";
  html += ">";
  html += "<option value='0'" + String(current == TYPE_FLOAT ? " selected" : "") + ">Float</option>";
  html += "<option value='1'" + String(current == TYPE_INT ? " selected" : "") + ">Int</option>";
  html += "<option value='2'" + String(current == TYPE_STRING ? " selected" : "") + ">String</option></select>";
  return html;
}

String messageRowHtml(const String& group, const char* event, uint8_t order,
                      const KeyOscMessage& message) {
  const String prefix = String(event) == "press" ? "p" : "r";
  String html = "<div class='osc-row' data-group='" + group + "' data-event='" + event + "'>";
  html += "<div class='order'><button type='button' class='mv' onclick='moveMsg(this,-1)'>&uarr;</button><button type='button' class='mv' onclick='moveMsg(this,1)'>&darr;</button></div>";
  html += "<div class='field'><label>" + String(tr("OSC Address", "OSCアドレス")) + "</label><input class='msg-address' maxlength='192' required name='" + prefix + "_address_" + group + "_" + String(order) + "' value='" + htmlEscape(message.address) + "' oninput='limitAndValidate(this,192)'><small><span class='err'></span><span class='bytes'></span></small></div>";
  html += "<div class='field'><label>" + String(tr("Type", "型")) + "</label>" + typeSelectHtml(prefix + "_type_" + group + "_" + String(order), message.valueType, true) + "<small></small></div>";
  html += "<div class='field'><label>" + String(tr("Value", "値")) + "</label><input class='msg-value' maxlength='128' name='" + prefix + "_value_" + group + "_" + String(order) + "' value='" + htmlEscape(message.valueStr) + "' oninput='limitAndValidate(this,128)'><small><span class='err'></span><span class='bytes'></span></small></div>";
  html += "<button type='button' class='remove-msg' onclick='removeMsg(this)'>" + String(tr("Delete", "削除")) + "</button></div>";
  return html;
}

String pressReleaseHtml(const String& group, const KeySetting& setting,
                        bool sequenceMode) {
  String html = "<div id='pr-" + group + "' style='display:" + (sequenceMode ? "none" : "block") + "'>";
  html += "<div class='usage'><strong>" + String(tr("Messages", "メッセージ")) + " <span id='count-" + group + "'>" + String(setting.pressMessageCount + setting.releaseMessageCount) + "</span> / 8</strong><span>" + String(tr("Press + Release", "押した時＋離した時")) + "</span></div>";
  html += "<input type='hidden' id='p-count-" + group + "' name='p_count_" + group + "' value='" + String(setting.pressMessageCount) + "'><input type='hidden' id='r-count-" + group + "' name='r_count_" + group + "' value='" + String(setting.releaseMessageCount) + "'>";
  html += "<div class='event-tabs'><button type='button' class='event-tab active' onclick=\"showEvent('" + group + "','press',this)\">" + String(tr("Press", "押した時")) + "</button><button type='button' class='event-tab' onclick=\"showEvent('" + group + "','release',this)\">" + String(tr("Release", "離した時")) + "</button></div>";
  html += "<div class='event-panel' data-group='" + group + "' data-event='press'><div class='osc-list' id='list-press-" + group + "'>";
  for (uint8_t i = 0; i < setting.pressMessageCount; ++i)
    html += messageRowHtml(group, "press", i, setting.pressMessages[i]);
  html += "</div><div class='empty'>" + String(tr("No OSC message is sent when pressed.", "押したときはOSCメッセージを送信しません。")) + "</div><button type='button' class='add-msg' data-group='" + group + "' data-event='press' onclick='addMsg(this)'>" + String(tr("+ Add OSC Message", "+ OSCメッセージを追加")) + "</button></div>";
  html += "<div class='event-panel' data-group='" + group + "' data-event='release' style='display:none'><div class='osc-list' id='list-release-" + group + "'>";
  for (uint8_t i = 0; i < setting.releaseMessageCount; ++i)
    html += messageRowHtml(group, "release", i, setting.releaseMessages[i]);
  html += "</div><div class='empty'>" + String(tr("No OSC message is sent when released.", "離したときはOSCメッセージを送信しません。")) + "</div><button type='button' class='add-msg' data-group='" + group + "' data-event='release' onclick='addMsg(this)'>" + String(tr("+ Add OSC Message", "+ OSCメッセージを追加")) + "</button></div></div>";
  return html;
}

void appendKeyCard(String& html, const KeySetting& setting, size_t cardIndex) {
  const String collapseKey = String(cardIndex) + "-" + setting.identity;
  const String deviceLabel = setting.builtIn ? "DualKey" : "Key";
  html += F("<div class='card device' data-device-index='");
  html += cardIndex;
  html += F("' data-collapse-key='");
  html += htmlEscape(collapseKey);
  html += F("'><div class='device-head'><h2><button id='collapse-");
  html += cardIndex;
  html += F("' class='collapse-button' type='button' aria-expanded='true' onclick=\"toggleDevice('");
  html += cardIndex;
  html += F("','");
  html += htmlEscape(collapseKey);
  html += F("')\">&#9660;</button><span class='badge badge-type'>");
  html += deviceLabel;
  html += F("</span> ");
  html += htmlEscape(setting.displayName);
  html += String(" <span class='badge badge-on'>") + tr("Connected", "接続済み") + "</span>";
  html += F("</h2></div><div id='device-body-");
  html += cardIndex;
  html += F("' class='device-body'><div class='uid'>");
  html += htmlEscape(setting.identity);
  html += F("</div><input type='hidden' name='identity_");
  html += cardIndex;
  html += F("' value='");
  html += htmlEscape(setting.identity);
  html += F("'><div class='key-grid'><div><label>");
  html += tr("Device Name", "デバイス名");
  html += F("</label><input name='display_name_");
  html += cardIndex;
  html += F("' maxlength='64' required value='");
  html += htmlEscape(setting.displayName);
  html += F("'></div><div><label>");
  html += tr("Key Mode", "キーモード");
  html += F("</label><select name='mode_");
  html += cardIndex;
  html += F("' onchange=\"toggleKeyMode('pr-");
  html += cardIndex;
  html += F("','seq-");
  html += cardIndex;
  html += F("',this)\"><option value='0'");
  if (setting.mode == MODE_PRESS_RELEASE) html += F(" selected");
  html += F(">");
  html += tr("Press / Release", "押した時／離した時");
  html += F("</option><option value='1'");
  if (setting.mode == MODE_SEQUENCE) html += F(" selected");
  html += F(">");
  html += tr("Sequence", "シーケンス");
  html += F("</option></select></div></div>");
  html += pressReleaseHtml(String(cardIndex), setting, setting.mode == MODE_SEQUENCE);
  html += F("<div id='seq-");
  html += cardIndex;
  html += F("' class='sequence-card' style='display:");
  html += setting.mode == MODE_SEQUENCE ? F("block") : F("none");
  html += F("'><h3>");
  html += tr("Advance the value on each press", "押すたびに値を進める");
  html += F("</h3><p class='note'>");
  html += tr("Move from Start by Step and return to Start after End.", "開始値から増減量ずつ進み、終了値を超えると開始値へ戻ります。");
  html += F("</p><div class='seq-grid'><div class='address-field seq-address'><label>");
  html += tr("OSC Address", "OSCアドレス");
  html += F("</label><input class='osc-address' maxlength='192' required name='seq_address_");
  html += cardIndex;
  html += F("' value='");
  html += htmlEscape(setting.sequence.address);
  html += F("' oninput='limitAndValidate(this,192)'><small><span class='err'></span><span class='bytes'></span></small></div><div><label>");
  html += tr("Start", "開始値");
  html += F("</label><input type='number' step='any' required name='seq_start_"); html += cardIndex; html += F("' value='");
  html += String(setting.sequence.start, 7);
  html += F("'></div><div><label>"); html += tr("End", "終了値"); html += F("</label><input type='number' step='any' required name='seq_end_"); html += cardIndex; html += F("' value='");
  html += String(setting.sequence.end, 7);
  html += F("'></div><div><label>"); html += tr("Step", "増減量"); html += F("</label><input type='number' step='any' required name='seq_step_"); html += cardIndex; html += F("' value='");
  html += String(setting.sequence.step, 7);
  html += F("'></div><div><label>"); html += tr("Type", "型"); html += F("</label>");
  html += typeSelectHtml("seq_type_" + String(cardIndex), setting.sequence.valueType);
  html += F("</div></div></div></div></div>");
}

void appendSavedDeviceCard(String& html, const KeySetting& setting) {
  String uid = setting.identity.startsWith("chain:")
                   ? setting.identity.substring(6) : setting.identity;
  html += F("<div class='card saved-device-card'><h2><span class='badge badge-type'>Key</span> ");
  html += htmlEscape(setting.displayName);
  html += F(" <span class='badge badge-off'>");
  html += tr("Not Connected", "未接続");
  html += F("</span></h2><p class='meta'>");
  html += tr("Type: ", "種類: ");
  html += F("<strong>Key</strong></p><div class='uid'>");
  html += htmlEscape(uid);
  html += F("</div><form method='post' action='/delete_device' onsubmit='deleteSavedDevice(event,this);return false'><input type='hidden' name='identity' value='");
  html += htmlEscape(setting.identity);
  html += F("'><button class='btn-warning' type='submit'>");
  html += tr("Delete Settings", "設定を削除");
  html += F("</button></form></div>");
}

void sendStatusPage(const String& message = String()) {
  String html = pageStart("ChainOSCmini");
  html += F("<div id='save-toast' class='toast' hidden></div><h1>Chain OSC Setting</h1>");
  html += F("<div class='card language-row'><h2>"); html += tr("Language", "言語");
  html += F("</h2><form action='/set_language' method='post'><select name='language' onchange='this.form.submit()'><option value='en'");
  if (!isJapaneseUi()) html += F(" selected");
  html += F(">English</option><option value='ja'");
  if (isJapaneseUi()) html += F(" selected");
  html += F(">日本語</option></select></form></div>");
  if (!message.isEmpty()) {
    html += F("<p class='status'>");
    html += htmlEscape(message);
    html += F("</p>");
  }
  html += F("<div class='card'><h2>"); html += tr("System", "システム");
  html += F("</h2><p class='status'>"); html += tr("Wi-Fi connected", "Wi-Fi接続済み");
  html += F("</p><div class='system-grid'><div class='system-item'><strong>Version</strong>");
  html += APP_VERSION;
  html += F("</div><div class='system-item'><strong>"); html += tr("IP Address", "IPアドレス"); html += F("</strong><code>");
  html += WiFi.localIP().toString();
  html += F("</code></div><div class='system-item'><strong>mDNS</strong><code>http://");
  html += WIFI_MDNS_HOST;
  html += F(".local/</code></div></div></div>");
  html += F("<form id='settings-form' method='post' action='/save-all' onsubmit='saveSettings(event);return false'><div class='card'><h2>"); html += tr("OSC Target", "OSC送信先"); html += F("</h2>");
  html += F("<label for='osc_host'>"); html += tr("Host or IP address", "ホスト名またはIPアドレス"); html += F("</label>");
  html += F("<input id='osc_host' name='osc_host' maxlength='253' required value='");
  html += htmlEscape(oscTargetHost());
  html += F("'>");
  html += F("<label for='osc_port'>"); html += tr("UDP Port", "UDPポート"); html += F("</label>");
  html += F("<input id='osc_port' name='osc_port' type='number' min='1' max='65535' required value='");
  html += oscTargetPort();
  html += F("'></div>");
  html += F("<h2 class='section-title'>"); html += tr("Connected Devices", "接続中のデバイス"); html += F("</h2>");
  size_t cardIndex = 0;
  for (size_t index = 0; index < keySettingsCount(); ++index) {
    KeySetting* setting = keySettingsAt(index);
    if (setting != nullptr &&
        (setting->builtIn || setting->connectedPortMask != 0)) {
      appendKeyCard(html, *setting, cardIndex++);
    }
  }
  html += F("<input type='hidden' name='connected_count' value='");
  html += cardIndex;
  html += F("'><div class='save-bar'><span id='dirty-status' class='dirty-status' hidden>");
  html += tr("Unsaved changes", "未保存の変更があります");
  html += F("</span><button class='primary' type='submit'>");
  html += tr("Save All Settings", "すべての設定を保存");
  html += F("</button></div></form>");
  html += F("<div class='card saved-settings'><h2>");
  html += tr("Saved Device Settings", "保存済みデバイス設定");
  html += F("</h2><p class='note'>");
  html += tr("Only saved devices that are not currently connected are shown.", "設定が保存されており、現在は接続されていないデバイスだけを表示します。");
  html += F("</p></div>");
  for (size_t index = 0; index < keySettingsCount(); ++index) {
    KeySetting* setting = keySettingsAt(index);
    if (setting != nullptr && !setting->builtIn &&
        setting->connectedPortMask == 0) {
      appendSavedDeviceCard(html, *setting);
    }
  }
  html += F("<div class='card wifi-actions'><h2>"); html += tr("Wi-Fi Settings", "Wi-Fi設定");
  html += F("</h2><form method='post' action='/forget-wifi' onsubmit=\"return confirm('"); html += tr("Delete saved Wi-Fi settings?", "保存済みWi-Fi設定を削除しますか？"); html += F("')\"><button class='danger' type='submit'>");
  html += tr("Forget Wi-Fi Settings", "Wi-Fi設定を削除"); html += F("</button></form></div>");
  sendPage(html);
}

bool parseInt32(const String& text, int32_t& value) {
  if (text.isEmpty()) return false;
  size_t index = text[0] == '-' ? 1 : 0;
  if (index == text.length()) return false;
  for (; index < text.length(); ++index) {
    if (!isdigit(static_cast<unsigned char>(text[index]))) return false;
  }
  const long long parsed = strtoll(text.c_str(), nullptr, 10);
  if (parsed < INT32_MIN || parsed > INT32_MAX) return false;
  value = static_cast<int32_t>(parsed);
  return true;
}

bool readKeySetting(size_t formIndex, KeySetting& candidate) {
  const String suffix = "_" + String(formIndex);
  const String identity = server.arg("identity" + suffix);
  KeySetting* current = nullptr;
  for (size_t index = 0; index < keySettingsCount(); ++index) {
    KeySetting* setting = keySettingsAt(index);
    if (setting != nullptr && setting->identity == identity &&
        (setting->builtIn || setting->connectedPortMask != 0)) {
      current = setting;
      break;
    }
  }
  if (current == nullptr) return false;
  candidate = *current;
  candidate.displayName = server.arg("display_name" + suffix);
  candidate.displayName.trim();
  candidate.mode = server.arg("mode" + suffix).toInt() == MODE_SEQUENCE
                       ? MODE_SEQUENCE : MODE_PRESS_RELEASE;
  const int pressCount = server.arg("p_count" + suffix).toInt();
  const int releaseCount = server.arg("r_count" + suffix).toInt();
  if (candidate.displayName.isEmpty() || pressCount < 0 || releaseCount < 0 ||
      pressCount + releaseCount > MAX_KEY_OSC_MESSAGES) return false;
  candidate.pressMessageCount = pressCount;
  candidate.releaseMessageCount = releaseCount;

  bool valid = true;
  for (uint8_t index = 0; valid && index < candidate.pressMessageCount; ++index) {
    KeyOscMessage& msg = candidate.pressMessages[index];
    const String item = suffix + "_" + String(index);
    msg.address = server.arg("p_address" + item); msg.address.trim();
    msg.valueStr = server.arg("p_value" + item);
    msg.valueType = static_cast<ValueType>(constrain(server.arg("p_type" + item).toInt(), 0, 2));
    if (msg.valueStr.length() > 128) valid = false;
    if (msg.valueType == TYPE_INT) { int32_t parsed; valid = valid && parseInt32(msg.valueStr, parsed); }
    else if (msg.valueType == TYPE_FLOAT) { char* end = nullptr; const float parsed = strtof(msg.valueStr.c_str(), &end); valid = valid && end != msg.valueStr.c_str() && *end == '\0' && isfinite(parsed); }
  }
  for (uint8_t index = 0; valid && index < candidate.releaseMessageCount; ++index) {
    KeyOscMessage& msg = candidate.releaseMessages[index];
    const String item = suffix + "_" + String(index);
    msg.address = server.arg("r_address" + item); msg.address.trim();
    msg.valueStr = server.arg("r_value" + item);
    msg.valueType = static_cast<ValueType>(constrain(server.arg("r_type" + item).toInt(), 0, 2));
    if (msg.valueStr.length() > 128) valid = false;
    if (msg.valueType == TYPE_INT) { int32_t parsed; valid = valid && parseInt32(msg.valueStr, parsed); }
    else if (msg.valueType == TYPE_FLOAT) { char* end = nullptr; const float parsed = strtof(msg.valueStr.c_str(), &end); valid = valid && end != msg.valueStr.c_str() && *end == '\0' && isfinite(parsed); }
  }
  candidate.sequence.address = server.arg("seq_address" + suffix);
  candidate.sequence.address.trim();
  const String startText = server.arg("seq_start" + suffix);
  const String endText = server.arg("seq_end" + suffix);
  const String stepText = server.arg("seq_step" + suffix);
  char *startEnd = nullptr, *endEnd = nullptr, *stepEnd = nullptr;
  candidate.sequence.start = strtof(startText.c_str(), &startEnd);
  candidate.sequence.end = strtof(endText.c_str(), &endEnd);
  candidate.sequence.step = strtof(stepText.c_str(), &stepEnd);
  candidate.sequence.valueType = static_cast<ValueType>(constrain(server.arg("seq_type" + suffix).toInt(), 0, 2));
  valid = valid && startEnd != startText.c_str() && *startEnd == '\0' &&
          endEnd != endText.c_str() && *endEnd == '\0' &&
          stepEnd != stepText.c_str() && *stepEnd == '\0' &&
          isfinite(candidate.sequence.start) && isfinite(candidate.sequence.end) &&
          isfinite(candidate.sequence.step);
  keySettingsNormalizeSequence(candidate.sequence);
  return valid;
}

void sendActionResult(int status, const String& message) {
  if (server.hasArg("ajax")) server.send(status, "text/plain; charset=utf-8", message);
  else sendStatusPage(message);
}

void handleSaveAll() {
  String host = server.arg("osc_host");
  String portText = server.arg("osc_port");
  host.trim(); portText.trim();
  bool numericPort = !portText.isEmpty();
  for (size_t i = 0; numericPort && i < portText.length(); ++i)
    numericPort = isdigit(static_cast<unsigned char>(portText[i]));
  const unsigned long port = numericPort ? portText.toInt() : 0;
  if (host.isEmpty() || host.length() > 253 || port < 1 || port > 65535) {
    sendActionResult(400, tr("Could not save settings. Check OSC Host and Port.", "設定を保存できませんでした。OSC送信先とポートを確認してください。"));
    return;
  }
  const int count = constrain(server.arg("connected_count").toInt(), 0, 40);
  for (int i = 0; i < count; ++i) {
    KeySetting candidate;
    if (!readKeySetting(i, candidate)) {
      sendActionResult(400, tr("Could not save settings. Check the device fields.", "設定を保存できませんでした。デバイスの設定項目を確認してください。"));
      return;
    }
  }
  if (!oscSaveTarget(host, static_cast<uint16_t>(port))) {
    sendActionResult(500, tr("Could not write settings to storage.", "設定をストレージへ書き込めませんでした。"));
    return;
  }
  for (int i = 0; i < count; ++i) {
    KeySetting candidate;
    if (!readKeySetting(i, candidate) || !keySettingsSave(candidate)) {
      sendActionResult(500, tr("Could not write all device settings to storage.", "すべてのデバイス設定をストレージへ書き込めませんでした。"));
      return;
    }
  }
  sendActionResult(200, tr("All settings saved.", "すべての設定を保存しました。"));
}

void handleDeleteDevice() {
  const String identity = server.arg("identity");
  if (!keySettingsDelete(identity)) {
    sendActionResult(400, tr("Could not delete device settings.", "デバイス設定を削除できませんでした。"));
    return;
  }
  sendActionResult(200, tr("Device settings deleted.", "デバイス設定を削除しました。"));
}

void handleSaveKey() {
  const String identity = server.arg("identity");
  KeySetting* current = nullptr;
  for (size_t index = 0; index < keySettingsCount(); ++index) {
    KeySetting* setting = keySettingsAt(index);
    if (setting != nullptr && setting->identity == identity) {
      current = setting;
      break;
    }
  }
  if (current == nullptr) {
    sendStatusPage(tr("Unknown key setting.", "対象のキー設定が見つかりません。"));
    return;
  }

  KeySetting candidate = *current;
  candidate.displayName = server.arg("display_name");
  candidate.displayName.trim();
  candidate.mode = server.arg("mode").toInt() == MODE_SEQUENCE
                       ? MODE_SEQUENCE : MODE_PRESS_RELEASE;
  const int pressCount = server.arg("p_count").toInt();
  const int releaseCount = server.arg("r_count").toInt();
  if (pressCount < 0 || releaseCount < 0 ||
      pressCount + releaseCount > MAX_KEY_OSC_MESSAGES) {
    sendStatusPage(tr("Press and Release messages must total 8 or fewer.", "PressとReleaseのメッセージは合計8件以内にしてください。"));
    return;
  }
  candidate.pressMessageCount = pressCount;
  candidate.releaseMessageCount = releaseCount;

  bool valid = !candidate.displayName.isEmpty();
  for (uint8_t index = 0; valid && index < candidate.pressMessageCount; ++index) {
    KeyOscMessage& message = candidate.pressMessages[index];
    message.address = server.arg("p_address_" + String(index));
    message.address.trim();
    message.valueStr = server.arg("p_value_" + String(index));
    message.valueType = static_cast<ValueType>(constrain(
        server.arg("p_type_" + String(index)).toInt(),
        static_cast<int>(TYPE_FLOAT), static_cast<int>(TYPE_STRING)));
    if (message.valueStr.length() > 128) valid = false;
    if (message.valueType == TYPE_INT) {
      int32_t parsed;
      valid = valid && parseInt32(message.valueStr, parsed);
    } else if (message.valueType == TYPE_FLOAT) {
      char* end = nullptr;
      const float parsed = strtof(message.valueStr.c_str(), &end);
      valid = valid && end != message.valueStr.c_str() && *end == '\0' && isfinite(parsed);
    }
  }
  for (uint8_t index = 0; valid && index < candidate.releaseMessageCount; ++index) {
    KeyOscMessage& message = candidate.releaseMessages[index];
    message.address = server.arg("r_address_" + String(index));
    message.address.trim();
    message.valueStr = server.arg("r_value_" + String(index));
    message.valueType = static_cast<ValueType>(constrain(
        server.arg("r_type_" + String(index)).toInt(),
        static_cast<int>(TYPE_FLOAT), static_cast<int>(TYPE_STRING)));
    if (message.valueStr.length() > 128) valid = false;
    if (message.valueType == TYPE_INT) {
      int32_t parsed;
      valid = valid && parseInt32(message.valueStr, parsed);
    } else if (message.valueType == TYPE_FLOAT) {
      char* end = nullptr;
      const float parsed = strtof(message.valueStr.c_str(), &end);
      valid = valid && end != message.valueStr.c_str() && *end == '\0' && isfinite(parsed);
    }
  }
  candidate.sequence.address = server.arg("seq_address");
  candidate.sequence.address.trim();
  char* startEnd = nullptr;
  char* endEnd = nullptr;
  char* stepEnd = nullptr;
  const String startText = server.arg("seq_start");
  const String endText = server.arg("seq_end");
  const String stepText = server.arg("seq_step");
  candidate.sequence.start = strtof(startText.c_str(), &startEnd);
  candidate.sequence.end = strtof(endText.c_str(), &endEnd);
  candidate.sequence.step = strtof(stepText.c_str(), &stepEnd);
  candidate.sequence.valueType = static_cast<ValueType>(constrain(
      server.arg("seq_type").toInt(), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  valid = valid && startEnd != startText.c_str() && *startEnd == '\0' &&
          endEnd != endText.c_str() && *endEnd == '\0' &&
          stepEnd != stepText.c_str() && *stepEnd == '\0' &&
          isfinite(candidate.sequence.start) && isfinite(candidate.sequence.end) &&
          isfinite(candidate.sequence.step);
  keySettingsNormalizeSequence(candidate.sequence);

  if (!valid || !keySettingsSave(candidate)) {
    sendStatusPage(tr("Could not save key settings. Check Address and values.", "キー設定を保存できませんでした。Addressと値を確認してください。"));
    return;
  }
  sendStatusPage(tr("Key settings saved.", "キー設定を保存しました。"));
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
    sendStatusPage(tr("Could not save OSC target. Check Host and Port.", "OSC送信先を保存できませんでした。ホストとポートを確認してください。"));
    return;
  }
  sendStatusPage(tr("OSC target saved.", "OSC送信先を保存しました。"));
}

void handleRoot() {
  applyBrowserLanguageOnFirstVisit();
  if (networkState == NetworkState::AP_MODE) {
    sendProvisioningPage();
  } else {
    sendStatusPage();
  }
}

void handleSetLanguage() {
  if (server.hasArg("language")) {
    uiLanguage = server.arg("language") == "ja" ? UiLanguage::JAPANESE
                                                  : UiLanguage::ENGLISH;
    saveUiLanguage();
  }
  server.sendHeader("Location", "/", true);
  server.send(303, "text/plain", "");
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
    error = tr("SSID must be 1–32 bytes.", "SSIDは1～32バイトで入力してください。");
    return false;
  }
  bool valid64DigitPsk = passwordBytes == 64;
  for (size_t index = 0; valid64DigitPsk && index < passwordBytes; ++index) {
    valid64DigitPsk = isxdigit(static_cast<unsigned char>(password[index]));
  }
  if (passwordBytes != 0 &&
      (passwordBytes < 8 || (passwordBytes > 63 && !valid64DigitPsk))) {
    error = tr("Password must be blank, 8–63 bytes, or a 64-digit hexadecimal PSK.", "パスワードは空欄、8～63バイト、または64桁の16進数PSKで入力してください。");
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
    sendProvisioningPage(tr("Could not open settings storage.", "設定ストレージを開けませんでした。"));
    return;
  }
  const size_t ssidWritten = preferences.putString("ssid", ssid);
  const size_t passwordWritten = preferences.putString("password", password);
  preferences.end();
  if (ssidWritten == 0 || (password.length() > 0 && passwordWritten == 0)) {
    sendProvisioningPage(tr("Could not save Wi-Fi settings.", "Wi-Fi設定を保存できませんでした。"));
    return;
  }

  Serial.printf("[ChainOSCmini][NET] credentials_saved ssid_bytes=%u password_bytes=%u\n",
                static_cast<unsigned int>(ssid.length()),
                static_cast<unsigned int>(password.length()));
  String html = pageStart("Wi-Fi Saved");
  html += F("<h1>Chain OSC Setting</h1><div class='card'><h2>"); html += tr("Wi-Fi settings saved", "Wi-Fi設定を保存しました");
  html += F("</h2><p class='status'>"); html += tr("Restarting ChainOSCmini…", "ChainOSCminiを再起動します…"); html += F("</p></div>");
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
  html += F("<h1>Chain OSC Setting</h1><div class='card'><h2>"); html += tr("Wi-Fi settings deleted", "Wi-Fi設定を削除しました");
  html += F("</h2><p class='status'>"); html += tr("Restarting in setup mode…", "設定モードで再起動します…"); html += F("</p></div>");
  sendPage(html);
  scheduleRestart();
}

void registerRoutes() {
  if (routesRegistered) {
    return;
  }
  const char* trackedHeaders[] = {"Accept-Language"};
  server.collectHeaders(trackedHeaders, 1);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/set_language", HTTP_POST, handleSetLanguage);
  server.on("/save-wifi", HTTP_POST, handleSaveWifi);
  server.on("/forget-wifi", HTTP_POST, handleForgetWifi);
  server.on("/save-osc", HTTP_POST, handleSaveOsc);
  server.on("/save-key", HTTP_POST, handleSaveKey);
  server.on("/save-all", HTTP_POST, handleSaveAll);
  server.on("/delete_device", HTTP_POST, handleDeleteDevice);
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

  if (preferences.begin("ui", true)) {
    const uint8_t storedLanguage = preferences.getUChar("language", 0xff);
    uiLanguageConfigured = storedLanguage <= static_cast<uint8_t>(UiLanguage::JAPANESE);
    if (uiLanguageConfigured)
      uiLanguage = static_cast<UiLanguage>(storedLanguage);
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
