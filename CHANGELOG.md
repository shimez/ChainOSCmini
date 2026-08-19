# Changelog

ChainOSCminiの主な変更履歴を記録します。

形式は[Keep a Changelog](https://keepachangelog.com/ja/1.1.0/)を参考にし、バージョン番号には[Semantic Versioning](https://semver.org/lang/ja/)を使用します。

## [Unreleased]

## [1.0.0]

### Added

- Chain Encoderの回転とクリック入力に対応
- EncoderのAbsolute／Increment、入出力範囲、型設定に対応
- EncoderクリックのPress / Release、最大8メッセージ、Sequenceに対応
- Encoder設定のUID単位保存、全体JSON、M5ChainOSC互換プリセットに対応
- Chain Angleの8-bit／12-bit入力とOSC送信に対応
- AngleのDeadband、出力範囲、Float／Int／String型設定に対応
- Angle設定のUID単位保存、全体JSON、M5ChainOSC互換プリセットに対応
- DualKey本体キーと接続中のChainデバイスを10秒間オレンジ点灯して識別する機能を追加
- Chain ToFの距離入力、有効最大距離、範囲外でのOSC送信停止に対応
- ToFのDeadband、出力方向・範囲・型、UID保存、全体JSON、M5ChainOSC互換プリセットに対応
- Chain JoystickのX/Y軸入力、クリック、Deadband、反転、出力範囲・型に対応
- GPIO47/GPIO48側ではJoystickのX軸・Y軸の正負を常に自動反転し、左右の物理的な向きを補正

## [0.9.1]

### Added

- AP Modeを紫のゆっくりした点滅で示す本体LED表示を追加
- Wi-Fi接続中を青のゆっくりした点滅、接続済みを青の常時点灯で示す機能を追加
- キー解放後に現在のWi-Fi状態を示すLED表示へ戻す処理を追加
- JSONインポートの容量境界を確認する32 KiB／64 KiBテストデータと生成スクリプトを追加

## [0.9.0]

### Added

- ChainOSCmini全体設定のJSONエクスポート／インポートに対応
- M5ChainOSCと共通の`ChainOSC-device-preset`形式によるKeyプリセットのエクスポート／インポートに対応
- 旧`M5ChainOSC-device-preset`形式のインポート互換性を追加
- 日本語／英語のユーザーガイドとプリセット・クイックスタートを追加
- JSONの形式、容量、schemaVersion、デバイス種類、Address、型、値、メッセージ件数の検証を追加

### Changed

- Wi-Fi設定削除、システム情報、全体設定バックアップ／復元の配置をM5ChainOSCのWeb UIへ統一

## [0.8.0]

### Added

- DualKey本体のKEY1／KEY2とChain Keyを設定するWeb UIを追加
- KeyのPress / Release、最大8件のOSCメッセージ、Float／Int／String型に対応
- KeyのSequenceモードとStart／End／Step／Type、周回動作に対応
- Chain Key設定のUID単位での保存・復元に対応
- 接続中デバイスと保存済み・未接続デバイスを分けて表示する機能を追加
- 保存済み・未接続デバイスの設定削除に対応
- 日本語／英語の表示切り替えに対応

### Changed

- Web UIの構成、デバイスカード、折りたたみ、一括保存ボタンをM5ChainOSCへ統一
- 設定保存をデバイスごとのNVS名前空間へ分離し、保存直後の読み戻し検証を追加

## 0.7.0

### Added

- OSC送信先のIPアドレス／ホスト名とUDPポートをWeb UIから設定する機能を追加
- DualKey本体キーとChain Keyの押下／解放によるOSC送信に対応
- Wi-Fi接続後のmDNS名`chainoscmini.local`による設定画面アクセスに対応

## 0.6.0

### Added

- 設定用アクセスポイント`ChainOSCmini-Setup`を追加
- キャプティブポータルによる2.4 GHz帯Wi-Fi設定に対応
- Wi-Fi認証情報のNVS保存と、保存済みWi-Fiへの自動接続に対応
- 接続タイムアウト時にAP Modeへ移行する処理を追加

### Fixed

- Chain DualKeyでクライアント接続時に再起動する問題を、Wi-Fi送信出力を2 dBmへ制限して改善

## 0.5.0

### Added

- GPIO5／GPIO6側とGPIO47／GPIO48側の両方のChainポートに対応
- 左右ポートを独立して列挙・監視し、ポート名とUIDでデバイスを識別する機能を追加
- 一方の切断や通信失敗時も反対側の状態を維持する処理を追加

## 0.4.0

### Added

- Chain Keyの押下／解放イベント取得に対応
- 複数Chain Keyの入力監視と、通常時の青／押下中のオレンジLED表示に対応
- 抜き差しや順序変更後の入力・LED再初期化に対応

## 0.3.0

### Added

- GPIO5／GPIO6側Chainポートのデバイス列挙に対応
- Chainデバイスの台数、ID、種類、UIDの診断ログを追加
- 接続、切断、台数および順序変更の検出を追加

### Fixed

- 一時的なChain通信タイムアウト時に直前の正常な列挙状態を維持するよう改善

## 0.2.0

### Added

- DualKey本体のKEY1（GPIO0）とKEY2（GPIO17）の入力検出に対応
- 20 msのデバウンスと押下／解放ログを追加
- 2個のWS2812BとLED電源制御に対応
- 本体キー押下中のオレンジLED表示を追加

## 0.1.0

### Added

- Chain DualKey向け最小起動ファームウェアを作成
- ESP32-S3、CPU、Flash、Sketch、Heap、PSRAM、リセット理由の診断ログを追加
- Arduino IDEとPlatformIOの両方に対応するプロジェクト構成を追加
- GPIOを駆動しない安全なbring-upモードを追加

[Unreleased]: https://github.com/shimez/ChainOSCmini/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/shimez/ChainOSCmini/releases/tag/v1.0.0
[0.9.1]: https://github.com/shimez/ChainOSCmini/releases/tag/v0.9.1
[0.9.0]: https://github.com/shimez/ChainOSCmini/releases/tag/v0.9.0
[0.8.0]: https://github.com/shimez/ChainOSCmini/releases/tag/v0.8.0
