---
layout: default
title: ChainOSCmini User Guide
permalink: /en/user-guide/
---

# ChainOSCmini User Guide

[日本語版](../../user-guide/)

This guide covers initial setup and the Web UI after installing ChainOSCmini on a Chain DualKey.

> [!IMPORTANT]
> ChainOSCmini is an unofficial, independently developed project. It is not an official M5Stack product. The Web UI has no authentication; use it only on a trusted local network.

## 1. Configure Wi-Fi

1. Put the power switch in the center `OFF/USB` position and power the device by USB.
2. Connect a phone or computer to `ChainOSCmini-Setup`.
3. Enter `12345678`.
4. If the captive portal does not open, visit `http://192.168.4.1/`.
5. Save the SSID and password of a 2.4 GHz Wi-Fi network.

The ESP32-S3 supports 2.4 GHz Wi-Fi only.

## 2. Open the Web UI

Open `http://chainoscmini.local/` from the same network. On Windows, resolve it with:

```powershell
Resolve-DnsName chainoscmini.local
```

If the `.local` address does not open on Windows, start PowerShell and run the command above. Open the reported IP address directly in the browser, for example `http://192.168.x.x/`.

## 3. Common settings

- `Language`: selects English or Japanese.
- `System`: shows firmware version, IP address, and mDNS address.
- `WiFi`: shows the IP and can erase saved Wi-Fi credentials.
- `Settings Backup & Restore`: exports or imports OSC target, UI language, built-in keys, and saved Chain Keys. Wi-Fi credentials are excluded.
- `OSC Target`: enter the receiver host/IP and UDP port. VRChat normally uses port `9000`.
- `Save All Settings`: saves the OSC target and all connected Key settings.

## 4. Enable OSC in VRChat

```text
Action Menu → Options → OSC → Enabled
```

## 5. Configure a Key

Version 0.9.0 supports DualKey KEY1/KEY2 and Chain Key.

### Press / Release

- Press and Release may contain up to eight messages in total.
- Each message has an OSC Address, Type, and Value.
- Messages are sent in displayed order and can be reordered.
- An event with zero messages sends nothing.
- Types are Float, Int, and String.

### Sequence

Each press advances from Start by Step. After passing End, it wraps to Start. Release sends nothing. Use Float or Int for the sequence type.

OSC addresses must start with `/`, are limited to 192 bytes, and cannot contain spaces or `# * , ? [ ] { }`. Values are limited to 128 bytes.

## 6. Device presets

Open `…` in the upper-right corner of any Key card.

- `Export Preset (JSON)` exports settings without UID or Device Name.
- `Import Preset (JSON)` applies and immediately stores a preset.

The shared format is `ChainOSC-device-preset`, compatible with M5ChainOSC Key presets. The legacy `M5ChainOSC-device-preset` format is also accepted.

## 7. Saved devices

Chain Key settings are stored per UID and survive reconnection, port changes, and device-order changes. Disconnected saved devices appear under `Saved Device Settings`; use `Delete Settings` to remove their stored configuration.

## 8. JSON backup

Full backups use `ChainOSCmini-settings` and include schema and firmware versions. They contain the OSC target, UI language, built-in keys, and saved Chain Keys, but never Wi-Fi credentials.

Invalid JSON, wrong formats, unsupported device types, invalid OSC values, and more than eight messages are rejected without applying the preset.

## 9. Troubleshooting

- If the UI does not open on Windows, run `Resolve-DnsName chainoscmini.local` in PowerShell and open the reported IP address.
- If OSC is not received, check receiver IP, port, firewall, and VRChat OSC status.
- If JSON import fails, confirm that a full backup is selected in the full-settings importer and a device preset in a Key's `…` menu.

## 10. Current scope

Version 0.9.0 supports the built-in DualKey buttons and Chain Key. Encoder, Joystick, Angle, ToF, and other Chain devices are planned for later versions.
