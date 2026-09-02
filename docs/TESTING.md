# ChainOSCmini 実機確認項目

リリース前の回帰テストに使用する確認項目です。現在の対象バージョンは1.4.1です。

## AP Mode全設定削除

- AP Modeのキャプティブポータル最下段に赤色の「すべての設定を削除」ボタンが表示される
- キャンセルすると設定が変更されない
- 確認するとWi-Fi、OSC送信先、UI言語、本体キー／Chainデバイス設定が削除される
- 削除後に再起動し、再びAP Modeで起動する

## ビルドと基本動作

- Arduino IDEでコンパイルできる
- Arduino IDEのコンパイル出力で、アプリ領域が約3.19 MiBとして認識される
- PlatformIOでビルドできる
- 両方からChain DualKeyへ書き込める
- 再起動後にシリアルログが表示される
- Chip ModelとFlash容量が想定どおり表示される
- 1分以上動作させても再起動しない
- 空きHeapが継続的に減少しない

## DualKey本体とLED

- AP Modeでは2個のLEDが紫でゆっくり点滅する
- Wi-Fi接続中は2個のLEDが青でゆっくり点滅する
- Wi-Fi接続後は2個のLEDが青で常時点灯する
- KEY1を押している間、対応するLEDだけがオレンジになる
- KEY2を押している間、対応するLEDだけがオレンジになる
- キーを離すと現在のWi-Fi状態色へ戻る
- 押下と解放がそれぞれ1回ずつシリアルへ表示される
- 同時押しを個別に検出できる
- キーを繰り返し操作しても再起動しない

## ChainポートとChain Key／Encoder／Angle／ToF／Joystick

- `G5_G6`側だけで列挙・入力・LEDが動作する
- `G47_G48`側だけでも列挙・入力・LEDが動作する
- 左右それぞれにChain Keyを接続すると、ポート別に`CONNECTED`が表示される
- 列挙された台数、ID、種類、UIDが表示される
- デバイスを外すと`DISCONNECTED`が表示される
- 再接続すると同じUIDが表示される
- 複数デバイスの接続と順序変更を検出できる
- Chainの走査中もDualKey本体のキーとLEDが動作する
- Chain Keyの押下／解放が対応するUIDとともに1回ずつ表示される
- 操作したChain Keyだけが青からオレンジへ変わる
- 複数のChain Keyを個別および同時に操作できる
- 抜き差しや接続順序変更後も、各UIDの入力を正しく取得できる
- 左右のChain Keyを同時に操作できる
- 両側に`id=1`が存在しても、ポート名とUIDで区別できる
- 一方だけを抜き差ししても、反対側の入力が継続する
- 同じChain Keyを反対側へ移してもUIDが変わらない
- Chain Encoderが左右どちらのポートでも種類とUID付きで列挙される
- Encoderを抜き差し、左右移動、接続順変更しても同じUIDとして認識される
- Chain Angleが左右どちらのポートでも種類とUID付きで列挙される
- Angleを抜き差し、左右移動、接続順変更しても同じUIDとして認識される
- Chain Joystickが左右どちらのポートでも種類とUID付きで列挙される
- Joystickを抜き差し、左右移動、接続順変更しても同じUIDとして認識される

抜き差しの瞬間には`TIMEOUT`が一度表示されることがあります。次の走査で自動復帰し、正常な列挙結果を失わないことを確認します。

## Wi-FiとWeb UI

- Wi-Fi未設定時に`ChainOSCmini-Setup`が表示される
- APへ接続するとキャプティブポータルが開く
- 2.4 GHz帯Wi-Fiの設定を保存して再起動できる
- 再起動後に保存済みWi-Fiへ接続できる
- シリアルログへIPアドレスが表示される
- `http://chainoscmini.local/`またはIPアドレスで設定画面を開ける
- 設定画面にバージョン、IPアドレス、mDNS名が表示される
- Wi-Fi設定を削除するとAP Modeへ戻る
- Wi-Fi接続待ちやWebアクセス中もDualKeyと左右Chainが動作する
- Wi-Fi有効時も空きHeapが継続的に減少しない

## OSCとデバイス設定

- Web画面でOSC送信先を保存し、再起動後も復元される
- DualKey KEY1／KEY2の押下・解放で設定したOSCメッセージを受信できる
- 左右のChain Keyを操作するとUID別設定でOSCメッセージを受信できる
- Wi-Fi設定を削除してもOSC送信先設定が保持される
- 本体KEY1／KEY2のAddress、型、値を個別変更できる
- PressとReleaseの合計8件まで追加でき、9件目を追加できない
- メッセージの追加、削除、並べ替え後の設定が再起動後も復元される
- メッセージ0件ではOSCを送信しない
- SequenceのStart／End／Step／Typeと周回動作が正しい
- Sequence設定が再起動後に復元される
- 複数のChain KeyがUID別に設定画面へ表示される
- Chain Keyの設定変更が直後の操作から反映される
- Chain Keyを抜き差し、左右移動、順序変更してもUID設定が復元される
- 取り外したChain Keyが「保存済みデバイス（未接続）」へ移動する
- 未接続の保存済みChain Key設定を削除し、再接続後に初期値となる
- 画面をスクロールしても「すべての設定を保存」ボタンが追従する
- OSC送信先と複数デバイス設定を一度の操作で保存できる
- 再接続すると同じUIDの設定が「接続中のデバイス」へ戻る
- DualKey本体キーの`…`から識別すると、対応する側のLEDだけが10秒間オレンジになる
- 接続中のChain Key／Encoder／Angle／ToF／Joystickの`…`から識別すると、対象UIDのLEDだけが10秒間オレンジになる
- 識別中にキー操作してもオレンジ表示が維持され、10秒後に通常の状態表示へ戻る

