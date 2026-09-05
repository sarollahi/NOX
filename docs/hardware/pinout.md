# Pinout — WeAct STM32WB55CGU6 (UFQFPN48)

Every bonded GPIO is assigned. `SPI1` (PB3/PB4/PB5) is a **single shared bus** for CC1101, the
LCD, the SD card, and the plug-in NFC/nRF24 modules.

## Always-on core

| Function | Pin(s) | Notes |
|---|---|---|
| SPI1 SCK / MOSI / MISO | PB3 / PB5 / PB4 | shared bus, AF5 |
| Display (ST7565) | CS PB2 · RST PB0 · DC PB1 | backlight → 3V3 |
| CC1101 SubGHz | CS PA7 · GDO0 PA1 | EXTI1 |
| microSD | CS PA4 · DET PA3 | DET HIGH = inserted |
| Buttons (SN74HC165) | PL PB8 · CP PB9 · Q7 PA15 | 6 buttons on D0–D5, 10 ms poll |
| Speaker | PA6 | TIM16_CH1 / AF14 |
| Vibro | PA5 | via NPN + flyback |
| iButton (1-Wire) | PA10 | + 1 kΩ pull-up |
| Battery ADC | PA2 | 100k/100k divider from raw 18650(+), ADC1_IN7 |
| USB | PA11 / PA12 | |
| SWD | PA13 / PA14 | debug header |
| LED (WeAct blue) | PE4 | shared — see bays |

## Buttons — SN74HC165

All six buttons hang off one shift register so the MCU spends only 3 pins:

| '165 | → MCU | | input | button |
|---|---|---|---|---|
| PL (SH/LD) | PB8 | | D0 | Up |
| CP (CLK) | PB9 | | D1 | Down |
| Q7 (QH) | PA15 | | D2 | Right |
| CE → GND, DS → GND | | | D3 | Left |
| | | | D4 | Ok |
| | | | D5 | Back |

Each button: input → button → GND, with a 10 kΩ pull-up to 3V3. Tie D6/D7 → 3V3. Q7 has an MCU
pull-up so a **missing '165 reads all-released** (no phantom keys).

## Plug-in add-on bays

The swappable modules (ESP32 / UHF-RFID / Wiegand / NFC / nRF24) live on two headers — full wiring
per module in **[plug-in-headers.md](plug-in-headers.md)**. Summary:

- **Bay 1 — Serial (5-pin):** PB6, PB7, 5V, 3V3, GND. ESP32 **or** UHF **or** Wiegand.
- **Bay 2 — SPI (9-pin):** PB3, PB5, PB4, PA8 (NFC CS), PA0 (NFC IRQ), PA9 (nRF24 CSN), PE4 (nRF24 CE), 3V3, GND.

## Do not use

- **PH3** — BOOT0 strap; anything idle-high here forces a DFU boot. Leave unconnected.
- **PC0/PC1/PC3** — not bonded on UFQFPN48.
- LF-RFID / IR pins — not present; those features are excluded (see [limitations.md](../limitations.md)).
