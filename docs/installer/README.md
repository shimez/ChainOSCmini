# ChainOSCmini Web Installer

ChainOSCminiのファームウェアをChain DualKeyへブラウザーから書き込むためのWeb Installerです。

現在の公開版は`1.3.0`です。

- Version 1.3.0: Wi-Fi認証情報、OSC送信先、Web UI言語をLittleFSへ移行し、旧NVS設定の自動移行と原子的な保存に対応
- Version 1.2.0: Device Preset Import Error Registry v1へ完全対応し、JSONインポートの検証と日英エラーメッセージをシリーズ共通化

## 公開URL

```text
https://shimez.github.io/ChainOSCmini/installer/
```

デスクトップ版のChromeまたはEdgeを使用します。

## 自動配信

GitHub ActionsがPlatformIOでmergedバイナリを生成してGitHub Releaseへ添付します。Releaseを公開すると、Pages Workflowが同じバイナリをPages成果物へ組み込みます。

```text
installer/firmware/ChainOSCmini-1.3.0-ChainDualKey-merged.bin
```

`manifest.json`はこのファイルをESP32-S3のoffset `0x0`へ書き込みます。Release Assetをブラウザーから直接参照しないため、CORSによる`Failed to fetch`を避けられます。

## ローカル確認

Releaseからmergedバイナリをダウンロードし、次の場所へ配置します。

```text
docs/installer/firmware/ChainOSCmini-1.3.0-ChainDualKey-merged.bin
```

その後、`docs/installer`でローカルWebサーバーを起動します。

```powershell
py -m http.server 8000 --bind 127.0.0.1
```

デスクトップ版ChromeまたはEdgeで`http://localhost:8000/`を開きます。
