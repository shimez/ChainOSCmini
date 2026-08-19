#!/usr/bin/env python3
"""Generate exact-size ChainOSCmini JSON fixtures for import boundary tests."""

from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = ROOT / "test-data" / "json-import"


def key(identity: str, name: str, address: str) -> dict:
    return {
        "identity": identity,
        "deviceType": 3,
        "deviceTypeName": "Key",
        "displayName": name,
        "builtIn": True,
        "key": {
            "mode": 0,
            "press": [{"address": address, "value": "1", "type": 1}],
            "release": [{"address": address, "value": "0", "type": 1}],
            "sequence": {
                "address": address,
                "type": 1,
                "start": 0,
                "end": 10,
                "step": 1,
            },
        },
    }


def document() -> dict:
    return {
        "format": "ChainOSCmini-settings",
        "schemaVersion": 1,
        "firmwareVersion": "1.0.0",
        "wifiCredentialsIncluded": False,
        "global": {
            "oscHost": "192.168.1.100",
            "oscPort": 9000,
            "uiLanguage": "ja",
        },
        "devices": [
            key("dualkey:1", "DualKey KEY1", "/chainoscmini/dualkey/key1"),
            key("dualkey:2", "DualKey KEY2", "/chainoscmini/dualkey/key2"),
        ],
    }


def write_exact_size(filename: str, target_bytes: int) -> None:
    data = document()
    data["stressTestPadding"] = ""
    compact = json.dumps(data, ensure_ascii=True, separators=(",", ":"))
    missing = target_bytes - len(compact.encode("utf-8"))
    if missing < 0:
        raise RuntimeError(f"Base JSON exceeds requested size: {target_bytes}")
    data["stressTestPadding"] = "X" * missing
    output = json.dumps(data, ensure_ascii=True, separators=(",", ":"))
    encoded = output.encode("utf-8")
    if len(encoded) != target_bytes:
        raise RuntimeError(f"Expected {target_bytes} bytes, got {len(encoded)}")
    json.loads(output)
    (OUTPUT_DIR / filename).write_bytes(encoded)


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    write_exact_size("ChainOSCmini-settings-exact-32KiB.json", 32 * 1024)
    write_exact_size("ChainOSCmini-settings-over-limit-64KiB.json", 64 * 1024)
    for path in sorted(OUTPUT_DIR.glob("*.json")):
        print(f"{path.name}: {path.stat().st_size} bytes")


if __name__ == "__main__":
    main()
