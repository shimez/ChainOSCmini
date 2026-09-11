---
layout: default
title: ChainOSCmini Quick Start
permalink: /en/quick-start/
---

# ChainOSCmini Quick Start

[日本語版](../../quick-start/)

This guide configures DualKey KEY1 manually without a Device Preset, then verifies OSC messages from ChainOSCmini in VRChat. See the [English User Guide](../user-guide/) for details.

## What you need

- M5Stack Chain DualKey
- A data-capable USB Type-C cable
- A 2.4 GHz Wi-Fi network
- A computer running VRChat
- Desktop Chrome or Edge

## 1. Install the firmware

1. Open the [Web Installer](../../installer/) in Chrome or Edge.
2. Set the DualKey power switch to `OFF/USB` and connect it by USB.
3. Select `Install ChainOSCmini` and choose the serial port.
4. Follow the on-screen instructions.

The LEDs slowly pulse purple in AP Mode, slowly pulse blue while connecting, and stay blue when connected.

## 2. Configure Wi-Fi

1. Connect to `ChainOSCmini-Setup`.
2. Enter password `12345678`.
3. If the captive portal does not open, visit `http://192.168.4.1/`.
4. Save the credentials for a 2.4 GHz Wi-Fi network.

## 3. Enable OSC in VRChat

Start VRChat and select **Action Menu → Options → OSC → Enabled**.

## 4. Find the IPv4 address of the VRChat PC

In Windows PowerShell or Command Prompt, run `ipconfig`. Find the IPv4 address of the Wi-Fi or Ethernet adapter connected to the same network as ChainOSCmini. Do not use a VPN or virtual adapter address.

## 5. Open the Web UI

Visit `http://chainoscmini.local/`. If Windows cannot resolve it, run `Resolve-DnsName chainoscmini.local` in PowerShell and open the returned IPv4 address.

## 6. Configure the OSC destination

1. Enter the IPv4 address of the PC running VRChat in the OSC destination host or IP address field.
2. Enter `9000` in the UDP port field.

## 7. Configure a Voice action on DualKey KEY1

In the **DualKey KEY1** card, enter the following values under **Press**:

- OSC Address: `/input/Voice`
- Type: `Int`
- Value: `1`

Switch to **Release** and enter:

- OSC Address: `/input/Voice`
- Type: `Int`
- Value: `0`

## 8. Save and verify the action

1. Select **Save All Settings**.
2. With VRChat running and OSC enabled, press DualKey KEY1.
3. Confirm that VRChat Voice becomes active and returns to its previous state when you release the key.

This confirms that ChainOSCmini sent an OSC message. KEY2, Chain devices, Encoder, Angle, ToF, Joystick, detailed Sequence settings, and Device Presets are covered in the [English User Guide](../user-guide/). Use Device Presets to reuse and share settings across ChainOSC products.
