# Target `f55` — Unleashed on WeAct STM32WB55CGU6 (NOX)

Retargets Unleashed firmware to a discrete-module "Flipper" built on a **WeAct Studio
STM32WB55CGU6** board (UFQFPN48), per the den4ik/JustEnotik DIY wiring, extended with an
**ST25R3916** NFC module.

This target **inherits `f7`** and overrides only the board HAL files, so every Unleashed
app and feature comes along unchanged. What changes is purely the hardware map:

- `target.json` — inherits `7`, links lib `flipper55`.
- `furi_hal/furi_hal_resources.{c,h}` — pin map + direct-GPIO buttons.
- `furi_hal/furi_hal_spi_config.c` — **all four SPI peripherals collapsed onto one bus (SPI1)**; external-SPI handle repointed off button/CS pins (§7).
- `furi_hal/furi_hal_sd.c` — SD SPI alt-function `SPI2 → SPI1`; SD auto-mount **disabled** (`is_present()` returns false) because no card-detect line exists and retry-mounting froze the UI (§10.3).
- `furi_hal/furi_hal_power.c` — battery gauge/charger probe made non-blocking (§7).
- `furi_hal/furi_hal_subghz.c` — every op no-ops when CC1101 is absent instead of crashing (§7).

Plus `site_scons/commandline.scons` registering `TARGET_HW=55`, and a one-line honesty fix in
the shared `lib/subghz/devices/cc1101_int/cc1101_int_interconnect.c` (`is_connect` actually
probes the chip instead of hardcoding `true`).

> ✅ **Status: BENCH-VERIFIED on real hardware.** Builds, flashes, boots, and runs: display,
> buttons, USB/CLI, and the BLE radio stack all confirmed working on a WeAct WB55CGU6.
> Firmware **812,604 bytes** is the known-good size.
>
> 🔴 **Do not exceed ~812 K** — the BLE radio stack sits at the top of the same 1 MB flash and the
> linker does **not** reserve it (see §10.2). Bigger builds silently overwrite it → dead board, no USB.
>
> Both required crystals (32 MHz HSE, 32.768 kHz LSE) are populated on this board — see §8.

---

## 1. Wiring (this target's pinout)

All of CC1101, the LCD, the SD card and the ST25R3916 share **one SPI bus (SPI1)**:

| Signal            | MCU pin | Notes                                    |
|-------------------|---------|------------------------------------------|
| **SPI SCK**       | PB3     | shared clock (AF5/SPI1)                  |
| **SPI MOSI**      | PB5     | shared (AF5/SPI1)                        |
| **SPI MISO**      | PB4     | shared (AF5/SPI1)                        |

| Module            | Line    | MCU pin |
|-------------------|---------|---------|
| **CC1101** SubGHz | CSN     | PA7     |
|                   | GDO0    | PA1  (EXTI1) |
| **LCD (ST756x)**  | CS      | PB2     |
|                   | RST     | PB0     |
|                   | RS/DC   | PB1     |
|                   | backlight | tie to **3V3** (no LED driver on board) |
| **SD card**       | CS      | PA4     |
|                   | DET     | PA3  (Adafruit 4682: HIGH=card in) |
| **ST25R3916** NFC | CS      | PA8     |
|                   | IRQ     | PA0  (EXTI0, rising) |
| **ESP32 (UART)**  | TX→esp RX | PB6 (USART1, 115200 8N1) |
|                   | RX←esp TX | PB7 |

