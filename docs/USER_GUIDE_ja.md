---
layout: default
title: ChainOSCmini 日本語ユーザーガイド
permalink: /user-guide/
---

# ChainOSCmini 日本語ユーザーガイド

[English version](../en/user-guide/)

このガイドでは、Chain DualKeyへファームウェアを書き込んだ後の初期設定と、Web UIの使い方を説明します。

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではありません。Web UIには認証機能がないため、家庭内LANなど信頼できるローカルネットワークで使用してください。

## 1. 初回のWi-Fi設定

1. Chain DualKeyの電源スイッチを中央の`OFF/USB`にしてUSB給電します。
2. スマートフォンまたはPCからSSID`ChainOSCmini-Setup`へ接続します。
3. パスワード`12345678`を入力します。
4. キャプティブポータルが開かない場合は`http://192.168.4.1/`を開きます。
5. 使用するWi-FiのSSIDとパスワードを保存します。
6. ChainOSCminiが再起動して保存したWi-Fiへ接続します。

Chain DualKeyのESP32-S3は**2.4 GHz帯Wi-Fiのみ**を使用します。5 GHz専用SSIDには接続できません。

## 2. 設定画面を開く

同じネットワーク上のブラウザーで次を開きます。

```text
http://chainoscmini.local/
```

Windowsで`.local`アドレスを開けない場合は、PowerShellを起動して次を実行します。

```powershell
Resolve-DnsName chainoscmini.local
```

結果に表示されたIPアドレスを確認し、ブラウザーのアドレス欄へ入力してアクセスします。

```text
http://192.168.x.x/
```

スマートフォンで`chainoscmini.local`を開き、Web UIのシステム情報からIPアドレスを確認する方法もあります。

## 3. Web UIの共通設定

### Language／言語

`English`または`日本語`を選択します。選択した言語は本体へ保存されます。

### System／システム

ファームウェアバージョン、現在のIPアドレス、mDNSアドレスを表示します。

### WiFi

現在のIPアドレスを表示します。`Delete WiFi Settings（Wi-Fi設定を削除）`を押すと認証情報を消去し、次回起動時に設定用アクセスポイントへ戻ります。

### Settings Backup & Restore／設定のバックアップと復元

- `Export Settings (JSON)`: OSC送信先、UI言語、本体キー、保存済みChain Keyをエクスポートします。
- `Import Settings (JSON)`: ChainOSCmini全体設定JSONを復元します。

JSONには`ChainOSCmini-settings`、schemaVersion、保存時のファームウェアバージョンが記録されます。Wi-FiのSSIDとパスワードは含まれません。

### OSC Target／OSC送信先

| 項目 | 意味 |
| --- | --- |
| `Host or IP address（ホスト名またはIPアドレス）` | OSCを受信するPCやアプリのアドレスです。 |
| `UDP Port（UDPポート）` | OSC受信ポートです。VRChatの標準受信ポートは通常`9000`です。 |

設定変更後は画面下部へ追従する`Save All Settings（すべての設定を保存）`を押します。

## 4. VRChatでOSCを有効にする

```text
リングメニュー → オプション → OSC → 有効
```

ChainOSCminiとVRChatを実行するPCを、相互に通信できる同じネットワークへ接続してください。

## 5. Keyの設定

0.9.0ではDualKey本体のKEY1／KEY2とChain Keyを設定できます。

| 項目 | 意味 |
| --- | --- |
| `Device Name（デバイス名）` | Keyを識別する名前です。 |
| `Key Mode（キーモード）` | `Press / Release`または`Sequence`を選択します。 |

### Press / Release

- PressとReleaseの合計で最大8メッセージです。
- 各メッセージにOSC Address、Type、Valueを設定します。
- 上下矢印で送信順を変更できます。
- `Delete（削除）`で0件にすると、その操作では送信しません。
- Typeは`Float`、`Int`、`String`を選択できます。

### Sequence

Keyを押すたびにStartからStepずつ値を進め、Endを越えるとStartへ戻ります。Keyを離した時は送信しません。

| 項目 | 意味 |
| --- | --- |
| `OSC Address（OSCアドレス）` | Sequence値の送信先です。 |
| `Start（開始値）` | 最初に送る値です。 |
| `End（終了値）` | Sequenceの終端値です。 |
| `Step（増減量）` | 押すたびに加算する量です。 |
| `Type（型）` | `Float`または`Int`を使用します。 |

## 6. OSC入力規則

- OSC Addressは`/`から始め、最大192バイトです。
- 空白と`# * , ? [ ] { }`は使用できません。
- Valueは最大128バイトです。
- FloatとIntには選択した型として解釈できる数値を入力します。

## 7. デバイスプリセット

各Key右上の`…`から操作します。

- `Export Preset (JSON)`: UIDとDevice Nameを含まないKey設定を保存します。
- `Import Preset (JSON)`: 選択したKeyへプリセットを適用し、直ちにストレージへ保存します。

形式はM5ChainOSCと共通の`ChainOSC-device-preset`です。M5ChainOSCのKeyプリセットをChainOSCminiへ読み込み、ChainOSCminiで作成したKeyプリセットをM5ChainOSCへ読み込めます。

## 8. 接続中／保存済みデバイス

- DualKey本体キーは常に接続中として表示されます。
- Chain Keyの設定はUID単位で保存され、抜き差し、接続順変更、左右ポート移動後も復元されます。
- 取り外したChain Keyは`Saved Device Settings（保存済みデバイス設定）`へ表示されます。
- `Delete Settings（設定を削除）`でUIDに保存した設定を削除できます。

## 9. トラブルシューティング

### 設定画面を開けない

- 同じWi-Fiへ接続されているか確認します。
- WindowsではPowerShellで`Resolve-DnsName chainoscmini.local`を実行し、表示されたIPアドレスでアクセスします。
- 5 GHz専用SSIDを使用していないか確認します。

### OSCが届かない

- OSC送信先のIPアドレスとポートを確認します。
- VRChat内でOSCを有効にします。
- PCのファイアウォールを確認します。
- Addressが`/`から始まることを確認します。

### JSONをインポートできない

- 全体設定には`ChainOSCmini-settings`が必要です。
- プリセットには`ChainOSC-device-preset`または旧`M5ChainOSC-device-preset`が必要です。
- 全体設定とプリセットを逆のインポート欄へ指定していないか確認します。
- 異なるデバイス種類、破損JSON、9件以上のメッセージ、不正なAddressや値は拒否されます。

## 10. 現在の対応範囲

Version 0.9.0はDualKey本体キーとChain Keyに対応しています。Encoder、Joystick、Angle、ToFなど、その他のChainデバイスは今後対応予定です。
