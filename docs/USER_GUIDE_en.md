---
layout: default
title: ChainOSCmini User Guide
permalink: /en/user-guide/
---

# ChainOSCmini User Guide

[日本語版](../../user-guide/)

This guide covers initial setup and the Web UI after installing ChainOSCmini on a Chain DualKey.

> [!IMPORTANT]
> ChainOSCmini is an unofficial, independently developed project. It is not an official M5Stack product.

## 1. Configure Wi-Fi

1. Put the power switch in the center `OFF/USB` position and power the device by USB.
2. Connect a phone or computer to `ChainOSCmini-Setup`.
3. Enter `12345678`.
4. If the captive portal does not open, visit `http://192.168.4.1/`.
5. Save the SSID and password of a 2.4 GHz Wi-Fi network.

The ESP32-S3 supports 2.4 GHz Wi-Fi only.

### Reading the built-in LEDs

- Slowly pulsing purple: AP Mode, waiting for Wi-Fi setup
- Slowly pulsing blue: connecting to Wi-Fi
- Solid blue: connected to Wi-Fi
- Orange: the corresponding built-in key is pressed

When the key is released, its LED returns to the current Wi-Fi status color.

## 2. Open the Web UI

Open `http://chainoscmini.local/` from the same network. On Windows, resolve it with:

```powershell
Resolve-DnsName chainoscmini.local
```

If the `.local` address does not open on Windows, start PowerShell and run the command above. Open the reported IP address directly in the browser, for example `http://192.168.x.x/`.

> [!IMPORTANT]
> The Web UI does not require authentication. Use ChainOSCmini only on a trusted local network, such as your home LAN. Use on shared networks at event venues, hotels, or public Wi-Fi hotspots is not recommended.

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

DualKey KEY1/KEY2 and Chain Key are supported.

### Press / Release

- Press and Release may contain up to eight messages in total.
- Each message has an OSC Address, Type, and Value.
- Messages are sent in displayed order and can be reordered.
- An event with zero messages sends nothing.
- Types are Float, Int, and String.

### Sequence

Each press advances from Start by Step. After passing End, it wraps to Start. Release sends nothing. Use Float or Int for the sequence type.

OSC addresses must start with `/`, are limited to 192 bytes, and cannot contain spaces or `# * , ? [ ] { }`. Values are limited to 128 bytes.

## 6. Configure an Encoder

### Encoder Rotation

| Parameter | Meaning |
| --- | --- |
| `OSC Address` | OSC destination for rotation values. |
| `Mode` | `Absolute` uses the current counter position; `Increment` uses the change since the previous reading. |
| `Abs In Min` | Input position mapped to Out Min in Absolute mode. Hidden in Increment mode. |
| `Abs In Max` | End of the repeating Absolute range. Hidden in Increment mode. |
| `Inc Scale` | Multiplier applied to each Increment delta. Reverse rotation produces a negative value. |
| `Out Min` | Lower output limit. |
| `Out Max` | Upper output limit. |
| `Out Type` | `Float`, rounded `Int`, or numeric `String`. |

With the defaults (`Abs In Min = 0`, `Abs In Max = 20`, `Out Min = 0`, `Out Max = 1`), counts `0`, `5`, `10`, and `15` produce approximately `0.00`, `0.25`, `0.50`, and `0.75`; count `20` wraps to `0.00`.

Because an encoder has no physical minimum or maximum position, it is useful to think of `Abs In Max` as the number of counts required to traverse one output cycle. A smaller value cycles more quickly, while a larger value provides finer adjustment. The current implementation wraps upon reaching `Abs In Max`, so the example reaches approximately `0.95` before returning to `0.00`, rather than sending `1.00` itself.

### Encoder Click

The green-accented click section works like Key. Press and Release support up to eight messages in total, and Sequence advances on each press. Rotation and click settings are independent.

## 7. Configure an Angle

| Parameter | Meaning |
| --- | --- |
| `OSC Address` | OSC destination for the angle value. |
| `Resolution` | `12-bit` uses an input range of 0–4095; `8-bit` uses 0–255. |
| `Deadband` | Sends only when the input differs from the last sent value by at least this amount, reducing jitter traffic. |
| `Out Min` | Output corresponding to the minimum sensor input. |
| `Out Max` | Output corresponding to the maximum sensor input. |
| `Out Type` | `Float`, rounded `Int`, or numeric `String`. |

