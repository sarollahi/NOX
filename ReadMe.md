<div align="center">

# NOX

**The Flipper Zero firmware, running on a hand-built ~$5 multi-tool.**

[![Release firmware](https://github.com/sarollahi/NOX/actions/workflows/release.yml/badge.svg)](https://github.com/sarollahi/NOX/actions/workflows/release.yml)
[![License: GPLv3](https://img.shields.io/badge/license-GPLv3-blue)](LICENSE)
[![Based on: Flipper Zero](https://img.shields.io/badge/based%20on-Flipper%20Zero%20(Unleashed)-6f4e37)](https://github.com/DarkFlippers/unleashed-firmware)
[![MCU: STM32WB55CGU6](https://img.shields.io/badge/MCU-STM32WB55CGU6-6f4e37)](docs/hardware/pinout.md)
[![Apps: 147](https://img.shields.io/badge/apps-147%20%2B%2086%20plugins-6f4e37)](docs/app-catalog.md)

</div>

**NOX runs the [Flipper Zero](https://github.com/flipperdevices/flipperzero-firmware) firmware
([Unleashed](https://github.com/DarkFlippers/unleashed-firmware) fork)** — the same codebase as the
real device — retargeted to a hand-built handheld: a bare **WeAct STM32WB55CGU6** dev board plus
cheap radio, RFID, display and input modules. No custom PCB, no $170 device. The **complete source
tree lives in this repo** and builds with a single command.

- **Whole codebase, in-repo.** Clone and run `./fbt` — no external checkout, no submodule dance. What you see is what ships.
- **Only the parts you want.** Peripherals hang off two **plug-in add-on bays** — swap ESP32 / UHF-RFID / NFC / nRF24 in and out; nothing is soldered permanently.
- **Missing hardware never breaks it.** A shared-SPI self-heal plus missing-hardware guards keep the UI alive when a module isn't plugged in.

> ⚠️ **Offensive-security tooling.** NOX ships WiFi/BLE/SubGHz/NFC/RFID attack apps. Use them only
> on hardware and networks you own or are explicitly authorized to test — see [SECURITY.md](SECURITY.md).

---

## Hardware

| Function | Part | Bus / pins |
|---|---|---|
| MCU | WeAct **STM32WB55CGU6** (UFQFPN48) | — |
| SubGHz | **CC1101** | shared SPI1, CS PA7, GDO0 PA1 |
| Display | **ST7565 128×64** | shared SPI1, CS PB2 / RST PB0 / DC PB1 |
| microSD | SPI SD module | shared SPI1, CS PA4, DET PA3 |
| NFC | **ST25R3916** *(plug-in)* | shared SPI1, CS PA8, IRQ PA2 |
| Buttons | **SN74HC165** shift register | PL PB8 / CP PA5 / Q7 PA15 |
| Infrared *(plug-in)* | IR LED + 38 kHz receiver | TX PB9 · RX PA0 (stock-Flipper IR pins) |
| Speaker (buzzer) · iButton | passive + NPN | PA6 (TIM16) · PA10 |
| Add-on bays | 5-pin serial + 9-pin SPI + IR | [plug-in headers](docs/hardware/plug-in-headers.md) |

Full pin map: **[docs/hardware/pinout.md](docs/hardware/pinout.md)**.

## Build & flash

Everything needed is in the tree. First build downloads the ARM toolchain automatically.

```bash
git clone https://github.com/sarollahi/NOX.git
cd NOX
./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
```

Flash the resulting firmware over USB-DFU:

```bash
STM32_Programmer_CLI -c port=usb1 -w dist/f55-C/flipper-z-f55-full-local.bin 0x08000000 -v --run
```

First-board bring-up (radio stack, OTP, the reliable `reboot2dfu` flashing method, SD-card apps):
**[docs/setup.md](docs/setup.md)**. Prebuilt binaries: **[Releases](https://github.com/sarollahi/NOX/releases)**.

## Features

| Area | Highlights | Reference |
|---|---|---|
| SubGHz | read/replay/bruteforce, ProtoView, ProtoPirate, wardriving, TPMS, POCSAG | [features.md](docs/features.md) |
| NFC 13.56 MHz | read/emulate, mfkey, Magic, PicoPass, Seader, Metroflip, ChaosID | [features.md](docs/features.md) |
| WiFi / BLE | Marauder, Ghost ESP, Evil Portal, BLE Spam, deauth, FlipperHTTP suite | [features.md](docs/features.md) |
| 2.4 GHz / UHF | nRF24 Mousejack/sniff, UHF-RFID (YRM100), Wiegand | [features.md](docs/features.md) |
| Infrared | learn/replay remotes, universal TV/AC database (plug-in IR module) | [features.md](docs/features.md) |
| USB / iButton | BadUSB, HID, U2F, iButton read/emulate/fuzz | [features.md](docs/features.md) |

Full catalogue (147 apps + 86 plugins): **[docs/app-catalog.md](docs/app-catalog.md)**.

## Plug-in add-on bays

- **Bay 1 — Serial (5-pin):** ESP32 **or** UHF-RFID **or** Wiegand (all share the UART).
- **Bay 2 — SPI (9-pin):** NFC **and/or** nRF24 — both coexist with SubGHz (shared SPI is time-sliced; RF-switch GPIO parked off PE4).
- **Bay 3 — IR (2-wire):** IR LED (PB9) + 38 kHz receiver (PA0), always available.

Wiring per module: **[docs/hardware/plug-in-headers.md](docs/hardware/plug-in-headers.md)**.

## Limitations

- **No 125/134 kHz LF-RFID** — no analog LF front-end, and those pins aren't bonded on the package. Excluded, not broken. (Infrared *is* supported — plug-in IR module on the stock-Flipper IR pins.)
- **No vibro / battery-% ADC** — those two pins were reassigned to free IR (buzzer stays).
- **One shared SPI bus** — some peripheral combinations are mutually exclusive.
- **Pin-starved** — the add-on bays reuse pins; run one serial device at a time.

Full detail with the *why*: **[docs/limitations.md](docs/limitations.md)**.

## Documentation

| I want to… | Read |
|---|---|
| Wire the board & modules | [pinout.md](docs/hardware/pinout.md) · [plug-in-headers.md](docs/hardware/plug-in-headers.md) |
| Build & flash | [setup.md](docs/setup.md) |
| See every feature | [features.md](docs/features.md) |
| Browse all apps | [app-catalog.md](docs/app-catalog.md) |
| Understand the trade-offs | [limitations.md](docs/limitations.md) |
| Use it responsibly | [SECURITY.md](SECURITY.md) |

## Credits & license

NOX is GPLv3 ([LICENSE](LICENSE)). The firmware core derives from the
[Unleashed](https://github.com/DarkFlippers/unleashed-firmware) /
[Flipper Zero](https://github.com/flipperdevices/flipperzero-firmware) codebase (GPLv3) — their
copyright notices are retained throughout the tree as the license requires. The custom `f55`
hardware target, the shared-bus/​input/​power adaptations, the ported apps under `applications_user/`,
and the hardware design are the work of this project. Bundled third-party apps keep their own
authorship and licenses.
