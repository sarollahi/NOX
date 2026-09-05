# NOX plug-in add-on headers

Goal: Wiegand / UHF-RFID / ESP32 / NFC / nRF24 are **not soldered on permanently** — you plug in
the one you need. They share pins, so only the rules below matter (one-at-a-time where noted).
No firmware change to swap: missing hardware never hangs (self-heal SPI + missing-hw guards).

Wire two female headers on the board and make matching module pigtails.

## Bay 1 — SERIAL (UART), ONE device at a time
5-pin header. PB6/PB7 are USART1 but also plain GPIO, so Wiegand can reuse them.

| Header pin | MCU | ESP32 (Marauder/Ghost/FlipperHTTP) | UHF (YRM100) | Wiegand reader |
|---|---|---|---|---|
| 1 | PB6 | → ESP **RX** | → module **RX** | **D0** |
| 2 | PB7 | ← ESP **TX** | ← module **TX** | **D1** |
| 3 | 5V  | ESP 5V/VIN | (module 3V3 pin below) | reader 12V* (own supply) |
| 4 | 3V3 | — | YRM100 VCC | pull-ups for D0/D1 |
| 5 | GND | GND | GND | GND |

- **ESP32 ⊕ UHF ⊕ Wiegand are mutually exclusive** (all own the UART). Plug one.
- Wiegand: readers are usually 12 V + open-collector D0/D1 → add **10k pull-ups D0/D1→3V3**;
  power the reader from its own 12 V, share GND only. (Emulate-mode drive pins: PA9, PE4.)
- \*Never feed 12 V into pins 1–5; D0/D1 must arrive at 3V3 level.

## Bay 2 — SPI (NFC / nRF24), shares SPI1
9-pin header. SCK/MOSI/MISO are the shared bus (also LCD/SD/CC1101 — always live).

| Header pin | MCU | NFC (ST25R3916) | nRF24L01+PA+LNA |
|---|---|---|---|
| 1 | PB3 | SCK | SCK |
| 2 | PB5 | MOSI | MOSI |
| 3 | PB4 | MISO | MISO |
| 4 | PA8 | NFC **CS** | — |
| 5 | PA0 | NFC **IRQ** | — |
| 6 | PA9 | — | **CSN** |
| 7 | PE4 | — | **CE** |
| 8 | 3V3 | VCC | VCC (use the 3V3 breakout adapter) |
| 9 | GND | GND | GND |

- NFC and nRF24 use **different CS pins**, so both can be plugged at once —
  **except** nRF24 **CE=PE4 also drives the SubGHz antenna switch** → don't run nRF24 while
  transmitting/receiving SubGHz. NFC + SubGHz is fine.
- Unplugged bay = apps still open, just sense nothing (Q7/CS pulled safe).

## Quick "what can run together"
- Always live: SubGHz (CC1101), LCD, microSD, buttons (’165), speaker, vibro, iButton, battery.
- Bay 1: ESP32 **or** UHF **or** Wiegand.
- Bay 2: NFC **and/or** nRF24 (nRF24 not with SubGHz).
- Example combos: SubGHz + NFC ✓ · ESP32-WiFi + NFC ✓ · nRF24 Mousejack (alone-ish) ✓ ·
  UHF-RFID + NFC ✓ · Wiegand capture (alone on UART) ✓.

## Firmware pin facts (already set)
- Wiegand app D0=PB6, D1=PB7, emulate-mosfets PA9/PE4 (repointed off SD-CS/CC1101-CS).
- nRF24 CSN=PA9 (external-SPI handle CS), CE=PE4 (repointed off the LCD-CS default).
- UHF/ESP32 use USART1 PB6/PB7 as-is.
- NFC CS=PA8, IRQ=PA0 as-is.
Dropped for good (no hardware): all LF-RFID (125/134 kHz) and Infrared apps.
