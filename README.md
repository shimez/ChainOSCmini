# ChainOSCmini

このプロジェクトのソフトウェア、Webサイト、ドキュメントは、OpenAI Codexとの協働により制作されています。

This project's software, website, and documentation are created in collaboration with OpenAI Codex.

M5Stack Chain DualKeyを使い、本体の2つのキーや左右に接続したM5Stack Chainデバイスの操作をOSCメッセージとして送信するファームウェアです。ブラウザーから送信先やデバイスごとの動作を設定でき、設定はDualKey本体のキーまたはChainデバイスのUID単位で本体へ保存されます。

主にVRChatのアバターパラメーター操作を想定していますが、OSCを受信できるアプリケーションで利用できます。

[ChainOSCシリーズポータル](https://shimez.github.io/ChainOSC/)

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではなく、同社との提携または承認を示すものではありません。

## 現在のバージョン

### v1.4.7

- Encoder v2のAmount／Direction設定とLegacyからの移行に対応
- Device PresetのEncoder v1／v2 Import／Exportに対応
- 設定ファイルがない新規Encoderはv2設定を初期設定として使用

過去の変更内容は[変更履歴](CHANGELOG.md)を参照してください。

## 主な機能

- DualKey本体キー、Chain Key、Chain Encoder、Chain Angle、Chain ToF、Chain JoystickからOSCメッセージを直接送信
- 左右両方のChainポートへ接続したデバイスを個別に認識
- Press / ReleaseとSequenceに対応
- 1キーあたり最大8件のOSCメッセージとFloat／Int／String型を設定可能
- Chain Key／Encoder／Angle／ToF／Joystickの設定をUID単位で保存し、抜き差しや左右移動後も復元
- ブラウザーから英語／日本語で設定可能
- 全体設定とデバイスプリセットのJSONエクスポート／インポートに対応
- 対応するChainOSCシリーズ製品とのデバイスプリセット共有
- AP ModeとキャプティブポータルによるWi-Fi初期設定
- Arduino IDE、PlatformIO、Web Installerに対応

## Documentation

- [日本語ユーザーガイド](https://shimez.github.io/ChainOSCmini/user-guide/)
- [English User Guide](https://shimez.github.io/ChainOSCmini/en/user-guide/)
- [クイックスタート](https://shimez.github.io/ChainOSCmini/quick-start/)
- [Quick Start](https://shimez.github.io/ChainOSCmini/en/quick-start/)
- [Web Installer](https://shimez.github.io/ChainOSCmini/installer/)
- [変更履歴](CHANGELOG.md)
- [実機確認項目](docs/TESTING.md)
- [ChainOSC共通仕様](https://github.com/shimez/ChainOSC)

## Device Preset対応

ChainOSCminiは、ChainOSCシリーズ共通の`ChainOSC-device-preset`形式に対応しています。

Encoder v1／v2を含む、Device TypeごとのschemaVersionに対応しています。

- Key、Encoder、Angle、ToF、Joystickのプリセットをエクスポート／インポート
- 対応する`deviceType`でデバイス種類を判定
- Key v1：M5ChainOSC、ChainOSCmini、ChainOSCnano、ChainOSCPad、ChainOSC for Windowsと共有
- Encoder v1：M5ChainOSC、ChainOSCmini、ChainOSCnano、ChainOSCPadと共有
- Encoder v2：M5ChainOSC、ChainOSCmini、ChainOSCnano、ChainOSCPadで共有
- Angle v1／ToF v1／Joystick v1：M5ChainOSC、ChainOSCmini、ChainOSCnano間で共有
- 共有可否はDevice Type、schemaVersion、各製品のImporter／Exporter対応に基づく
- UID、Device Name、接続ポートなど、インポート先固有の情報は含めない
- Import時は共通仕様に基づいてJSON構文、必須項目、JSON型、OSC設定、Sequence、デバイス固有値・範囲を検証
- 不正なプリセットを拒否した場合、既存設定を変更しない

Device Preset v1／v2の詳細な仕様、JSON Schema、fixture、Error Registryは、[ChainOSC共通仕様](https://github.com/shimez/ChainOSC)を参照してください。

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
| EncoderでVRChatカメラをズーム | `/usercamera/Zoom` | Amount、出力範囲 `20`～`300` |
| AngleでVRChatカメラをズーム | `/usercamera/Zoom` | 12-bit、出力範囲 `20`～`300` |
| JoystickでVRChat内を移動 | `/input/Vertical`／`/input/Horizontal` | 出力範囲 `-1`～`1` |

詳細やその他の設定例は[ChainOSC Device Presets](https://github.com/shimez/ChainOSC/tree/main/presets)を参照してください。

## Wi-Fi初期設定

1. ChainOSCminiを起動します。
2. スマートフォンまたはPCから`ChainOSCmini-Setup`へ接続します。
3. Wi-Fiパスワードとして`12345678`を入力します。
4. キャプティブポータルが自動表示されない場合は、ブラウザーで`http://192.168.4.1/`を開きます。
5. ChainOSCminiとOSC送信先が利用する2.4 GHz帯Wi-FiのSSIDとパスワードを保存します。
6. 再起動後、`http://chainoscmini.local/`を開きます。

WindowsでmDNS名を確認する場合:

```powershell
Resolve-DnsName chainoscmini.local
```

mDNS名で設定画面を開けない場合は、上記コマンドの結果に表示されたIPアドレスをブラウザーで開いてください。

> [!IMPORTANT]
> Web UIには認証機能がありません。ChainOSCminiは、家庭内LANなど信頼できるローカルネットワークで使用してください。イベント会場、ホテル、公共Wi-Fiなど、不特定の利用者が接続するネットワークでの使用は推奨しません。

> [!NOTE]
> ESP32-S3は2.4 GHz帯Wi-Fiを使用します。5 GHz専用のSSIDには接続できません。Wi-Fi認証情報、OSC送信先、Web UI言語はLittleFSへ保存されます。旧NVS設定が存在する場合は初回起動時に移行されます。

電源スイッチに関係するGPIO7／GPIO8は設定も駆動も行いません。

## シリアルログ

USBシリアルへ次の情報を出力します。

- ChainOSCminiのバージョン
- ビルド日時
- ESP32の型番、リビジョン、コア数
- CPUクロック
- リセット理由
- Flash、Sketch、Heap、PSRAMの容量
- 5秒間隔の稼働時間と空きHeap
- LittleFSのファイルサイズ、総容量、使用量、空き容量

シリアルモニターは`115200 bps`で開いてください。

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

COMポートを固定する場合は、`platformio_override.ini`などローカル専用の設定で`upload_port`と`monitor_port`を指定してください。

## GitHub Actions／Web Installer

- `main`へプッシュすると、Actions画面からPlatformIOビルドを確認できます。
- `vX.Y.Z`形式のバージョンタグをプッシュすると、mergedバイナリとSHA-256を生成し、ドラフトReleaseを作成します。
- ドラフトReleaseを公開すると、GitHub PagesがReleaseのバイナリを取り込み、Web Installerを配信します。
- Web Installerの公開URLは`https://shimez.github.io/ChainOSCmini/installer/`です。

## Arduino IDE／PlatformIO共通化

実装本体は`src/app.cpp`の`appSetup()`／`appLoop()`です。

- Arduino IDEはルートの`ChainOSCmini.ino`をエントリーポイントとして使用します。
- Arduino IDEでは8 MB Flash向けの大容量Partition Schemeを選択します。PlatformIOは`default_8MB.csv`を使用します。
- PlatformIOは`CHAINOSCMINI_PLATFORMIO`を定義し、`src/main.cpp`からエントリーポイントを提供します。
- `src/`内のincludeは相対的なファイル名で統一します。

## ライセンス

特に明記がない限り、ChainOSCminiの独自ソースコードおよびドキュメントは[MIT License](LICENSE)で提供されます。

Web Installerなどで配布するコンパイル済みファームウェアには、LGPL-2.1、LGPL-3.0、MITなど、各ライセンスで提供される第三者コンポーネントが含まれます。使用コンポーネント、バージョン、著作権表示および対応ソースへのリンクは[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)を参照してください。Arduino-ESP32に適用されるLGPL-2.1の本文は[licenses/LGPL-2.1.txt](licenses/LGPL-2.1.txt)、Adafruit NeoPixelに適用されるLGPL-3.0の本文は[licenses/LGPL-3.0.txt](licenses/LGPL-3.0.txt)に収録しています。

ChainOSCminiは個人開発の非公式プロジェクトです。MITライセンスは、M5Stack Technology Co., Ltd.、Adafruit Industriesその他の第三者の商標使用を許諾するものではありません。
