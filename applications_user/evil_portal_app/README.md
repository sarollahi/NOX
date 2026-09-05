# Evil Portal (NOX port)

Captive-portal / credential-capture app: an attached ESP32 serves a fake Wi‑Fi login page and
harvested credentials are logged to the Flipper SD card. The portal HTML is editable on-device.

- **Upstream:** [bigbrodude6119/flipper-zero-evil-portal](https://github.com/bigbrodude6119/flipper-zero-evil-portal)
- **Bay:** Serial (ESP32). One-at-a-time with other UART devices.
- **ESP firmware:** flash the Evil Portal / Marauder ESP firmware (the ESP does the Wi‑Fi).

**NOX port:** migrated the UART layer from the removed `furi_hal_uart` API to `furi_hal_serial`
(`furi_hal_serial_control_acquire` + `furi_hal_serial_async_rx_start`; RX callback signature and
byte read updated; TX via the acquired handle). `UART_CH` → `FuriHalSerialIdUsart`.

> Authorized testing only — this captures third-party credentials. See ../../SECURITY.md.