The first reading initializes the input without sending OSC. Later changes that meet the Deadband are mapped to Out Min–Out Max and sent. `12-bit` is recommended for smoother control. With 12-bit resolution and an output range of `0` to `1`, a midpoint reading sends approximately `0.5`.

## 8. Configure a ToF

| Parameter | Meaning |
| --- | --- |
| `OSC Address` | OSC destination for the mapped distance value. |
| `Deadband (mm)` | Sends when the distance changes by at least this amount. |
| `Maximum Distance (mm)` | Upper end of the active range, from 31 to 2000 mm. |
| `Direction` | Selects `Near → Out Min / Far → Out Max` or `Near → Out Max / Far → Out Min`. |
| `Out Min / Out Max` | Output range mapped from the active distance range. |
| `Out Type` | `Float` or rounded `Int`. |

The active range is 30 mm or more and less than Maximum Distance. OSC transmission stops when no target is in this range and resumes with the first valid reading after re-entry.

For example, with Maximum Distance `500`, output `0`–`1`, and `Near → Out Max / Far → Out Min`, a reading near 30 mm sends approximately `1` and approaches `0` near 500 mm. At 500 mm or beyond, transmission stops.

## 9. Configure a Joystick

| Parameter | Meaning |
| --- | --- |
| `X Address` | OSC destination for the X axis. |
| `Y Address` | OSC destination for the Y axis. |
| `Invert X / Y` | Reverses the sign of the selected axis. |
| `Deadband` | Sends an axis only after its raw value changes by at least this amount. The raw range is approximately `-127` to `127`; the minimum is `1`. |
| `Out Min / Out Max` | Output range mapped from the -127 to 127 input. |
| `Out Type` | `Float`, `Int`, or `String`. |

When a Joystick is connected to the GPIO47/GPIO48 side, ChainOSCmini always reverses the signs of both X and Y to compensate for the physical orientation. No setting is required. The Web UI's `Invert X` and `Invert Y` options are applied after this automatic correction.

For example, Out Min `-1` and Out Max `1` produce approximately `0` at center and `-1`/`1` at the ends. X and Y are sent independently to their respective OSC addresses.

Joystick Click supports the same Press / Release, eight-message limit, and Sequence behavior as Key. Joystick presets remain compatible with M5ChainOSC.

## 10. Device presets

Open `…` in the upper-right corner of any Key, Encoder, Angle, ToF, or Joystick card.

- `Export Preset (JSON)` exports settings without UID or Device Name.
- `Import Preset (JSON)` applies and immediately stores a preset.

The shared format is `ChainOSC-device-preset`, compatible with M5ChainOSC Key, Encoder, Angle, ToF, and Joystick presets. The legacy `M5ChainOSC-device-preset` format is also accepted.

## 11. Saved devices

Chain Key, Encoder, Angle, ToF, and Joystick settings are stored per UID and survive reconnection, port changes, and device-order changes. Disconnected saved devices appear under `Saved Device Settings`; use `Delete Settings` to remove their stored configuration.

## 12. JSON backup

Full backups use `ChainOSCmini-settings` and include schema and firmware versions. They contain the OSC target, UI language, built-in keys, and saved Chain Keys, but never Wi-Fi credentials.

Invalid JSON, wrong formats, unsupported device types, invalid OSC values, and more than eight messages are rejected without applying the preset.

## 13. Troubleshooting

- If the UI does not open on Windows, run `Resolve-DnsName chainoscmini.local` in PowerShell and open the reported IP address.
- If OSC is not received, check receiver IP, port, firewall, and VRChat OSC status.
- If JSON import fails, confirm that a full backup is selected in the full-settings importer and a device preset in a Key's `…` menu.

## 14. Current scope

The built-in DualKey buttons, Chain Key, Chain Encoder, Chain Angle, Chain ToF, and Chain Joystick are supported. Other Chain devices are planned for later versions.
