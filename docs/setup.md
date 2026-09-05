# Setup

## 1. Build the firmware

```bash
git clone https://github.com/sarollahi/NOX.git
cd NOX
./scripts/build.sh
```

The script clones Unleashed, installs the `f55` target, registers `TARGET_HW=55`, patches the
shared input service, vendors the ported apps, and builds. First run downloads the ARM toolchain
(a few minutes). Output:

```
dist/nox-firmware.bin   # flash at 0x08000000
dist/nox-firmware.dfu   # or install with qFlipper
```

Prebuilt binaries are attached to each [GitHub release](https://github.com/sarollahi/NOX/releases)
(verify against `SHA256SUMS.txt`).

## 2. First-board preparation (once per board)

1. **OTP** — write the Flipper OTP to `0x1FFF7000` (qFlipper-OTP tool + STM32CubeProgrammer, hold BOOT0 at connect).
2. **Radio stack** — install the **light** BLE copro stack (leaves room for SubGHz + BLE).
   Flash via qFlipper, or `./fbt TARGET_HW=55 copro_dist` in the Unleashed clone. Needs a
   connect-under-reset SWD link (soldered DEBUG header).
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

The firmware alone gives you the core UI and compiled-in apps. The full app set (147 `.fap` +
86 plugins) ships as SD resources. In your Unleashed build tree after `build.sh`:

```bash
cp -R .build/unleashed/build/f55-firmware-C/resources/. /Volumes/<YOUR_SD>/
```

To add or remove individual apps later, drop/delete `.fap` files under the SD `apps/<Category>/`
folders — no rebuild or reflash needed. See [app-catalog.md](app-catalog.md).

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
