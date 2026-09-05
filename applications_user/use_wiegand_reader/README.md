# Wiegand Reader (NOX port)

Reads (and can emulate) the **Wiegand** D0/D1 protocol used by access-control card readers —
capture badge bitstreams, replay them.

- **Upstream:** all-the-plugins / community Wiegand reader.
- **Bay:** Serial — **D0 = PB6, D1 = PB7** (repointed for NOX). Emulate-drive pins PA9/PE4.
- **Wiring:** reader D0/D1 → PB6/PB7 with 10 kΩ pull-ups to 3V3; power the reader from its own
  12 V, share GND only. Never feed 12 V into the header pins.

**NOX port:** pin map moved off the stock `gpio_ext_pa4/pa7/pb3/pa6` (which are SD‑CS / CC1101‑CS /
SCK / speaker on this board) to the Serial bay: `pinD0=gpio_usart_tx`, `pinD1=gpio_usart_rx`,
mosfet drive `pinD0mosfet=gpio_i2c_power_scl` (PA9), `pinD1mosfet=gpio_periph_power` (PE4).
