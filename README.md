# ChainOSCmini

このプロジェクトのソフトウェア、Webサイト、ドキュメントは、OpenAI Codexとの協働により制作されています。

This project's software, website, and documentation are created in collaboration with OpenAI Codex.

M5Stack Chain DualKeyを使い、本体の2つのキーや左右に接続したM5Stack Chainデバイスの操作をOSCメッセージとして送信するファームウェアです。ブラウザーから送信先やデバイスごとの動作を設定でき、設定はDualKey本体のキーまたはChainデバイスのUID単位で本体へ保存されます。

主にVRChatのアバターパラメーター操作を想定していますが、OSCを受信できるアプリケーションで利用できます。

[ChainOSCシリーズポータル](https://shimez.github.io/ChainOSC/)

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではなく、同社との提携または承認を示すものではありません。

## Version 1.4.2

AP Modeのキャプティブポータルからも、確認後にLittleFSとNVSの全設定を削除して再起動できるようにしました。

## Version 1.4.0

Web UIの最下段から、確認後にLittleFSとNVSの全設定を削除して本体を再起動できるようにしました。

## Version 1.3.0

Wi-Fi認証情報、OSC送信先、Web UI言語の保存先をNVSからLittleFSへ移行しました。旧NVS設定は初回起動時に自動移行され、検証済み一時ファイルからの原子的な置換で保存されます。

## Version 1.2.0

Device Preset Import Error Registry v1へ完全対応し、JSON構文、必須項目、JSON型、OSC設定、Sequence、デバイス固有値・範囲、保存失敗のエラーコードと日英メッセージをChainOSCシリーズで統一しました。不正なプリセットは既存設定を変更せず拒否します。

## Version 1.1.1

PlatformIOおよびGitHub Actionsで生成するファームウェアのUSB CDCを起動時から有効化し、Web Installer版でもUSBシリアルログを確認できるようにしたメンテナンスリリースです。

## Version 1.1.0

デバイス設定の保存先をNVSからLittleFSへ移行しました。保存済みの旧NVS設定は初回読込時にLittleFSへ自動移行されます。UID全体をファイル名に使用し、本体キーは`Key1.json`／`Key2.json`として保存します。保存時のファイルサイズとLittleFSの使用量・空き容量はシリアルログで確認できます。

## Version 1.0.1

OSC送信先の表記をほかのChainOSCシリーズと統一したメンテナンスリリースです。

## Version 1.0.0

ChainOSCminiとして予定していた基本機能を備えた最初の安定版です。M5ChainOSCとの設定・プリセット互換性を重視しています。

- EncoderのAbsolute／Increment回転値をOSC送信
- EncoderクリックのPress / ReleaseとSequenceに対応
- Encoder設定をUID単位で保存・復元
- M5ChainOSC互換のEncoderプリセットをエクスポート／インポート
- Angleの8-bit／12-bit入力、Deadband、出力範囲・型設定に対応
- Angle設定をUID単位で保存し、M5ChainOSC互換プリセットを共有
- ToFの有効距離、Deadband、出力方向・範囲・型設定に対応
- ToF設定をUID単位で保存し、M5ChainOSC互換プリセットを共有
- JoystickのX/Y軸、Deadband、反転、出力範囲・型、クリック設定に対応
- GPIO47/GPIO48側に接続したJoystickはX軸・Y軸の正負を自動反転

詳しい変更内容は[変更履歴](CHANGELOG.md)を参照してください。

## 主な機能

- DualKey本体キー、Chain Key、Chain Encoder、Chain Angle、Chain ToF、Chain JoystickからOSCメッセージを直接送信
- 左右両方のChainポートへ接続したデバイスを個別に認識
- Press / ReleaseとSequenceに対応
- 1キーあたり最大8件のOSCメッセージとFloat／Int／String型を設定可能
- Chain Key／Encoder／Angle／ToF／Joystickの設定をUID単位で保存し、抜き差しや左右移動後も復元
- ブラウザーから英語／日本語で設定可能
- 全体設定とデバイスプリセットのJSONエクスポート／インポートに対応
- M5ChainOSCとKey／Encoder／Angle／ToF／Joystickプリセットを共有可能
- AP ModeとキャプティブポータルによるWi-Fi初期設定
- Arduino IDE、PlatformIO、Web Installerに対応

## OSC Address

以下は初期設定で使用するOSC Addressです。Address、値、型はWeb UIから用途に合わせて変更できます。

| 入力 | OSC Address | 値 |
|---|---|---|
| DualKey KEY1 | `/chainoscmini/dualkey/key1` | 押した時 `1`、離した時 `0` |
| DualKey KEY2 | `/chainoscmini/dualkey/key2` | 押した時 `1`、離した時 `0` |
| Chain Key | `/chainoscmini/chain/key/<UID>` | 押した時 `1`、離した時 `0` |
| Chain Encoder | `/avatar/parameters/Encoder` | 回転値 |
| Chain Angle | `/avatar/parameters/Angle` | 角度値 |
| Chain ToF | `/avatar/parameters/ToF` | 距離の変換値 |
| Chain Joystick X/Y | `/avatar/parameters/JoyX`／`JoyY` | スティック位置の変換値 |

例えば次のように設定できます。

| 用途 | OSC Address | 設定値 |
|---|---|---|
| VRChatのマイクON／OFF | `/input/Voice` | 押した時 `1`／離した時 `0` |
| VRChatのAFKモードON／OFF | `/input/AFKToggle` | 押した時 `1`／離した時 `0` |
| EncoderでVRChatカメラをズーム | `/usercamera/Zoom` | Absolute、出力範囲 `20`～`300` |
| AngleでVRChatカメラをズーム | `/usercamera/Zoom` | 12-bit、出力範囲 `20`～`300` |
| JoystickでVRChat内を移動 | `/input/Vertical`／`/input/Horizontal` | 出力範囲 `-1`～`1` |

詳細やその他の設定例は[M5ChainOSC Device Presets](https://github.com/shimez/M5ChainOSC/tree/main/presets)を参照してください。

## Wi-Fi初期設定

1. ChainOSCminiを起動します。
2. スマートフォンまたはPCから`ChainOSCmini-Setup`へ接続します。
3. Wi-Fiパスワードとして`12345678`を入力します。
4. キャプティブポータルが自動表示されない場合は、ブラウザーで`http://192.168.4.1/`を開きます。
5. ChainOSCminiとOSC送信先が利用する2.4 GHz帯Wi-FiのSSIDとパスワードを保存します。
6. 再起動後、`http://chainoscmini.local/`を開きます。

WindowsでmDNS名を確認する場合はPowerShellで次を実行できます。

```powershell
Resolve-DnsName chainoscmini.local
```

mDNS名で設定画面を開けない場合は、上記コマンドの結果に表示されたIPアドレスをブラウザーで開いてください。

> [!IMPORTANT]
> Web UIには認証機能がありません。ChainOSCminiは、家庭内LANなど信頼できるローカルネットワークで使用してください。イベント会場、ホテル、公共Wi-Fiなど、不特定の利用者が接続するネットワークでの使用は推奨しません。

> [!NOTE]
> ESP32-S3は2.4 GHz帯Wi-Fiを使用します。5 GHz専用のSSIDには接続できません。Wi-Fi認証情報はESP32-S3のNVSへ保存されます。ChainOSCminiは信頼できるローカルネットワークで使用してください。

電源スイッチに関係するGPIO7／GPIO8は設定も駆動も行いません。

起動から5秒後、USBシリアルへ次の情報を出力します。Arduino IDEのシリアルモニターがUSB再接続後に接続する時間を確保するため、起動直後には表示しません。

- ChainOSCminiのバージョン
- ビルド日時
- ESP32の型番、リビジョン、コア数
- CPUクロック
- リセット理由
- Flash、Sketch、Heap、PSRAMの容量
- 5秒間隔の稼働時間と空きHeap

起動診断は0.1.0と同様に起動から5秒後に表示します。キーとLEDは`appSetup()`完了後から動作します。

## Arduino IDE

1. Espressif Systemsの`esp32`ボードパッケージを導入します。
2. Arduino IDEのライブラリマネージャーから`M5Unified`、`Adafruit NeoPixel`、`M5Chain`、`ArduinoOSC`、`ArduinoJson`を導入します。
3. `ChainOSCmini.ino`を開きます。
4. ボードは`ESP32S3 Dev Module`を選択します。現在の`M5ChainDualKey`ボード定義ではアプリ領域が約1.25 MiBとなり、ChainOSCminiを格納できない場合があります。
5. `Flash Size`は`8MB`、`USB CDC On Boot`は`Enabled`を選択します。
6. `Partition Scheme`は、8 MB Flash向けで3 MiB以上のアプリ領域を持つ構成を選択します。表示される場合は`8M with spiffs (3MB APP/1.5MB SPIFFS)`を推奨します。名称が異なる場合は`Huge APP (3MB No OTA/1MB SPIFFS)`も使用できます。
7. 実機で確認したCOMポートを選び、書き込みます。
8. シリアルモニターを`115200 bps`で開きます。

## PlatformIO

プロジェクトルートで実行します。

```powershell
pio run
```

書き込みとシリアル監視は次のコマンドです。

```powershell
pio run --target upload
pio device monitor
```

COMポートを固定する場合は、ローカル環境だけで使用する`platformio_override.ini`などから`upload_port`と`monitor_port`を指定してください。個人環境のCOMポート番号はリポジトリへコミットしません。

## GitHub Actions／Web Installer

- `main`へプッシュすると、Actions画面からPlatformIOビルドを手動確認できます。
- `v1.0.0`のようなバージョンタグをプッシュすると、mergedバイナリとSHA-256を生成し、ドラフトReleaseを作成します。
- ドラフトReleaseを公開すると、GitHub PagesがReleaseのバイナリを取り込み、Web Installerを自動配信します。
- 公開URLは`https://shimez.github.io/ChainOSCmini/installer/`です。
- Web Installerでは、Release Assetを直接参照せずPagesと同じオリジンからファームウェアを配信します。

## Documentation

- [日本語ユーザーガイド](https://shimez.github.io/ChainOSCmini/user-guide/)
- [English User Guide](https://shimez.github.io/ChainOSCmini/en/user-guide/)
- [プリセット・クイックスタート](https://shimez.github.io/ChainOSCmini/quick-start-presets/)
- [Preset Quick Start](https://shimez.github.io/ChainOSCmini/en/quick-start-presets/)
- [Web Installer](https://shimez.github.io/ChainOSCmini/installer/)
- [変更履歴](CHANGELOG.md)
- [実機確認項目](docs/TESTING.md)

## Arduino IDE／PlatformIO共通化

実装本体は`src/app.cpp`の`appSetup()`／`appLoop()`です。

- Arduino IDEはルートの`ChainOSCmini.ino`をエントリーポイントとして使用します。
- Arduino IDEでは8 MB Flash向けの大容量Partition Schemeを選択します。PlatformIOは`default_8MB.csv`を使用します。
- PlatformIOは`CHAINOSCMINI_PLATFORMIO`を定義し、`src/main.cpp`からエントリーポイントを提供します。
- `src/`内のincludeは相対的なファイル名で統一します。

## 今後の予定

- その他のChainデバイスへの対応
- M5ChainOSCとの機能・UI・プリセット互換性の継続的な改善

## ライセンス

特に明記がない限り、ChainOSCminiの独自ソースコードおよびドキュメントは[MIT License](LICENSE)で提供されます。

Web Installerなどで配布するコンパイル済みファームウェアには、LGPL-2.1、LGPL-3.0、MITなど、各ライセンスで提供される第三者コンポーネントが含まれます。使用コンポーネント、バージョン、著作権表示および対応ソースへのリンクは[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)を参照してください。Arduino-ESP32に適用されるLGPL-2.1の本文は[licenses/LGPL-2.1.txt](licenses/LGPL-2.1.txt)、Adafruit NeoPixelに適用されるLGPL-3.0の本文は[licenses/LGPL-3.0.txt](licenses/LGPL-3.0.txt)に収録しています。

ChainOSCminiは個人開発の非公式プロジェクトです。MITライセンスは、M5Stack Technology Co., Ltd.、Adafruit Industriesその他の第三者の商標使用を許諾するものではありません。
