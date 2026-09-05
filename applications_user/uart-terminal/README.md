# UART Terminal (NOX port)

A serial console on the Flipper — talk to any UART device (ESP32, modems, dev boards) at a
configurable baud rate, with hex/text views and a send buffer.

- **Upstream:** [cool4uma/UART_Terminal](https://github.com/cool4uma/UART_Terminal)
- **Bay:** Serial. Pins PB6 (TX) / PB7 (RX).

**NOX port:** same `furi_hal_uart` → `furi_hal_serial` migration as Evil Portal (acquire handle,
async RX callback, handle-based TX), `UART_CH` → `FuriHalSerialIdUsart`.
