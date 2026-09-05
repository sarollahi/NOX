# Limitations

NOX is a retarget of Flipper firmware onto cheaper, smaller silicon (STM32**WB55CGU6**,
UFQFPN48) than the real Flipper's WB55**RG**. Fewer pins, one SPI bus, no analog RF front-end.
These are the honest trade-offs.

## Cannot work on this board (excluded, not broken)

| Missing | Why | Affected apps |
|---|---|---|
| **125/134 kHz LF-RFID** | No LF analog front-end (carrier driver + LC coil + envelope demod); `RFID_*` pins aren't bonded on UFQFPN48. No external module can substitute — the Flipper LF app drives on-die analog, not a plug-in. | lfrfid, T5577 writers, EM4100, FDX-B, RFID metal detector |
| **Infrared** | No IR LED / receiver, no free timer pin left after dropping IR. | infrared, all AC-remote apps |

These apps are removed from the build. Don't buy hardware to "add" them — it won't integrate.

## Shared single SPI bus

CC1101, the LCD, the SD card, NFC and nRF24 all share **one SPI1 bus** (PB3/4/5). The real
Flipper uses two. Consequences:

- Bus handles reconfigure the SCK/MOSI/MISO pins on release; the display/SD handle re-applies the
  SPI1 alternate function on **every** activation so it self-heals (this was a real bring-up bug).
- Any new SPI device you add must not assume pin state persists across a radio/NFC transaction.

## Pin starvation & the add-on bays

Every GPIO on the package is allocated. The two [plug-in bays](hardware/plug-in-headers.md) reuse
pins, so some things are mutually exclusive:

- **Serial bay:** ESP32 **or** UHF-RFID **or** Wiegand — one at a time (all own USART1).
- **SPI bay:** NFC and nRF24 can coexist (different CS), but **nRF24 CE = PE4 also drives the
  SubGHz antenna switch** → don't run nRF24 while transmitting/receiving SubGHz.
- **CAN (MCP2515) / W5500 Ethernet** add-ons fit the SPI bay but borrow its CS/INT lines, so they
  compete with NFC/nRF24 — plug one SPI add-on at a time.
- **Wiegand emulate** (not just read) needs the drive pins PA9/PE4 from the SPI bay in addition to
  D0/D1 on the serial bay.

There are **no more bonded GPIOs** to expose — 14 header pins is the physical maximum.

## Flash ceiling

The BLE radio stack sits near the top of the 1 MB flash and the linker does **not** reserve it.
Firmware must stay below the stack install address (light stack: `0x080D7000` = 880,640 B).
Current build ≈ 866 KB, ~14 KB headroom. Compiled-in apps eat this budget; SD `.fap` apps do not.

## Bring-up / bench realities

- **SWD** over dupont jumpers is unreliable on this board; use the `reboot2dfu` race method
  ([setup.md](setup.md)) for app flashing. Radio-stack swaps need a soldered SWD header.
- The following are **built and size-safe but unverified on hardware** by the author of a given
  build: the SN74HC165 button scan timing, battery-% ADC calibration, speaker/vibro, nRF24 timing.
  Treat first power-on as bring-up; the divider ratio and '165 bit-delay are tunable knobs.