### Chain Joystick

- X/Y軸がDeadband以上変化したときだけ、それぞれのOSC Addressへ送信する
- Invert X／Invert Yが各軸に正しく適用される
- Out Min／Out MaxとFloat／Int／Stringが正しく適用される
- クリックのPress / Release合計8件、0件、並べ替え、SequenceがKeyと同様に動作する
- GPIO5/GPIO6側では通常のX/Y軸として動作する
- GPIO47/GPIO48側では、設定操作なしでX軸とY軸の正負が両方とも自動反転する
- GPIO47/GPIO48側の自動補正後に、Web UIのX軸反転／Y軸反転が追加で適用される
- 同じJoystickを左右へ移動してもUID設定が復元され、それぞれのポートに応じた向きで動作する
- 全体JSONとM5ChainOSC互換Joystickプリセットをエクスポート／インポートできる

### Chain Encoder

- 初回の回転値取得ではOSCを送信せず、その後の回転変化で送信する
- Absoluteで入力範囲が循環し、Out Min／Out Maxへ変換される
- Incrementで回転差分×Inc Scaleを送信し、逆回転では負の値になる
- Out TypeのFloat／Int／Stringが正しく送信される
- クリックのPress / Release合計8件、0件、追加、削除、並べ替えがKeyと同様に動作する
- クリックのSequenceでStart／End／Step／Typeと周回動作が正しい
- クリック時はPress / Releaseで赤、Sequenceで緑、離した後は青へ戻る
- 回転設定とクリック設定が同時に動作する
- 設定を保存して再起動、抜き差し、左右移動してもUID単位で復元される
- 取り外したEncoderが保存済みデバイスへ移り、削除後の再接続で初期値になる

### Chain Angle

- 初回の角度値取得ではOSCを送信せず、その後の変化で送信する
- 12-bitでは0～4095、8-bitでは0～255をOut Min／Out Maxへ変換する
- Deadband未満の変化では送信せず、以上の変化では送信する
- Out TypeのFloat／Int／Stringが正しく送信される
- 設定を保存して再起動、抜き差し、左右移動してもUID単位で復元される
- 取り外したAngleが保存済みデバイスへ移り、削除後の再接続で初期値になる

## システム設定のLittleFS保存とNVS移行

- 旧ファームウェアでWi-Fi、OSC送信先、Web UI言語をNVSへ保存する
- ファイルシステムを消去せずに更新し、`[ChainOSCmini][SYSTEM] migration source=nvs target=littlefs result=ok`が出る
- 再起動後もWi-Fi接続、OSC送信先、Web UI言語が復元される
- システム設定の保存ログでファイルサイズ、LittleFS総容量、使用量、空き容量を確認できる
- Wi-Fi設定を削除してもOSC送信先とWeb UI言語は維持される
- Web Installerまたはmerged firmwareでファイルシステムを消去せず更新し、同じ設定が維持される

## JSONバックアップとプリセット

- 全体設定JSONをエクスポートでき、`format`が`ChainOSCmini-settings`である
- 全体設定JSONにWi-FiのSSIDとパスワードが含まれない
- 全体設定JSONをインポートするとOSC送信先、UI言語、本体キー、保存済みChain Keyが復元される
- 再起動後もインポートした設定が保持される
- Chain Keyプリセットをエクスポートでき、`format`が`ChainOSC-device-preset`である
- DualKey本体のKEY1／KEY2にも「…」メニューが表示され、プリセットをエクスポート／インポートできる
- Chain KeyプリセットにUIDとデバイス名が含まれない
- M5ChainOSCでエクスポートしたKeyプリセットをChainOSCminiへインポートできる
- ChainOSCminiでエクスポートしたKeyプリセットをM5ChainOSCへインポートできる
- EncoderプリセットにUIDとデバイス名が含まれない
- M5ChainOSCとChainOSCminiの間でEncoderプリセットを相互にインポートできる
- 全体設定JSONでEncoder設定をエクスポート／復元できる
- AngleプリセットにUIDとデバイス名が含まれない
- M5ChainOSCとChainOSCminiの間でAngleプリセットを相互にインポートできる
- 全体設定JSONでAngle設定をエクスポート／復元できる
- 30 mm以上かつMaximum Distance未満でToF値を送信し、範囲外では送信を停止する
- ToFのDeadband、近距離／遠距離の出力方向、Out Min／Max、Float／Intが正しい
- ToF設定が再起動、抜き差し、左右移動後もUID単位で復元される
- M5ChainOSCとChainOSCminiの間でToFプリセットを相互にインポートできる
- 全体設定JSONでToF設定をエクスポート／復元できる
- 全体設定JSONをプリセットとして選択した場合はエラーになり、設定が変化しない
- 破損JSON、異なるformat、schemaVersion、デバイス種類、無効なAddress、9件以上のメッセージを拒否する
- 32 KiB境界テストJSONを正常にインポートできる
- 64 KiBテストJSONを容量超過として拒否する
