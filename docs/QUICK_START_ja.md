---
layout: default
title: ChainOSCmini クイックスタート
permalink: /quick-start/
---

# ChainOSCmini クイックスタート

[English version](../en/quick-start/)

このガイドでは、Device Presetを使わずにDualKey KEY1を手入力で設定し、ChainOSCminiからVRChatへOSCメッセージを送信して動作を確認します。詳しい設定は[日本語ユーザーガイド](../user-guide/)を参照してください。

## 用意するもの

- M5Stack Chain DualKey
- データ通信対応USB Type-Cケーブル
- 2.4 GHz帯Wi-Fi
- VRChatを実行するPC
- デスクトップ版ChromeまたはEdge

## 1. ファームウェアを書き込む

1. [Web Installer](../installer/)をChromeまたはEdgeで開きます。
2. DualKeyの電源スイッチを`OFF/USB`にしてUSB接続します。
3. `Install ChainOSCmini`を押し、シリアルポートを選択します。
4. 画面の案内に従ってインストールします。

## 2. Wi-Fiを設定する

1. SSID `ChainOSCmini-Setup`へ接続します。
2. パスワード`12345678`を入力します。
3. キャプティブポータルが開かない場合は`http://192.168.4.1/`を開きます。
4. 2.4 GHz帯Wi-FiのSSIDとパスワードを保存します。

LEDは、紫のゆっくりした点滅がAP Mode、青のゆっくりした点滅が接続中、青の点灯が接続済みを示します。

## 3. VRChatでOSCを有効にする

VRChatを起動し、リングメニュー → オプション → OSC → 有効に設定します。

## 4. VRChatを実行しているPCのIPv4アドレスを確認する

WindowsでPowerShellまたはコマンドプロンプトを開き、`ipconfig`を実行します。ChainOSCminiと同じネットワークに接続しているWi-FiまたはEthernetアダプターのIPv4アドレスを確認してください。VPNや仮想アダプターではなく、実際に接続中のアダプターを選びます。

## 5. 設定画面を開く

ブラウザーで`http://chainoscmini.local/`を開きます。Windowsで開けない場合はPowerShellで次を実行し、表示されたIPv4アドレスをブラウザーで開きます。

```powershell
Resolve-DnsName chainoscmini.local
```

## 6. OSC送信先を設定する

1. 「OSC送信先」のホスト名またはIPアドレスに、VRChatを実行しているPCのIPv4アドレスを入力します。
2. UDPポートに`9000`を入力します。

## 7. DualKey KEY1にVoice操作を設定する

設定画面の「DualKey KEY1」カードで、まず「押した時」に次の値を入力します。

- OSCアドレス：`/input/Voice`
- 型：`Int`
- 値：`1`

「離した時」に切り替えて、次の値を入力します。

- OSCアドレス：`/input/Voice`
- 型：`Int`
- 値：`0`

## 8. 保存して動作を確認する

1. 「すべての設定を保存」を押します。
2. VRChatが起動していてOSCが有効な状態で、DualKeyのKEY1を押します。
3. VRChatのVoice入力状態が切り替わり、Keyを離すと元に戻ることを確認します。

Voiceが切り替われば、ChainOSCminiからOSCメッセージを送信できています。KEY2、Chainデバイス、Encoder、Angle、ToF、Joystick、Sequenceの詳細、Device Presetは[日本語ユーザーガイド](../user-guide/)で説明しています。設定の再利用や他のChainOSCシリーズとの共有にはDevice Presetを利用できます。
