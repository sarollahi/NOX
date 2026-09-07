# Setup

## 1. Build the firmware

This repo is a complete firmware tree — the `f55` target, the patched input service and the
ported apps are all committed here. Build it in place with `fbt`:

```bash
git clone https://github.com/sarollahi/NOX.git
cd NOX
./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
```

A plain clone is enough. The upstream submodules are vendored as ordinary files here, so there
is nothing to `git submodule update`. The first build downloads the pinned ARM toolchain
(gcc-arm-none-eabi 12.3, fbt toolchain 39) into `./toolchain`, which takes a few minutes and
needs about 1 GB. Output:

```
dist/f55-C/flipper-z-f55-full-local.bin    # flash at 0x08000000
dist/f55-C/flipper-z-f55-full-local.dfu    # or install with qFlipper
dist/f55-C/f55-update-local/               # full update bundle (fw + radio + resources)
```

**Expected one-time SDK-API step.** The `f55` target ships its own `furi_hal_resources.h`, so
the first build can stop with *"SDK version is not finalized … API version is still WIP"*. The
firmware compiled — this is the normal Flipper API-tracking prompt. Accept the marks and re-run:

```bash
sed -i '' -E 's/^([A-Za-z]+),\?,/\1,+,/; s/^Version,v,/Version,+,/' targets/f55/api_symbols.csv
./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
```

Other useful targets: `./fbt TARGET_HW=55 fw_dist` (raw firmware),
`./fbt TARGET_HW=55 flash_usb_full` (flash over USB DFU + install resources),
`./fbt TARGET_HW=55 copro_dist` (BLE radio stack).

> 🔴 **Keep the firmware at or below ~812 K.** The BLE radio stack sits at the top of the same
> 1 MB flash and the linker does **not** reserve it — a bigger image silently overwrites it and
> the board dies with no USB. See `targets/f55/README_NOX.md` §10.2.

Prebuilt binaries are attached to each [GitHub release](https://github.com/sarollahi/NOX/releases)
(verify against `SHA256SUMS.txt`).

## 2. First-board preparation (once per board)

1. **OTP** — write the Flipper OTP to `0x1FFF7000` (qFlipper-OTP tool + STM32CubeProgrammer, hold BOOT0 at connect).
2. **Radio stack** — install the **light** BLE copro stack (leaves room for SubGHz + BLE).
   Flash via qFlipper, or `./fbt TARGET_HW=55 copro_dist`. Needs a connect-under-reset SWD
   link (soldered DEBUG header).
3. **microSD** — FAT32, then copy the SD apps (step 4).

## 3. Flash the firmware — `reboot2dfu` race (reliable method)

SWD over jumpers and the BOOT0 button are both flaky on this board. The reliable path is to race
the software-DFU window:

```bash
# 1. On the Flipper CLI (/dev/cu.usbmodemflip_* or your port):
power reboot2dfu

# 2. Immediately (within ~6 s), start the flash — beginning the transfer holds it in DFU:
STM32_Programmer_CLI -c port=usb1 -w dist/nox-firmware.bin 0x08000000 -v --run
```

Or, with qFlipper: **Install from file** → `dist/nox-firmware.dfu`.

## 4. SD-card apps

The firmware alone gives you the core UI and compiled-in apps. Everything else ships as SD
resources. A default `f55` build deploys **255 `.fap` apps and 130 standalone `.fal` plugins**:
every manifest that declares `f55` (or `all`) support and is not a debug app or an example.
After the build in step 1, copy the generated resources onto the card:

```bash
cp -R build/f55-firmware-C/resources/. /Volumes/<YOUR_SD>/
```

(`flash_usb_full` does the same thing over USB without mounting the card.)

There is no build flag that ships a smaller curated subset. The only filter fbt applies is
"not a debug app, not an example", so a fresh build gives you everything that supports `f55`.
To trim the set, delete `.fap` files under the SD `apps/<Category>/` folders afterwards; no
rebuild or reflash is needed. The `★` marks in [app-catalog.md](app-catalog.md) record one such
hand-trimmed card, not a build option.

## 5. Wire the modules

Core wiring: [hardware/pinout.md](hardware/pinout.md). Swappable modules
(ESP32 / UHF / Wiegand / NFC / nRF24): [hardware/plug-in-headers.md](hardware/plug-in-headers.md).

## 6. ESP32 companion firmware

WiFi apps (Marauder, Ghost ESP, the FlipperHTTP suite) need firmware on the ESP32 itself. Flash it
**from the Flipper** with the on-board **ESP Flasher** app, or with the vendor flashers:

- Marauder — <https://github.com/justcallmekoko/ESP32Marauder>
- Ghost ESP — <https://github.com/jaylikesbunda/Ghost_ESP>
- FlipperHTTP (FlipWiFi/Weather/etc.) — <https://github.com/jblanked/FlipperHTTP>

Wire: ESP32 TX→PB7, RX←PB6, GND common, ESP32 on 5V (Serial bay).
