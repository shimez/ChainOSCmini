# ChainOSCmini Web Installer

ChainOSCminiのファームウェアをChain DualKeyへブラウザーから書き込むためのWeb Installerです。

現在の公開版は`0.9.0`です。開発版であることを理解したうえで使用してください。

## 公開URL

```text
https://shimez.github.io/ChainOSCmini/installer/
```

デスクトップ版のChromeまたはEdgeを使用します。

## 自動配信

GitHub ActionsがPlatformIOでmergedバイナリを生成してGitHub Releaseへ添付します。Releaseを公開すると、Pages Workflowが同じバイナリをPages成果物へ組み込みます。

```text
installer/firmware/ChainOSCmini-0.9.0-ChainDualKey-merged.bin
```

`manifest.json`はこのファイルをESP32-S3のoffset `0x0`へ書き込みます。Release Assetをブラウザーから直接参照しないため、CORSによる`Failed to fetch`を避けられます。

## ローカル確認

Releaseからmergedバイナリをダウンロードし、次の場所へ配置します。

```text
docs/installer/firmware/ChainOSCmini-0.9.0-ChainDualKey-merged.bin
```

その後、`docs/installer`でローカルWebサーバーを起動します。

```powershell
py -m http.server 8000 --bind 127.0.0.1
```

デスクトップ版ChromeまたはEdgeで`http://localhost:8000/`を開きます。
