# ChainOSCmini

Chain DualKey上で動作するOSCコントローラーを目指す、開発中の非公式プロジェクトです。

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではなく、同社との提携または承認を示すものではありません。

## Version 0.9.1

0.9.0の機能に、LEDによるWi-Fi状態表示を追加した開発版です。

- AP Modeでは2個のLEDを紫でゆっくり点滅
- Wi-Fi接続中は2個のLEDを青でゆっくり点滅
- Wi-Fi接続後は2個のLEDを青で常時点灯
- キー押下中は対象LEDをオレンジで表示し、解放後は現在のWi-Fi状態色へ復帰

詳しい変更内容は[変更履歴](CHANGELOG.md)を参照してください。

## 主な機能

- DualKey本体キーとChain KeyからOSCメッセージを直接送信
- 左右両方のChainポートへ接続したデバイスを個別に認識
- Press / ReleaseとSequenceに対応
- 1キーあたり最大8件のOSCメッセージとFloat／Int／String型を設定可能
- Chain Keyの設定をUID単位で保存し、抜き差しや左右移動後も復元
- ブラウザーから英語／日本語で設定可能
- 全体設定とデバイスプリセットのJSONエクスポート／インポートに対応
- M5ChainOSCとKeyプリセットを共有可能
- AP ModeとキャプティブポータルによるWi-Fi初期設定
- Arduino IDE、PlatformIO、Web Installerに対応

## OSC Address

以下は初期設定で使用するOSC Addressです。Address、値、型はWeb UIから用途に合わせて変更できます。

| 入力 | OSC Address | 値 |
|---|---|---|
| DualKey KEY1 | `/chainoscmini/dualkey/key1` | 押した時 `1`、離した時 `0` |
| DualKey KEY2 | `/chainoscmini/dualkey/key2` | 押した時 `1`、離した時 `0` |
| Chain Key | `/chainoscmini/chain/key/<UID>` | 押した時 `1`、離した時 `0` |

Chain Keyの`<UID>`には、ログに表示される24桁の16進数UIDが入ります。

例えば次のように設定できます。

| 用途 | OSC Address | 設定値 |
|---|---|---|
| VRChatのマイクON／OFF | `/input/Voice` | 押した時 `1`／離した時 `0` |
| VRChatのAFKモードON／OFF | `/input/AFKToggle` | 押した時 `1`／離した時 `0` |

そのほかの設定例は[M5ChainOSC Device Presets](https://github.com/shimez/M5ChainOSC/tree/main/presets)を参照してください。

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
4. ボードは`M5ChainDualKey`を選択します。表示されない場合は暫定的に`ESP32S3 Dev Module`を選択します。
5. `ESP32S3 Dev Module`の場合、Flash Sizeは`8MB`、USB CDC On Bootは`Enabled`を選択します。
6. 実機で確認したCOMポートを選び、書き込みます。
7. シリアルモニターを`115200 bps`で開きます。

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
- `v0.9.1`タグをプッシュすると、mergedバイナリとSHA-256を生成し、ドラフトReleaseを作成します。
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
- PlatformIOは`CHAINOSCMINI_PLATFORMIO`を定義し、`src/main.cpp`からエントリーポイントを提供します。
- `src/`内のincludeは相対的なファイル名で統一します。

## 今後の予定

- Encoder、Joystick、Angle、ToFなど、その他のChainデバイスへの対応
- M5ChainOSCとの機能・UI・プリセット互換性の継続的な改善

## License

このプロジェクトの独自コードは[MIT License](LICENSE)で公開します。
