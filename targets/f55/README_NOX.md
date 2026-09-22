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
|                   | IRQ     | PA2  (ex battery-ADC pin) |
| **Infrared**      | TX      | PB9 (TIM1_CH3N) |
|                   | RX      | PA0 (TIM2_CH1) |
| **ESP32 (UART)**  | TX→esp RX | PB6 (USART1, 115200 8N1) |
|                   | RX←esp TX | PB7 |

**Direct-GPIO peripherals** (freed once the buttons moved to the '165):

| Function | MCU pin | Wiring |
|----------|---------|--------|
| **Speaker** | PA6 | TIM16_CH1/AF14 → piezo, or small speaker via NPN + flyback |
| **iButton** | PA10 | 1-Wire data + **1kΩ pull-up to 3V3** + contact ring. (Was PH3 — moved: PH3 is BOOT0.) |

**Dropped to make room for Infrared** (see `git show 3df4a7ac`): **Vibro** (was PA5, now the
'165 clock; parked on non-bonded PC1) and **Battery %** (was the PA2 ADC divider, now the NFC
IRQ). Nothing needs wiring for either.

**Buttons — via one SN74HC165 (PISO shift register).** All 6 buttons hang off the '165's
parallel inputs; the MCU reads them serially through 3 pins (the input service polls at 10 ms):

| '165 pin | → MCU | | '165 input | Button |
|----------|-------|-|-----------|--------|
| PL (SH/LD, pin1) | PB8 | | D0 (A) | UP |
| CP (CLK, pin2)   | PA5 | | D1 (B) | DOWN |
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
> No general-purpose spares are left: PA9 (ex power-I²C SCL) is the parked external-SPI CS and
PE4 (blue LED) is the nRF24 CE.

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
| 8 | GPIO | PA0 | **now Infrared RX** (TIM2_CH1) — not a free GPIO |
| 9 | GPIO | PA3 | SD card-detect (Adafruit 4682 DET) — not a free GPIO |
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
# keep the API MAJOR equal to f7's -- see below
sed -i '' -E "s/^Version,([+v?]),[0-9]+\./Version,\1,$(awk -F, '$1=="Version"{split($3,v,"."); print v[1]; exit}' targets/f7/api_symbols.csv)./" targets/f55/api_symbols.csv
./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
```
(On an older checkout without `targets/f55/api_symbols.csv`, run the same `sed` on
`targets/f7/api_symbols.csv`.) `bootstrap_nox.sh` does this automatically.

**Never accept the MAJOR bump.** fbt proposes one (`VersionBump.MAJOR`,
`scripts/fbt/sdk/cache.py`) whenever the tracked symbol set shrinks, which it does
transiently while a new target is being wired up. But f55's API is a strict superset of
f7's -- symbols added, none removed or deprecated -- so only the minor should move.
Shipping a major bump makes the firmware export API 89 while every published `.fap`
(app catalog, Unleashed app pack, anything built with ufbt) is API 88; the loader
compares majors only and greets every external app with
*"App Too Old -- APP:88 < FW:89"*. Keep `targets/f55/api_symbols.csv` on the same major
as `targets/f7/api_symbols.csv`.

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
| Infrared | ✅ TX PB9 (TIM1_CH3N) / RX PA0 (TIM2_CH1) |
| nRF24 2.4 GHz | ✅ shared SPI1, CE on PE4 |
| Vibro | ❌ dropped — PA5 became the '165 clock |
| Battery % | ❌ dropped — PA2 became the NFC IRQ; no gauge chip either |
| 125 kHz LF RFID | ❌ no LF coil/driver + PB13/PC5 not on package |

All the above are compiled in. There is no battery sensing at all on this board — no BQ27220
gauge and no ADC divider — so `furi_hal_power_get_pct()` reports a flat 100%. Everything not
fitted still opens in the menu and simply senses/drives nothing — missing hardware never hangs
or bricks (see §7).

> **Note — the buttons and these peripherals share pins by design.** Moving the 6 buttons onto
> the '165 is what freed PA2/PA5/PA6 (battery/vibro/speaker) and PB8/PB9/PA15 (the '165 lines).
> The '165 is therefore **required** for this build; without it you have no navigation.

---

## 5. Known bench-tuning knobs / gaps

- **SD card-detect is a real line on PA3** (Adafruit 4682 DET, **HIGH = card inserted** — the
  opposite polarity to a stock Flipper). Detection gates mounting and the retry count is down to
  3. (PA15 is the '165 Q7 — it is **not** a card-detect pin, and neither is PA9, which is the
  nRF24 chip-select.)

  `furi_hal_sd_is_present()` mostly just reads DET, with one exception for boards where the line
  was never wired at all — plenty of cheap breakouts do not break the pin out:

  1. **DET high** — present, and the line is recorded as working.
  2. **DET low, having read high before** — absent. Byte-for-byte stock behaviour, so removal
     detection, the unmount in `storage_ext_tick_internal()` and the early exit on the retry
     ladders in `furi_hal_sd.c` all keep working.
  3. **DET low, never having read high** — the line may be unconnected, so claim present until
     one real `furi_hal_sd_init()` answers, then report that answer.

  Only case 3 is new, and only reachable where DET has never once worked. An empty slot there
  costs one failed init at boot and is quiet after. A card inserted *after* boot still needs a
  wired DET to be noticed.
- **LCD contrast/bias**: cheap ST756x panels vary. If the image is blank/ghosted, adjust the
  u8g2 contrast/bias in the display init (shared `lib/`), not this target.
- **Single-bus SPI is serialized** by one mutex (required — a second enable of SPI1 hard-crashes
  via `furi_check`). Heavy concurrent NFC + display + SD activity will be slower than a stock
  Flipper's two-bus design. This is inherent to the wiring, not a bug.
- **`gpio_pins[]` is already trimmed for this board.** This target overrides
  `furi_hal_resources.c`, and its table exposes only the four debug pins (PA13/PA14 SWD, PB6/PB7
  USART1) with `debug = true` so they are not driven to analog at boot. The f7 table that listed
  PA4/PA6/PA7/PB2/PB3 does not apply here, and PC0/PC1/PC3 are deliberately omitted because they
  are not bonded on UFQFPN48. Consequence worth knowing: the GPIO app cannot reach the expansion
  header pins, and the `gpio` CLI cannot read PA3, so SD card-detect cannot be probed from the
  command line.
- **`cc1101_ext` is deliberately not built for f55 — leave it that way.**
  `applications/drivers/subghz/application.fam` declares `targets=["f7"]`, so the external-CC1101
  plugin never compiles here. That is load-bearing: its pins are hardcoded for a real Flipper's
  expansion header and are wrong on this board — `g0_pin` is **PB2, the LCD CS**, and the E07 amp
  line is PC3. Apps that request the external radio get `NULL` from
  `subghz_devices_get_by_name()` and fall back to the internal CC1101 on PA7, which is the
  correct behaviour. **Do not add `"f55"` to that fam** without repointing both pins first, or
  the first external-radio TX will drive the display's chip-select.
- **SPI device coexistence is already solved by distinct chip selects; there is no
  `external_extra` bus and none is needed.** Momentum adds a second external-SPI handle
  (`furi_hal_spi_bus_handle_external_extra`) so an external CC1101 and an nRF24 can hold
  separate chip selects. On this board that problem does not arise: every SPI device already
  has its own CS on the single mutex-serialised SPI1 bus.

  | Device | Handle | CS | Extra lines |
  |---|---|---|---|
  | CC1101 SubGHz (internal) | `..._subghz` | PA7 | GDO0 PA1 |
  | LCD / SD | `..._display` / `..._sd_*` | PB2 / PA4 | SD DET PA3 |
  | ST25R3916 NFC | `..._nfc` | PA8 | IRQ PA2 |
  | nRF24 | `..._external` | PA9 | CE PE4 |

  A fourth handle would need a fourth chip-select pin, and this package has none free (see the
  spares note in §1), so adding one could only park it on a pin that already means something.
  The RF-switch GPIO parked on non-bonded PC3 is not a CS conflict either - it is an unused
  control line for a switch this board does not have, and non-bonded is the correct place for
  it. Combination support (SubGHz + NFC, SubGHz + nRF24, ESP32 + NFC, ...) is listed in
  `docs/hardware/plug-in-headers.md`.
- **There is no internal filesystem, and `/int` is redirected onto the SD card.** Only
  `storage_ext.c` exists; `storage.c` initialises `ST_EXT` alone and `storage_processing.c`
  treats only `ST_EXT` as a valid type. `storage_process_alias()` rewrites any `/int/...` path to
  `/ext/.int/...` and creates that directory on demand.

  This is the most surprising consequence on this board: **with no SD card, no settings survive a
  reboot.** Every settings file uses `INT_PATH(...)` — desktop, notification, power, input,
  expansion, BT keys, dolphin state — so all of them resolve onto the card, and
  `..._settings_load()` fails on every boot and zeroes the struct to defaults.

  Within one session it depends on who applies the setting. The settings app calls
  `desktop_api_set_settings()` on exit, which hands the struct to the desktop service and applies
  it from RAM, so Show Clock, the Lock Screen Clock options, dummy mode and auto-lock all take
  effect and hold until reboot. Battery View and Hide Battery Icon do not: the scene's exit
  handler calls `power_trigger_ui_update()`, which re-reads from storage, fails, and reverts them
  on the spot.

  Porting a real internal filesystem is not an option here: it would need flash pages, and only
  about 15 KB is free below the BLE stack.

- **`furi_hal_power_{enable,disable}_external_3_3v()` are no-ops on this target.** There is no
  switchable 3V3 rail (the ME6231 LDO is always on) and the pin those functions drive,
  `gpio_periph_power`, is **PE4** here — blue LED, park pin, and the **nRF24 CE**. Left active
  they would glitch CE whenever `furi_hal_sd_init()` power-cycles the card on a mount retry, and
  `power 3v3 on/off` in the CLI would just blink the LED. PE4 is parked once at init and then
  belongs to whichever driver claims it.
- **`furi_hal_adc` uses VDDA, not VREFBUF** (`targets/f55/furi_hal/furi_hal_adc.c`). VREF+ is not
  bonded out on the UFQFPN48 package, so VREFBUF can never become ready; stock code crashed on a
  `furi_check("VREF fail")` after a 500 ms wait. The override references VDDA and calibrates it
  from the internal VREFINT. Accuracy now tracks the 3V3 rail rather than a 2.048 V buffer, so a
  requested `FuriHalAdcScale` is range-checked but not honoured.

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
- **SD enabled via a real card-detect on PA3.** Earlier builds forced
  `furi_hal_sd_is_present()` to false because a card-detect-less module retry-mounted forever
  and froze the UI for seconds at a time. With the Adafruit 4682 DET wired, detection gates
  mounting and that stall is gone.
- **NFC** — CS on PA8, IRQ on PA2. (PA0 is Infrared RX now.)
- Buttons: all six read through the SN74HC165 — **PL=PB8, CP=PA5, Q7=PA15**, buttons on D0..D5
  (D0=Up D1=Down D2=Right D3=Left D4=Ok D5=Back). The old direct-GPIO button map is gone; the
  '165 is required for navigation.
