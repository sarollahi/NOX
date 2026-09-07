# Limitations

NOX is a retarget of Flipper firmware onto cheaper, smaller silicon (STM32**WB55CGU6**,
UFQFPN48) than the real Flipper's WB55**RG**. Fewer pins, one SPI bus, no analog RF front-end.
These are the honest trade-offs.

## Cannot work on this board (excluded, not broken)

| Missing | Why | Affected apps |
|---|---|---|
| **125/134 kHz LF-RFID** | No LF analog front-end (carrier driver + LC coil + envelope demod); `RFID_*` pins aren't bonded on UFQFPN48. No external module can substitute — the Flipper LF app drives on-die analog, not a plug-in. | lfrfid, T5577 writers, EM4100, FDX-B, RFID metal detector |

LF-RFID is removed from the build. Don't buy hardware to "add" it — it won't integrate.

**Infrared is supported** (it was previously excluded). IR TX (PB9) and IR RX (PA0) are the same
pins/timers the real Flipper uses, so the stock IR HAL runs unmodified. It's a plug-in module (IR
LED + 38 kHz demodulator — see [pinout.md](hardware/pinout.md)). Freeing those two pins cost the
**vibro** motor (PA5 is now the '165 CLK) and the **battery-% ADC** (PA2 is now the NFC IRQ); the
buzzer stays. Battery % currently reports full on USB regardless (ADC sense disabled).

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
- **SPI bay:** NFC and nRF24 coexist (different CS), and both now coexist with **SubGHz** too —
  the RF-switch GPIO was parked on non-bonded PC3 (no switch chip exists on this board), so PE4 is
  the nRF24 CE alone. The one true limit is the shared SPI1 bus: it's **time-sliced**, so you
  interleave (poll CC1101, then nRF24) rather than clock both in the same instant. No simultaneous
  transmit on both radios at once.
- **CAN (MCP2515) / W5500 Ethernet** add-ons fit the SPI bay but borrow its CS/INT lines, so they
  compete with NFC/nRF24 — plug one SPI add-on at a time.
- **Wiegand emulate** (not just read) needs the drive pins PA9/PE4 from the SPI bay in addition to
  D0/D1 on the serial bay.

There are **no more bonded GPIOs** to expose — 14 header pins is the physical maximum.

## No SD card means no settings

There is no internal filesystem on this board. Only the external storage backend is compiled in,
and `storage_process_alias()` redirects every `/int/...` path to `/ext/.int/...` on the SD card.

Every settings file uses `INT_PATH(...)`: desktop, notification, power, input, expansion, BT keys
and dolphin state. So **all of them live on the SD card**, and with no card inserted none of them
survive a reboot: `..._settings_load()` fails on each boot and the struct is zeroed to defaults.

Within a single session the behaviour splits, depending on who applies the setting:

- **Applied from the desktop's in-RAM copy, so they hold until reboot.** On exit the settings app
  calls `desktop_api_set_settings()`, which stores the struct in the desktop service and applies
  it from there; the failed disk write only costs you persistence across a reboot. This covers
  Show Clock, the Lock Screen Clock options, dummy mode and auto-lock.
- **Re-read from storage, so they revert the moment you leave the settings screen.** The scene's
  exit handler calls `power_trigger_ui_update()`, which does its own `desktop_settings_load()`,
  fails, and gets defaults back. This covers Battery View and Hide Battery Icon.

(`DesktopGlobalReloadSettings`, which does re-read from disk, is only sent when an SD card
mounts, so it does not clobber the RAM copy on its own.)

Porting a real internal filesystem is not available as a fix: it would need flash pages, and only
about 15 KB is free below the BLE radio stack.

Card-detect is on PA3, active HIGH. `furi_hal_sd_is_present()` mostly just reads it, with one
exception for boards where the line was never wired:

1. DET high: card present. This also records that the line works.
2. DET low, having read high at some point: card absent. Identical to stock, so removal
   detection, unmounting and the early exit on the retry ladders all behave normally.
3. DET low, never having read high: the line may not be connected, so claim present until one
   real init attempt answers, then report that answer.

Only case 3 is new, and it can only happen on a board where DET has never once worked. An empty
slot there costs one failed init at boot and is quiet afterwards.

## Flash ceiling

The BLE radio stack sits near the top of the 1 MB flash and the linker does **not** reserve it.
Firmware must stay below the stack install address (light stack: `0x080D7000` = 880,640 B).
Current build ≈ 866 KB, ~14 KB headroom. Compiled-in apps eat this budget; SD `.fap` apps do not.

## Bring-up / bench realities

- **SWD** over dupont jumpers is unreliable on this board; use the `reboot2dfu` race method
  ([setup.md](setup.md)) for app flashing. Radio-stack swaps need a soldered SWD header.
- **Verified on hardware:** SubGHz (CC1101), display, SN74HC165 buttons, NFC, IR TX/RX, buzzer.
- **Not on this board:** battery-% ADC and vibro were removed to free pins for IR (battery reports
  full on USB; no vibro). nRF24 timing is built but unverified — treat first use as bring-up. The
  '165 bit-delay is a tunable knob if long wires glitch the read.
