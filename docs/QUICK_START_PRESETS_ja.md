---
layout: default
title: ChainOSCmini プリセット・クイックスタート
permalink: /quick-start-presets/
---

# ChainOSCmini プリセット・クイックスタート

[English version](../en/quick-start-presets/)

このガイドでは、ファームウェアのインストールからKeyプリセットを使ってVRChatへOSCを送るまでを案内します。詳しい設定は[日本語ユーザーガイド](../user-guide/)を参照してください。

## 用意するもの

- M5Stack Chain DualKey
- 必要に応じてM5Stack Chain Key
- データ通信対応USB Type-Cケーブル
- 2.4 GHz帯Wi-Fi
- VRChatを実行するPC
- デスクトップ版ChromeまたはEdge

## 1. ファームウェアを書き込む

1. [ChainOSCmini Web Installer](https://shimez.github.io/ChainOSCmini/installer/)をChromeまたはEdgeで開きます。
2. Chain DualKeyの電源スイッチを中央の`OFF/USB`にします。
3. USBケーブルでPCへ接続します。
4. `Install ChainOSCmini`を押してシリアルポートを選択します。
5. 画面の案内に従って書き込みます。

ポートが表示されない場合は、KEY1を押しながらUSBを接続してダウンロードモードを試します。

## 2. LEDで接続状態を確認する

| LED表示 | 状態 |
|---|---|
| 紫でゆっくり点滅 | AP Mode（Wi-Fi設定待ち） |
| 青でゆっくり点滅 | Wi-Fiへ接続中 |
| 青で常時点灯 | Wi-Fi接続済み |
| オレンジ | 対応するDualKey本体キーを押している状態 |

キーを離すと、その時点のWi-Fi状態を示す色へ戻ります。

ファームウェアを書き込んだ直後、Wi-Fiが未設定の場合は紫の点滅になります。これは正常な動作で、次の手順でWi-Fi設定を待っている状態です。

## 3. Wi-Fiを設定する

1. `ChainOSCmini-Setup`へ接続します。
2. パスワード`12345678`を入力します。
3. ポータルが開かない場合は`http://192.168.4.1/`を開きます。
4. 2.4 GHz帯Wi-FiのSSIDとパスワードを保存します。

## 4. VRChatでOSCを有効にする

```text
リングメニュー → オプション → OSC → 有効
```

## 5. PCのIPアドレスを確認する

WindowsでPowerShellまたはコマンドプロンプトを開きます。

```powershell
ipconfig
```

ChainOSCminiと同じネットワークに接続したWi-FiまたはEthernetアダプターの`IPv4 Address`を確認します。

## 6. ChainOSCminiの設定画面を開く

1. `http://chainoscmini.local/`をブラウザーで開きます。
2. 開けない場合はWindows PowerShellを起動し、`Resolve-DnsName chainoscmini.local`を実行します。結果に表示されたIPアドレスをブラウザーへ入力して設定画面を開きます。
3. `OSC Target`へPCのIPv4アドレスとポート`9000`を入力します。

## 7. Keyプリセットを入手する

ChainOSCmini 0.9.1は、M5ChainOSCと共通のKeyプリセット形式を使用します。[M5ChainOSC Device Presets](https://github.com/shimez/M5ChainOSC/tree/main/presets/key)からKey用JSONをダウンロードできます。

例：

- `key-vrchat-voice-control.json`: マイクのON／OFF
- `key-vrchat-camera-controls.json`: 配信用カメラ設定の切り替え
- `key-vrchat-afk-control.json`: AFKモードのON／OFF

## 8. プリセットをインポートする

1. 設定対象のDualKey KEY1／KEY2またはChain Keyを確認します。
2. Keyカード右上の`…`を押します。
3. `Import Preset (JSON)（プリセットをインポート）`を選択します。
4. Key用JSONを選び、確認画面で実行します。
5. インポート後の設定がカードへ反映されたことを確認します。

プリセットはインポート時に保存されます。ほかの画面項目も変更した場合は`Save All Settings`を押してください。

## 9. 動作を確認する

VRChatが起動しOSCが有効な状態で、プリセットを適用したKeyを操作します。

届かない場合は、PCのIPv4アドレス、ポート`9000`、Windows Defender Firewall、VRChatのOSC設定を確認してください。
