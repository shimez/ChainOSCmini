---
layout: default
title: ChainOSCmini Preset Quick Start
permalink: /en/quick-start-presets/
---

# ChainOSCmini Preset Quick Start

[日本語版](../../quick-start-presets/)

This guide installs ChainOSCmini and sends OSC to VRChat with a shared Key preset. See the [English User Guide](../user-guide/) for details.

## What you need

- M5Stack Chain DualKey
- An optional M5Stack Chain Key
- A USB Type-C data cable
- A 2.4 GHz Wi-Fi network
- A computer running VRChat
- Desktop Chrome or Edge

## 1. Install firmware

1. Open the [ChainOSCmini Web Installer](https://shimez.github.io/ChainOSCmini/installer/).
2. Put the DualKey power switch in the center `OFF/USB` position.
3. Connect USB, select `Install ChainOSCmini`, and choose its serial port.
4. Follow the installer instructions.

If the port is missing, hold KEY1 while reconnecting USB to try download mode.

## 2. Read the connection status LEDs

| LED indication | Status |
|---|---|
| Slowly pulsing purple | AP Mode, waiting for Wi-Fi setup |
| Slowly pulsing blue | Connecting to Wi-Fi |
| Solid blue | Connected to Wi-Fi |
| Orange | The corresponding built-in DualKey button is pressed |

When the button is released, its LED returns to the current Wi-Fi status color.

Immediately after installation, a DualKey without saved Wi-Fi settings slowly pulses purple. This is normal and means it is waiting for Wi-Fi setup in the next step.

## 3. Configure Wi-Fi

Connect to `ChainOSCmini-Setup` with password `12345678`. If needed, open `http://192.168.4.1/`, then save a 2.4 GHz Wi-Fi SSID and password.

## 4. Enable VRChat OSC

```text
Action Menu → Options → OSC → Enabled
```

## 5. Configure the destination

Use `ipconfig` to find the VRChat computer's IPv4 address. Open `http://chainoscmini.local/`, enter that address under `OSC Target`, and set the port to `9000`. If the `.local` address does not open, start PowerShell, run `Resolve-DnsName chainoscmini.local`, and open the reported ChainOSCmini IP address directly in the browser.

## 6. Import a Key preset

Download a Key JSON file from [M5ChainOSC Device Presets](https://github.com/shimez/M5ChainOSC/tree/main/presets/key). ChainOSCmini and M5ChainOSC use the same Key preset format.

1. Open `…` on DualKey KEY1/KEY2 or a connected Chain Key.
2. Select `Import Preset (JSON)`.
3. Choose the Key preset and confirm.
4. Verify that the imported values appear, then operate the Key in VRChat.

If OSC is not received, check the IPv4 address, port `9000`, Windows firewall, and VRChat OSC status.
