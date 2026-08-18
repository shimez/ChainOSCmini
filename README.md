# ChainOSCmini

Chain DualKey上で動作するOSCコントローラーを目指す、開発中の非公式プロジェクトです。

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではなく、同社との提携または承認を示すものではありません。

## Version 0.5.0

Chain DualKey本体のキー／RGB LEDに加え、左右2系統のChainポートを同時利用できることを実機確認するための診断版です。Wi-Fi、OSC、Web UI、設定保存はまだ実装していません。

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
2. Arduino IDEのライブラリマネージャーから`Adafruit NeoPixel`と`M5Chain`を導入します。
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

## 0.5.0の実機確認項目

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

抜き差しの瞬間には`TIMEOUT`が一度表示されることがあります。次の走査で自動復帰し、正常な列挙結果を失わない設計です。

## 次の段階

実機確認後、Wi-Fi、AP Mode、キャプティブポータル、mDNSなど、ネットワーク基盤の段階的な移植へ進みます。

## License

このプロジェクトの独自コードは[MIT License](LICENSE)で公開します。
