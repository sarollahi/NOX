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
| Buttons (SN74HC165) | PL PB8 · CP PA5 · Q7 PA15 | 6 buttons on D0–D5, 10 ms poll |
| IR transmit | PB9 | TIM1_CH3N — plug-in IR LED module (e.g. KY-005) |
| IR receive | PA0 | TIM2_CH1 — plug-in 38 kHz demodulator (TSOP38238 / KY-022) |
| Speaker (buzzer) | PA6 | TIM16_CH1 / AF14 |
| iButton (1-Wire) | PA10 | + 1 kΩ pull-up |
| USB | PA11 / PA12 | |
| SWD | PA13 / PA14 | debug header |
| LED (WeAct blue) | PE4 | shared — see bays |

IR TX (PB9) and IR RX (PA0) are the **same pins/timers the real Flipper uses**, so the stock IR
HAL runs unmodified. Both are plug-in — nothing there when no IR module is attached, harmless.

> **Dropped vs the earlier layout:** vibro (was PA5, now the '165 CLK) and the battery-% ADC (was
> PA2, now the NFC IRQ) were removed to free the two pins IR needs. Buzzer stays on PA6.

## Buttons — SN74HC165

All six buttons hang off one shift register so the MCU spends only 3 pins:

| '165 | → MCU | | input | button |
|---|---|---|---|---|
| PL (SH/LD) | PB8 | | D0 | Up |
| CP (CLK) | PA5 | | D1 | Down |
| Q7 (QH) | PA15 | | D2 | Right |
| CE → GND, DS → GND | | | D3 | Left |
| | | | D4 | Ok |
| | | | D5 | Back |

Each button: input → button → GND, with a 10 kΩ pull-up to 3V3. Tie D6/D7 → 3V3. Q7 has an MCU
pull-up so a **missing '165 reads all-released** (no phantom keys). The '165 is bit-banged (not on
the SPI bus), so it never contends with CC1101/NFC/SD.

## Infrared (plug-in)

Two-wire signal + power. Uses the stock-Flipper IR pins, so no firmware retarget.

| IR module | → MCU | Notes |
|---|---|---|
| Transmitter DAT (KY-005) | PB9 | drive an IR LED via its NPN; VCC 3V3 (or 5V for range), GND |
| Receiver DAT (TSOP38238 / KY-022) | PA0 | demodulated 38 kHz output; **VCC = 3V3 only** (5V output would over-volt PA0) |

## Plug-in add-on bays

The swappable modules (ESP32 / UHF-RFID / Wiegand / NFC / nRF24) live on two headers — full wiring
per module in **[plug-in-headers.md](plug-in-headers.md)**. Summary:

- **Bay 1 — Serial (5-pin):** PB6, PB7, 5V, 3V3, GND. ESP32 **or** UHF **or** Wiegand.
- **Bay 2 — SPI (9-pin):** PB3, PB5, PB4, PA8 (NFC CS), PA2 (NFC IRQ), PA9 (nRF24 CSN), PE4 (nRF24 CE), 3V3, GND.

## Do not use

- **PH3** — BOOT0 strap; anything idle-high here forces a DFU boot. Leave unconnected.
- **PC0/PC1/PC3** — not bonded on UFQFPN48; used as no-op sinks (vibro → PC1, SubGHz RF-switch → PC3, so PE4 is free for nRF24 CE).
- LF-RFID pins — not present; 125/134 kHz is excluded (see [limitations.md](../limitations.md)).