**Direct-GPIO peripherals** (freed once the buttons moved to the '165):

| Function | MCU pin | Wiring |
|----------|---------|--------|
| **Speaker** | PA6 | TIM16_CH1/AF14 → piezo, or small speaker via NPN + flyback |
| **Vibro**   | PA5 | motor via NPN (base 1kΩ) + flyback diode; not straight to the pin |
| **iButton** | PA10 | 1-Wire data + **1kΩ pull-up to 3V3** + contact ring. (Was PH3 — moved: PH3 is BOOT0.) |
| **Battery %** | PA2 | 100k/100k divider from **raw 18650(+)** to PA2, 100nF PA2→GND. ADC1_IN7. |

**Buttons — via one SN74HC165 (PISO shift register).** All 6 buttons hang off the '165's
parallel inputs; the MCU reads them serially through 3 pins (the input service polls at 10 ms):

| '165 pin | → MCU | | '165 input | Button |
|----------|-------|-|-----------|--------|
| PL (SH/LD, pin1) | PB8 | | D0 (A) | UP |
| CP (CLK, pin2)   | PB9 | | D1 (B) | DOWN |
| Q7 (QH, pin9)    | PA15 | | D2 (C) | RIGHT |
| CE (pin15) | GND | | D3 (D) | LEFT |
| DS (pin10) | GND | | D4 (E) | OK |
| VCC/GND | 3V3/GND | | D5 (F) | BACK |

- Each button: '165 input → button → **GND**, with a **10kΩ pull-up to 3V3** on the input
  (active-low). Tie unused **D6 (G) / D7 (H) to 3V3**.
- Q7 has an MCU pull-up: an **unwired '165 reads as all-released**, so a missing board never
  fires phantom keys. Verified bit-order/keymap with a host self-check (`/tmp/test_165.c`).

> **Leave PH3 unconnected.** It's BOOT0; the WeAct carrier's pull-down + BOOT0 button handle
> flash-vs-DFU boot. Nothing (not even iButton — that's why it moved to PA10) goes on PH3.
> PA9 (ex power-I²C SCL) and PE4 (blue LED) are the only remaining spares.

## Expansion header (1×14 female, rear wall)

Pin 1 is marked with a triangle on the case, next to the outermost hole. Body is 35.96 mm
long (2.54x14+0.4); pin 1 to pin 14 span is 33.02 mm. Order follows the
Flipper Zero GPIO header so add-on wiring guides translate directly.

| pin | signal | STM32 | notes |
|----:|--------|-------|-------|
| 1 | 5V | VBUS / charger 5V out | power hungry add-ons (ESP32) draw from HERE, not 3V3 |
| 2 | GND | | |
| 3 | 3V3 | ME6231 LDO | shared with display, CC1101, SD — keep add-on draw under ~100 mA |
| 4 | TX | PB6 (USART1 TX) | → add-on RX. Marauder / expansion protocol, 115200 8N1 |
| 5 | RX | PB7 (USART1 RX) | ← add-on TX |
| 6 | SCL | PA9 (I2C1) | |
| 7 | SDA | PA10 (I2C1) | |
| 8 | GPIO | PA0 | ADC-capable; ex-NFC IRQ (no NFC module fitted) |
| 9 | GPIO | PA3 | ADC-capable |
| 10 | GPIO | PA8 | ex-NFC CS (no NFC module fitted) |
| 11 | NRST | | reset; lets SWD connect-under-reset |
| 12 | SWDIO | PA13 | reflash / debug with the case closed |
| 13 | SWCLK | PA14 | |
| 14 | GND | | spare ground — 13-pin parts are not sold, so a 14-pin is used |

**ESP32 WiFi add-on (Marauder):** ESP TX→pin 5, ESP RX→pin 4, GND→pin 2, and power the ESP
dev board from **pin 1 (5V)** into its own 5V/VIN input so its onboard regulator carries the
WiFi TX peaks. Do not run an ESP32 from pin 3. Use the "ESP32 WiFi Marauder companion" FAP.

Firmware follow-up (batched into the next reflash): list PA0/PA3/PA8 in `gpio_pins[]` so the
GPIO apps can drive them; PB6/PB7 and PA9/PA10 already carry USART1 / I2C1.

Power: `G`→GND, `3V3`→module VCC.

Reserved / do not repurpose: PA13/PA14 (SWD), PA11/PA12 (USB), PB6/PB7 (debug UART, optional),
PA9/PA10 (power-I²C, no devices — left idle), PE4 (WeAct on-board **blue LED**, D1 via 5.1kΩ —
firmware parks unused peripherals here; it may just glow, harmless), PH3 (BOOT0 strap).

Power the modules from the **3V3** header pins (regulated by the on-board ME6231 LDO), NOT the
**5V/VB** pins (those are USB VBUS). Feed 5V to no module.

---

## 2. Build

From the repo root, with the normal Unleashed/fbt toolchain:

```bash
./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
```

Output (flashable) lands in `dist/f55-update-<suffix>/` including the `flipper-z-f55-*.dfu`
and the full update bundle. Other useful targets: `./fbt TARGET_HW=55 fw_dist` (raw fw),
`./fbt TARGET_HW=55 flash_usb_full` (flash over USB DFU + install resources).

The BLE coprocessor stack is unchanged from `f7` — flash the same `copro` stack Unleashed
ships (`./fbt TARGET_HW=55 copro_dist` / qFlipper) at the stock radio-stack address.

**Expected one-time SDK-API step.** Adding a target with its own `furi_hal_resources.h` shifts
the tracked SDK API surface, so the first build stops with *"SDK version is not finalized …
API version is still WIP"*. This is **not** a code error (the firmware compiled). Accept the
auto-generated marks and re-run:
```bash
sed -i '' -E 's/^([A-Za-z]+),\?,/\1,+,/; s/^Version,v,/Version,+,/' targets/f55/api_symbols.csv
./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
```
(On an older checkout without `targets/f55/api_symbols.csv`, run the same `sed` on
`targets/f7/api_symbols.csv`.) `bootstrap_nox.sh` does this automatically.

---

## 3. First-time board prep (once per board)

1. **OTP is OPTIONAL — recommended to skip it.** A blank STM32 OTP reads `0xFFFFFFFF`; the
   firmware detects "empty" and loads default identity (no crash). OTP is a **one-time,
   irreversible** write, so leave it blank unless you have a reason. The board boots and runs
   fine with default name/region. (If you *do* want a custom name: qFlipper-OTP tool, values
   Version 12 / Firmware 7 / Body 9 / Connection 6, display `mgg`, write with STM32CubeProgrammer
   at `0x1FFF7000`. Get it right the first time — it can't be rewritten.)
2. **Flash firmware**: the `.dfu` from §2 via qFlipper ("Install from file") or STM32CubeProgrammer.
   Enter DFU by holding the WeAct **BOOT0** button at power-on (not the OK button).
3. **SD card**: format FAT32, copy the Unleashed resources/databases so apps don't error on first run.

---

## 4. Feature support (hardware-limited, not firmware-limited)

| Feature | State |
|---|---|
| SubGHz (CC1101) | ✅ supported |
| NFC (ST25R3916) | ✅ supported |
| LCD + full UI | ✅ supported |
| microSD | ✅ supported |
| 6-key navigation | ✅ via SN74HC165 (PL PB8 / CP PB9 / Q7 PA15), 10 ms poll |
| BLE | ✅ (light copro stack) |
| USB (CDC/HID/U2F) | ✅ |
| ESP32 / UART add-ons | ✅ USART1 PB6/PB7 (Marauder etc. as `.fap` on SD) |
| iButton / 1-Wire | ✅ PA10 + 1kΩ pull-up (app is MENUEXTERNAL `.fap`) |
| Speaker | ✅ PA6 (TIM16_CH1/AF14) |
| Vibro | ✅ PA5 |
| Battery % | ✅ ADC on PA2 (100k/100k divider), no gauge chip |
| 125 kHz LF RFID | ❌ no LF coil/driver + PB13/PC5 not on package |
| Infrared | ❌ dropped (no free timer pin left) |

All the above are compiled in. Battery % comes from the ADC path (`furi_hal_power`
falls back to it whenever no BQ27220 gauge answers). Everything not fitted still opens in the
menu and simply senses/drives nothing — missing hardware never hangs or bricks (see §7).

> **Note — the buttons and these peripherals share pins by design.** Moving the 6 buttons onto
> the '165 is what freed PA2/PA5/PA6 (battery/vibro/speaker) and PB8/PB9/PA15 (the '165 lines).
> The '165 is therefore **required** for this build; without it you have no navigation.

---

## 5. Known bench-tuning knobs / gaps

- **SD card-detect** is parked on PA15 (no CD line on cheap SD modules); mounting relies on
  SPI probe. If the card won't mount, force "present" or wire a real CD pin.
- **LCD contrast/bias**: cheap ST756x panels vary. If the image is blank/ghosted, adjust the
  u8g2 contrast/bias in the display init (shared `lib/`), not this target.
- **Single-bus SPI is serialized** by one mutex (required — a second enable of SPI1 hard-crashes
  via `furi_check`). Heavy concurrent NFC + display + SD activity will be slower than a stock
  Flipper's two-bus design. This is inherent to the wiring, not a bug.
- **`gpio_pins[]` external-header table** is inherited from f7 and still lists pins now used by
  modules (PA4/PA6/PA7/PB2/PB3). The GPIO app could drive them into conflict; trim the table if
  you expose a user GPIO header.

---

## 6. Optional: buttons via SN74HC165 (PISO shift register)

If you build the shift-register button board instead of 6 direct GPIOs (frees pins, the DIY
"recommended" wiring), it needs a small driver — it is **not** wired up in this target yet
because it touches shared code and can't be validated without the hardware. The two seams:

1. `applications/services/input/input.c` — the per-key **read** is a one-line GPIO read
   (~line 37) and the idle **wait** is a forever-blocking EXTI thread-flag (~line 185).
2. Replace those two with: a ~5–10 ms periodic poll that latches the '165 (PL low pulse),
   clocks out 8 bits (CP ~1 MHz), and maps 6 bits to the 6 `InputKey`s. Reuse the existing
   debounce/short/long/repeat/pubsub machinery verbatim.

Suggested '165 pins: PL, CP, Q7 on three free GPIOs (e.g. PA3/PA15/PE4 if you free them from
parking); tie `/CE` and `DS` to GND; 10 kΩ pull-ups on the 6 buttons. This is the "may lag"
tradeoff noted in the DIY readme — the direct-GPIO map above avoids it.

---

## 7. Missing hardware — does it brick?

Flipper supports many peripherals you don't have. Audited every boot-time init for a
`furi_check`/`furi_crash` that fires when a chip is absent:

| Boot init | Chip absent behavior | Boot-safe? |
|---|---|---|
| `furi_hal_power_init` | I²C gauge (BQ27220) + charger (BQ25896) | ✅ **fixed here** |
| `furi_hal_subghz_init` (CC1101) | GDO0 self-test times out → state `Broken`, apps no-op (fixed here) | ✅ no crash |
| `furi_hal_nfc_init` (ST25R3916) | chip-ID mismatch → logs "Wrong chip id", returns error | ✅ no crash |
| `furi_hal_sd` (microSD) | SPI probe fails → "SD not mounted" | ✅ no crash (hot-plug) |
| `furi_hal_bt_init` (BLE copro) | on-chip core2; no radio stack → BLE unavailable | ✅ no crash |
| `rfid` / `ibutton` / `speaker` / `vibro` init | configure MCU peripherals only, **no external probe** | ✅ no crash |

Why I²C is safe: `furi_hal_i2c_tx/rx` **return `bool`** (false on NACK) — they do not assert on
a missing device. The gauge/charger just report `false` and boot continues.

**The one thing I fixed:** stock `furi_hal_power_init` retried the gauge/charger probe with
`furi_delay_us(4000000)` + `furi_delay_us(2020202)` on each I²C failure — so with **no battery
IC on the board, boot froze ~12 s** every power-on (not a crash, but looks like one). This
target does a single non-blocking probe → instant boot. (Add a real gauge later → restore the
delay, see the `ponytail:` note in `furi_hal_power.c`.)

### Per-app safety (incremental bring-up)

You will assemble modules one at a time. Opening an app whose chip isn't wired should not
crash — handled per subsystem:

- **SubGHz (CC1101) — fixed here.** `furi_hal_subghz.c` is overridden so every operation
  no-ops when the boot self-test failed (`state == SubGhzStateBroken`): `idle`, `rx`, `tx`,
  `sleep`, `start/stop_async_rx`, `start/stop_async_tx`, `is_async_tx_complete`. Stock code
  hard-`furi_check`ed a `cc1101_wait_status_state()` timeout / `state == Idle` and crashed to
  desktop. Now the SubGHz app opens with no CC1101 wired, shows no signal, no crash. Also fixed:
  the internal-radio `is_connect()` in `lib/subghz/.../cc1101_int_interconnect.c` was a stub
  hardcoded to `true`; it now actually probes `cc1101_get_partnumber`, so presence checks are
  honest (unchanged on a real Flipper where the chip answers).
- **NFC (ST25R3916).** Poller/listener HAL uses an error-return model (`FuriHalNfcError`), not
  `furi_check` on chip comms — reads to an absent chip time out and return errors the app
  surfaces as "no tag". No boot brick, no hard crash. Not overridden.
- **LF-RFID / IR / iButton / speaker.** Apps open; sense/drive nothing (pins parked on PE4).
  Their HAL init touches MCU peripherals only, no external probe → safe.

**Net:** with your set (CC1101 + ST25R3916 + LCD + SD + buttons) it boots to the desktop and
every unbuilt peripheral is quiet. Nothing you leave unconnected bricks the boot, and you can
open SubGHz/NFC before their chips are wired without a crash.

---

## 8. Crystals — required, and CONFIRMED PRESENT on your board

The clock/RTC HAL (inherited from stock Flipper) hard-depends on two crystals; missing either =
unbootable with no clean error. **Verified against the WeAct WB55CoreBoard V1.0 schematic — both
are populated on your board:**

| Crystal | Board part | Pins | Status |
|---|---|---|---|
| **HSE 32 MHz** | X2 (XTAL2016, ±10 ppm) | OSC_IN/OSC_OUT | ✅ present |
| **LSE 32.768 kHz** | X1 (NX3215SA-32.768K) + 12 pF caps | PC14/PC15 | ✅ present |

So the top boot-failure cause does **not** apply to your board — it has the LSE that bare WB55
modules often omit. (For reference, if either were missing: no HSE = silent dead hang in
`furi_hal_clock_init` with no timeout/no HSI fallback; no LSE = red-LED blink → reset loop.)

Also verified from the schematic:
- **BOOT0 (PH3)** has a 10 kΩ pull-down (R1) + the on-board BOOT0 button → boots from flash
  reliably. Leave PH3 unwired; hold the **BOOT0 button** at power-on to enter DFU for flashing.
- **VBAT** is fed from 3V3 through a BAT54C (D2) → no battery or VBAT wiring needed.
- **PE4** is the on-board blue LED (D1, via 5.1 kΩ), not a button — safe as the park pin.
- **USB-C** (PA11/PA12, CC resistors present) for flashing; **SWD** on the DEBUG header
  (PA13/PA14); on-board **ME6231 3V3 LDO** powers the 3V3 rail.

Everything else is optional for boot: **OTP** may stay blank (defaults load), and **no BLE radio
stack** is fine (`bt_init` is async, never blocks) — BLE is simply unavailable until you flash one.

---

## 9. Board-safety audit summary (does it damage the MCU?)

Audited every firmware-driven pin against this wiring for short-circuit / over-drive:

- **No boot-time damage path.** Every push-pull output at boot (display RST/DC/CS, the CS lines
  PA7/PA8/PA4/PB2, shared SPI1 AF pins PB3/4/5) drives only its own net — none fights a rail or
  another output. Buttons are only ever Input+pull-up, never driven. Disconnecting any module
  just floats its own CS/reset line — never turns an output into a damaging state.
- **Fixed a latent short:** the inherited external-SPI handle mapped SPI outputs onto **PA6
  (RIGHT button)** and **PA7 (CC1101 CS)** — if any app (e.g. the JS SPI module) opened that bus
  it would drive the RIGHT-button pin push-pull (a short to GND when pressed). Repointed to the
  shared bus with CS on the dead card-detect pin, so it can't clobber a button or a live CS.
- **PE4 is the WeAct on-board blue LED** (D1 via 5.1 kΩ), the park pin for absent
  speaker/vibro/IR/etc. Driving it just lights the LED through 5.1 kΩ (~0.5 mA) — harmless,
  no damage. Leave it unwired.

**Conclusion:** nothing in this firmware physically damages the board on the §1 wiring, wired
correctly. The real risks are *boot* (crystals, §8) and *miswiring*, not silicon damage.

---

## 10. Bring-up log — two bugs that cost real time

### 10.1 Shared-bus pins were being killed (display died permanently)
**Symptom:** display worked for a moment at boot, then went blank forever. No reset, rewire,
new breadboard, or new wires helped. Board stayed alive (CLI answered) the whole time.

**Cause:** on this board every SPI device shares **one** bus (PB3/PB4/PB5). The SubGHz and NFC
handles reconfigure those pins when they release the bus:
- SubGHz `Deactivate` → `miso/mosi/sck` set to **Analog**
- NFC `Deinit` → `sck`=input, `mosi`=**push-pull LOW** (st25r3916 "transparent mode")

Stock Flipper has two separate buses, so this is harmless there. Here those are the display's
clock and data. The display handle configured its pins **once at Init** and never again, so the
first SubGHz/NFC bus access killed the display permanently.

**Fix (in `furi_hal_spi_config.c`):** the display/SD handle now re-applies SPI1 AF to
`miso/mosi/sck` on **every** `Activate`, so it self-heals regardless of what the radio handles
leave behind. Any future device added to this shared bus must not assume pin state persists.

### 10.2 Firmware overran the BLE radio stack (board dead, no USB)
**Symptom:** after enabling extra apps, the board stopped enumerating on USB entirely.

**Cause:** the NFC parsers / CLI plugins compile **into** the firmware. Enabling them grew it
812 K → 864 K. The BLE radio stack lives near the top of the same 1 MB flash, and **the linker
does not reserve it** — it happily reported "180 K free" that was actually the radio stack. The
larger image overwrote it, CPU2 got garbage, and the board hardfaulted before USB init.

**Rule:** keep firmware at/below **~812 K** unless you first read the *real* radio-stack address
off the chip (STM32CubeProgrammer) and trim to fit below it. The build's
`Using guessed radio address 0x080D7000` is a **guess** — do not trust it. Dropping BLE would
reclaim ~150 K if more app space is ever needed.

### 10.3 Current known-good configuration
- Firmware **812,604 bytes** — display, buttons, USB/CLI, BLE stack all working.
- Apps: stock set only (no extra NFC parsers / CLI plugins — they overflow flash).
- **SD disabled** (`furi_hal_sd_is_present()` returns false) — the blue AMS1117 SD module never
  mounted; leaving auto-mount on caused repeated multi-second UI freezes.
- **NFC not used** — PA8 (CS) and PA0 (IRQ) are free.
- Buttons: UP=PB8, DOWN=PA5, LEFT=PA2, RIGHT=PA6, OK=PA15, BACK=PB9 (4-pin tactile: use
  **diagonal** legs, other side to GND).
