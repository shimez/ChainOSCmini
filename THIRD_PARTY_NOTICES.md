# Third-Party Notices

ChainOSCminiの独自部分は、ルートの[MIT License](LICENSE)で提供されます。
コンパイル済みファームウェアには、以下の第三者ソフトウェアが含まれます。各コンポーネントには、それぞれのライセンスが適用されます。

以下のバージョンは、PlatformIOで本文書を整備した時点の解決結果です。`platformio.ini`で範囲指定されている依存関係については、その制約も併記しています。個別のReleaseに対応する正確な依存関係は、対象タグをチェックアウトしてPlatformIOで確認してください。

## ビルド基盤

| Component | Version | License | Source |
| --- | --- | --- | --- |
| Arduino-ESP32 | 2.0.17 | LGPL-2.1 | <https://github.com/espressif/arduino-esp32/tree/2.0.17> |

Arduino-ESP32のLGPL-2.1ライセンス本文は[licenses/LGPL-2.1.txt](licenses/LGPL-2.1.txt)に収録しています。Arduino-ESP32およびその構成要素の対応ソース、著作権表示、追加のライセンス情報については、上記のバージョン固定リンクを参照してください。

## ライブラリ

| Component | Resolved version / Constraint | License | Copyright notice | Source |
| --- | --- | --- | --- | --- |
| M5Unified | 0.2.20 / `^0.2.10` | MIT | Copyright (c) 2021 M5Stack | <https://github.com/m5stack/M5Unified> |
| M5GFX | 0.2.27 (transitive dependency) | MIT | Copyright (c) 2021 M5Stack | <https://github.com/m5stack/M5GFX> |
| M5Chain | 1.0.8 | MIT | Copyright (c) 2026 M5Stack Technology CO LTD | <https://github.com/m5stack/M5Chain> |
| Adafruit NeoPixel | 1.15.5 / `^1.12.3` | LGPL-3.0 | Copyright holders listed in the upstream source | <https://github.com/adafruit/Adafruit_NeoPixel> |
| ArduinoOSC | 0.6.0 | MIT | Copyright (c) 2017 Hideaki Tai | <https://github.com/hideakitai/ArduinoOSC> |
| ArduinoJson | 7.4.3 / `^7.4.2` | MIT | Copyright © 2014-2026 Benoit BLANCHON | <https://github.com/bblanchon/ArduinoJson> |
| ArxContainer | 0.7.0 (transitive dependency) | MIT | Copyright (c) 2019 Hideaki Tai | <https://github.com/hideakitai/ArxContainer> |
| ArxSmartPtr | 0.3.0 (transitive dependency) | MIT | Copyright (c) 2020 Hideaki Tai | <https://github.com/hideakitai/ArxSmartPtr> |
| ArxTypeTraits | 0.3.2 (transitive dependency) | MIT | Copyright (c) 2020 Hideaki Tai | <https://github.com/hideakitai/ArxTypeTraits> |
| DebugLog | 0.8.4 (transitive dependency) | MIT | Copyright (c) 2019 Hideaki Tai | <https://github.com/hideakitai/DebugLog> |

Adafruit NeoPixelのLGPL-3.0ライセンス本文は[licenses/LGPL-3.0.txt](licenses/LGPL-3.0.txt)に収録しています。MITライセンスの全文はルートの[LICENSE](LICENSE)に収録しています。第三者コンポーネントの著作権は、それぞれの著作権者に帰属します。

## 再現可能なビルド

配布ファームウェアに対応するChainOSCminiのソースコード、`platformio.ini`、ビルド手順はこのリポジトリで公開しています。リリース時のGitタグをチェックアウトし、PlatformIOで次を実行してください。

```sh
pio run -e chain_dualkey
```

PlatformIOとesptoolはビルドおよびファームウェア生成に使用するツールであり、ChainOSCminiのファームウェアへ組み込まれるコンポーネントとしては扱っていません。
