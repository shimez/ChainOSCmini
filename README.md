# ChainOSCmini

Chain DualKey上で動作するOSCコントローラーを目指す、開発中の非公式プロジェクトです。

> [!IMPORTANT]
> ChainOSCminiは個人が開発する非公式プロジェクトです。M5Stack Technology Co., Ltd.による公式製品ではなく、同社との提携または承認を示すものではありません。

## Version 0.1.0

実機の安全な起動確認を目的とした最小構成です。現段階ではWi-Fi、OSC、Web UI、設定保存、キー、LED、Chainポートの制御は実装していません。

起動するとUSBシリアルへ次の情報を出力します。

- ChainOSCminiのバージョン
- ビルド日時
- ESP32の型番、リビジョン、コア数
- CPUクロック
- リセット理由
- Flash、Sketch、Heap、PSRAMの容量
- 5秒間隔の稼働時間と空きHeap

GPIO割り当てを実機と一次資料で確認するまでは、周辺GPIOを駆動しないSafe Modeで動作します。

## Arduino IDE

1. Espressif Systemsの`esp32`ボードパッケージを導入します。
2. `ChainOSCmini.ino`を開きます。
3. ボードは暫定的に`ESP32S3 Dev Module`を選択します。
4. Flash Sizeは`8MB`、USB CDC On Bootは`Enabled`を選択します。
5. 実機で確認したCOMポートを選び、書き込みます。
6. シリアルモニターを`115200 bps`で開きます。

Chain DualKey向けの公式Arduinoボード定義が確認できた場合は、正式な設定へ置き換えます。

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

## 0.1.0の実機確認項目

- Arduino IDEでコンパイルできる
- PlatformIOでビルドできる
- 両方からChain DualKeyへ書き込める
- 再起動後にシリアルログが表示される
- Chip ModelとFlash容量が想定どおり表示される
- 1分以上動作させても再起動しない
- 空きHeapが継続的に減少しない

## 次の段階

実機確認後、キーとLEDのGPIOを検証した`0.2.0`へ進みます。その後、Wi-Fi、AP Mode、キャプティブポータル、mDNS、Web UI、OSC送信を段階的に移植します。

## License

このプロジェクトの独自コードは[MIT License](LICENSE)で公開します。
