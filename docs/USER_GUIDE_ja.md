---
layout: default
title: ChainOSCmini 日本語ユーザーガイド
permalink: /user-guide/
---

# ChainOSCmini 日本語ユーザーガイド

[English version](../en/user-guide/)

このガイドでは、Chain DualKeyへファームウェアを書き込んだ後の初期設定と、Web UIの使い方を説明します。

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではありません。

## 1. 初回のWi-Fi設定

1. Chain DualKeyの電源スイッチを中央の`OFF/USB`にしてUSB給電します。
2. スマートフォンまたはPCからSSID`ChainOSCmini-Setup`へ接続します。
3. パスワード`12345678`を入力します。
4. キャプティブポータルが開かない場合は`http://192.168.4.1/`を開きます。
5. 使用するWi-FiのSSIDとパスワードを保存します。
6. ChainOSCminiが再起動して保存したWi-Fiへ接続します。

Chain DualKeyのESP32-S3は**2.4 GHz帯Wi-Fiのみ**を使用します。5 GHz専用SSIDには接続できません。

### 本体LEDによる接続状態の確認

- 紫のゆっくりした点滅：AP Mode（Wi-Fi設定待ち）
- 青のゆっくりした点滅：Wi-Fiへ接続中
- 青の常時点灯：Wi-Fi接続済み
- オレンジ：対応する本体キーを押している状態

キーを離すと、その時点のWi-Fi状態を示す色へ戻ります。

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

> [!IMPORTANT]
> Web UIには認証機能がありません。ChainOSCminiは、家庭内LANなど信頼できるローカルネットワークで使用してください。イベント会場、ホテル、公共Wi-Fiなど、不特定の利用者が接続するネットワークでの使用は推奨しません。

## 3. Web UIの共通設定

### Language／言語

`English`または`日本語`を選択します。選択した言語は本体へ保存されます。

### System／システム

ファームウェアバージョン、現在のIPアドレス、mDNSアドレスを表示します。

### WiFi

現在のIPアドレスを表示します。`Delete WiFi Settings（Wi-Fi設定を削除）`を押すと認証情報を消去し、次回起動時に設定用アクセスポイントへ戻ります。

### Settings Backup & Restore／設定のバックアップと復元

- `Export Settings (JSON)`: OSC送信先、UI言語、本体キー、保存済みChain Key／Encoder／Angle／ToF／Joystickをエクスポートします。
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

DualKey本体のKEY1／KEY2とChain Keyを設定できます。

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

## 6. Encoderの設定

### Encoder Rotation（エンコーダー回転）

| 項目 | 意味 |
| --- | --- |
| `OSC Address（OSCアドレス）` | 回転値の送信先です。 |
| `Mode（モード）` | `Absolute（絶対値）`は現在の回転カウント、`Increment（増分）`は直前からの回転量を使用します。 |
| `Abs In Min（絶対値入力の最小値）` | Absoluteで`Out Min`に対応させる入力位置です。Incrementでは非表示になります。 |
| `Abs In Max（絶対値入力の最大値）` | Absolute入力範囲の終端です。到達すると先頭へ循環します。Incrementでは非表示になります。 |
| `Inc Scale（増分倍率）` | Incrementの回転差分へ掛ける倍率です。逆方向では負の値になります。 |
| `Out Min（出力最小値）` | 送信値の下限です。 |
| `Out Max（出力最大値）` | 送信値の上限です。 |
| `Out Type（出力の型）` | `Float`、四捨五入した`Int`、数値を文字列化した`String`を選択します。 |

Absoluteでは、エンコーダーの内部カウントに一定範囲を1周として設定し、`Out Min`～`Out Max`へ変換します。初期値の`Abs In Min = 0`、`Abs In Max = 20`、`Out Min = 0`、`Out Max = 1`では、カウント`0`が`0.00`、`5`が`0.25`、`10`が`0.50`、`15`が`0.75`、`20`で`0.00`へ戻ります。

エンコーダーには物理的な最小位置と最大位置がないため、`Abs In Max`は「出力範囲が一周するまでのカウント数」と考えると分かりやすくなります。値を小さくすると少ない回転で一周し、大きくすると細かく調整できます。現在の実装では`Abs In Max`に到達した時点で`Abs In Min`へ戻るため、上記の例では`Out Max`そのものではなく`0.95`付近まで送信してから`0.00`へ戻ります。

### Encoder Click（エンコーダークリック）

緑色の帯で囲まれたクリック設定はKeyと同じです。`Press / Release`では合計8メッセージまで設定でき、`Sequence`では押すたびに値が進みます。回転設定とクリック設定は独立して使用できます。

## 7. Angleの設定

| 項目 | 意味 |
| --- | --- |
| `OSC Address（OSCアドレス）` | 角度値の送信先です。 |
| `Resolution（分解能）` | `12-bit`は0～4095、`8-bit`は0～255の入力値を使用します。 |
| `Deadband（不感帯）` | 前回送信した入力値との差がこの値以上になったときだけ送信します。小さな揺れによる連続送信を抑えます。 |
| `Out Min（出力最小値）` | センサー入力の最小値に対応する出力値です。 |
| `Out Max（出力最大値）` | センサー入力の最大値に対応する出力値です。 |
| `Out Type（出力の型）` | `Float`、四捨五入した`Int`、数値を文字列化した`String`を選択します。 |

