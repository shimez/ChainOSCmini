# ChainOSCmini per-device-type boundary test data

This directory contains whole-settings JSON files for testing the 40-entry
limit independently for Encoder, Angle, Key, Joystick, and ToF settings.

For each device type:

- `ChainOSCmini-storage-test-<type>-40.json` contains 40 entries and is the acceptance-boundary test.
- `ChainOSCmini-storage-test-<type>-41.json` contains 41 entries and must be rejected without changing existing settings.

The older mixed files contain a total of 40 or 41 entries distributed across
all five device types. Use the type-specific files for per-type capacity tests.

## Key settings

The two built-in Keys do not consume the 40-entry saved Chain Key limit.
Forty external Key settings can be saved. Newly connected 41st and later external Key devices (up to the 64-device detection limit)
is shown in the Web UI, but saving it must return the per-type limit error.

Recommended procedure:

Repeat the following for each device type:

1. Export a backup of the current settings.
2. Start from a clean set of saved test-device settings.
3. Import the corresponding 40-entry file.
4. Confirm the reported restored count and inspect the saved-device list.
5. Reboot and confirm that all accepted settings remain available.
6. Re-save, export, and confirm the number and values of the entries.
7. Confirm normal OSC transmission and Web UI operation.
8. Import the corresponding 41-entry file and confirm rejection.
9. Confirm that the previously saved settings remain unchanged before and after reboot.

For the connected-device boundary test, keep 40 settings of one type saved and
connect a previously unknown device of the same type. Confirm that its card is
shown in the connected-device section. Saving must fail with
`デバイスの設定が種別ごとの上限40件を超えています。`, and the existing
40 files must remain unchanged.

Importing a settings file does not remove settings absent from that file. Delete
prior test settings before each clean boundary measurement.
