# ChainOSCmini

Chain DualKey上で動作するOSCコントローラーを目指す、開発中の非公式プロジェクトです。

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではなく、同社との提携または承認を示すものではありません。

## Version 0.7.0

0.6.0までのDualKey本体・左右Chain・ネットワーク機能に、基本的なOSC送信を追加した開発版です。UID単位の詳細設定UIはまだ実装していません。

- KEY1（GPIO0）とKEY2（GPIO17）の押下・解放を検出
- 20msのデバウンス
- 2個のWS2812Bを通常時は暗い青、押下中はオレンジで表示
- キーイベントをUSBシリアルへ出力
- LEDデータはGPIO21、LED電源制御はGPIO40を使用
- GPIO5（RX）／GPIO6（TX）側を`G5_G6`として初期化
- GPIO47（RX）／GPIO48（TX）側を`G47_G48`として初期化
- 2つのM5ChainインスタンスとUARTを独立管理
- 各ポートを2秒間隔で個別に走査
- 接続デバイスのID、種類、12バイトUIDをシリアル表示
- デバイスの接続、取り外し、交換、台数・順序変更を検出
- 抜き差し中の一時的な通信失敗では、直前の正常な列挙結果を維持
- 複数のChain Keyを25ms間隔でポーリング
- Chain Keyの押下・解放をChain IDとUID付きでシリアル表示
- Chain Keyは通常時に青、押下中にオレンジで点灯
- 抜き差しや順序変更後に入力とLEDを再初期化
- M5Chain内部のボタン通知キューを消費し、連続操作時のHeap低下を防止
- ログへポート名を付加し、両側に存在する同じChain IDを区別
- 一方の切断・タイムアウト時も、反対側の状態を維持
- Wi-Fi設定をESP32-S3のNVSへ保存
- M5ChainOSCと同じ起動順序でM5Unified、Wi-Fi、DualKey／Chainの順に初期化
- 保存済みWi-Fiへ最大15秒間接続し、失敗時は設定用APへ移行
- 未設定または15秒間接続できない場合は設定用APを開始
- キャプティブポータルからSSIDとパスワードを設定
- 接続後は`http://chainoscmini.local/`またはIPアドレスで状態を確認
- Web画面から保存済みWi-Fi設定を削除可能
- 実機検証結果に基づきWi-Fi送信出力を2 dBmへ制限
- Web画面からOSC送信先のホスト名／IPアドレスとUDPポートを設定
- DualKey本体のKEY1／KEY2から押下時`1`、解放時`0`を送信
- Chain KeyからUID別のOSC Addressへ押下時`1`、解放時`0`を送信

## OSC Address

| 入力 | OSC Address | 値 |
|---|---|---|
| DualKey KEY1 | `/chainoscmini/dualkey/key1` | 押下 `1`、解放 `0` |
| DualKey KEY2 | `/chainoscmini/dualkey/key2` | 押下 `1`、解放 `0` |
| Chain Key | `/chainoscmini/chain/key/<UID>` | 押下 `1`、解放 `0` |

Chain Keyの`<UID>`には、ログに表示される24桁の16進数UIDが入ります。

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

mDNSでアクセスできない場合は、シリアルログの`state=CONNECTED ip=...`に表示されたIPアドレスをブラウザーで開いてください。

> [!NOTE]
> ESP32-S3は2.4 GHz帯Wi-Fiを使用します。5 GHz専用のSSIDには接続できません。Wi-Fi認証情報はESP32-S3のNVSへ保存されます。本診断版は信頼できるローカルネットワークで使用してください。

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
2. Arduino IDEのライブラリマネージャーから`M5Unified`、`Adafruit NeoPixel`、`M5Chain`、`ArduinoOSC`を導入します。
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

## Arduino IDE／PlatformIO共通化

実装本体は`src/app.cpp`の`appSetup()`／`appLoop()`です。

- Arduino IDEはルートの`ChainOSCmini.ino`をエントリーポイントとして使用します。
- PlatformIOは`CHAINOSCMINI_PLATFORMIO`を定義し、`src/main.cpp`からエントリーポイントを提供します。
- `src/`内のincludeは相対的なファイル名で統一します。

## 0.7.0の実機確認項目

- Arduino IDEでコンパイルできる
- PlatformIOでビルドできる
- 両方からChain DualKeyへ書き込める
- 再起動後にシリアルログが表示される
- Chip ModelとFlash容量が想定どおり表示される
- 1分以上動作させても再起動しない
- 空きHeapが継続的に減少しない
- 起動すると2個のLEDが暗い青で点灯する
- KEY1を押している間、対応するLEDだけがオレンジになる
- KEY2を押している間、対応するLEDだけがオレンジになる
- 押下と解放がそれぞれ1回ずつシリアルへ表示される
- 同時押しを個別に検出できる
- キーを繰り返し操作しても再起動しない
- `G5_G6`側だけで従来どおり列挙・入力・LEDが動作する
- `G47_G48`側だけでも列挙・入力・LEDが動作する
- 左右それぞれにChain Keyを接続すると、ポート別に`CONNECTED`が表示される
- 列挙された台数、ID、種類、UIDが表示される
- デバイスを外すと`DISCONNECTED`が表示される
- 再接続すると同じUIDが表示される
- 複数デバイスの接続と順序変更を検出できる
- Chainの走査中もDualKey本体のキーとLEDが動作する
- Chain Keyを押すと、対応するUIDの`PRESSED`が1回表示される
- Chain Keyを離すと、対応するUIDの`RELEASED`が1回表示される
- 操作したChain Keyだけが青からオレンジへ変わる
- 複数のChain Keyを個別および同時に操作できる
- 抜き差しや接続順序変更後も、各UIDの入力を正しく取得できる
- 左右のChain Keyを同時に操作できる
- 両側に`id=1`が存在しても、ポート名とUIDで区別できる
- 一方だけを抜き差ししても、反対側の入力が継続する
- 同じChain Keyを反対側へ移してもUIDが変わらない
- Wi-Fi未設定時に`ChainOSCmini-Setup`が表示される
- APへ接続するとキャプティブポータルが開く
- 2.4 GHz帯Wi-Fiの設定を保存して再起動できる
- 再起動後に保存済みWi-Fiへ接続できる
- シリアルログへIPアドレスが表示される
- `http://chainoscmini.local/`またはIPアドレスで状態ページを開ける
- 状態ページにバージョン、IPアドレス、mDNS名が表示される
- Wi-Fi設定を削除するとAP Modeへ戻る
- Wi-Fi接続待ちやWebアクセス中もDualKeyと左右Chainが動作する
- Wi-Fi有効時も空きHeapが継続的に減少しない
- Web画面でOSC送信先を保存し、再起動後も復元される
- DualKey KEY1／KEY2の押下・解放でOSCの`1`／`0`を受信できる
- 左右のChain Keyを操作するとUID別Addressで`1`／`0`を受信できる
- Wi-Fi設定を削除してもOSC送信先設定が保持される

抜き差しの瞬間には`TIMEOUT`が一度表示されることがあります。次の走査で自動復帰し、正常な列挙結果を失わない設計です。

## 次の段階

実機確認後、DualKey本体とChainデバイスのUID単位設定、複数OSCメッセージ、M5ChainOSC互換プリセットの読み込みへ進みます。

## License

このプロジェクトの独自コードは[MIT License](LICENSE)で公開します。