初回の読み取りではOSCを送信せず、以後はDeadband以上の変化があったときに入力範囲を`Out Min`～`Out Max`へ変換して送信します。通常は滑らかな値を得られる`12-bit`を推奨します。例えば12-bit、`Out Min = 0`、`Out Max = 1`なら、中央付近の入力値はおおむね`0.5`として送信されます。

## 8. ToFの設定

| 項目 | 意味 |
| --- | --- |
| `OSC Address（OSCアドレス）` | 距離を変換した値の送信先です。 |
| `Deadband (mm)（不感帯）` | 前回送信した距離との差がこの値以上になったときに送信します。 |
| `Maximum Distance (mm)（最大距離）` | 使用する距離範囲の上限です。31～2000 mmで設定します。 |
| `Direction（出力方向）` | `Near → Out Min / Far → Out Max（近い → 出力最小値／遠い → 出力最大値）`または`Near → Out Max / Far → Out Min（近い → 出力最大値／遠い → 出力最小値）`を選びます。 |
| `Out Min／Out Max（出力最小値／最大値）` | 有効距離範囲を変換する出力範囲です。 |
| `Out Type（出力の型）` | `Float`または四捨五入した`Int`を選択します。 |

有効範囲は30 mm以上、Maximum Distance未満です。手や対象物が範囲外へ移動すると最大値を送信するのではなく、OSC送信を停止します。再び範囲内へ入ると最初の有効値から送信を再開します。

例えば`Maximum Distance = 500`、`Out Min = 0`、`Out Max = 1`、`Near → Out Max / Far → Out Min`なら、30 mm付近で約`1`、500 mmへ近づくにつれて`0`へ変化し、500 mm以上では送信を停止します。

## 9. Joystickの設定

| 項目 | 意味 |
| --- | --- |
| `X Address（X軸OSCアドレス）` | X軸の送信先です。 |
| `Y Address（Y軸OSCアドレス）` | Y軸の送信先です。 |
| `Invert X / Y（X軸／Y軸反転）` | 選択した軸の正負を反転します。 |
| `Deadband（不感帯）` | 前回送信時の生入力値との差がこの値以上になった軸だけを送信します。生入力は概ね`-127`～`127`で、実装上の最小値は`1`です。 |
| `Out Min / Out Max（出力最小値／最大値）` | 入力-127～127を変換する出力範囲です。 |
| `Out Type（出力の型）` | `Float`、`Int`、`String`を選択します。 |

GPIO47/GPIO48側に接続したJoystickは、左右の物理的な向きを補正するため、X軸とY軸の正負が常に自動反転されます。設定操作は不要です。Web UIの`X軸反転`／`Y軸反転`は、この自動補正後の値へ追加で適用されます。

例えば`Out Min = -1`、`Out Max = 1`なら、スティック中央付近は`0`、両端はおおむね`-1`と`1`になります。X軸とY軸はそれぞれ別のOSC Addressへ送信されます。

Joystick Click（ジョイスティッククリック）のPress / Release、最大8メッセージ、SequenceはKeyと同じ仕様です。Joystick設定とプリセットはM5ChainOSCと互換性があります。

## 10. OSC入力規則

- OSC Addressは`/`から始め、最大192バイトです。
- 空白と`# * , ? [ ] { }`は使用できません。
- Valueは最大128バイトです。
- FloatとIntには選択した型として解釈できる数値を入力します。

## 11. デバイスプリセット

各Key／Encoder／Angle／ToF／Joystick右上の`…`から操作します。

- `Export Preset (JSON)`: UIDとDevice Nameを含まないデバイス設定を保存します。
- `Import Preset (JSON)`: 同じ種類のデバイスへプリセットを適用し、直ちにストレージへ保存します。

形式はM5ChainOSCと共通の`ChainOSC-device-preset`です。Key、Encoder、Angle、ToF、Joystickのプリセットを両プロジェクト間で共有できます。旧`M5ChainOSC-device-preset`形式もインポートできます。

## 12. 接続中／保存済みデバイス

- DualKey本体キーは常に接続中として表示されます。
- Chain Key／Encoder／Angle／ToF／Joystickの設定はUID単位で保存され、抜き差し、接続順変更、左右ポート移動後も復元されます。
- 取り外したChainデバイスは`Saved Device Settings（保存済みデバイス設定）`へ表示されます。
- `Delete Settings（設定を削除）`でUIDに保存した設定を削除できます。

## 13. トラブルシューティング

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

## 14. 現在の対応範囲

現在はDualKey本体キー、Chain Key、Chain Encoder、Chain Angle、Chain ToF、Chain Joystickに対応しています。その他のChainデバイスは今後対応予定です。
