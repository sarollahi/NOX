<sub>NOX · hand-built Flipper</sub>

# WeAct STM32WB55CGU6 — wiring map

Every pad on the WeAct board, what it drives and which nets are shared, as built into the NOX case.
Everything is hard-wired inside; the 14-pin add-on header is gone. Each pad carries a **letter** as
well as a colour, so it reads even if colour doesn't — colours are colour-blind-safe (blue / orange
/ grey / purple, no red-green). Pin assignments are the `targets/f55` ones (summary in
[pinout.md](pinout.md)); the same pins also work on [plug-in headers](plug-in-headers.md) for a
bench build.

🟦 **D** Direct — 1 wire to the pad · 🟧 **S** Shared — splice / bus strip / rail ·
⬜ **○** Optional / leave · 🟪 **✕** Never wire (BOOT0)

## What changed

*2 Nov 2026 · now matches the case build*

- **Pad rows corrected.** The top row starts with two GND pads (G G 3V3 3V3 …) and the bottom row ends in a GND pad. The old map showed one and none, so every top-row pad from 3V3 on was one place off. Pads are now numbered from the USB end, the same numbers the case's wire list uses.
- **No add-on header.** NFC, nRF24 and the ESP32 are wired straight in.
- **ESP32-C5 Mini inside the case** on PB6/PB7, powered from the charger's 5 V output.
- **NFC is the Elechouse ST25R3916B board**, which takes 5 V on its cable, not 3.3 V.
- **Buzzer has an NPN driver** on PA6, with its own section below.
- **Power wiring added:** battery, switch, charger and where its 5 V goes.

## Board layout

*two 0.1″ header rows · drawn with USB-C at the top*

![WeAct STM32WB55CGU6 pad map: every pad, its category and what it connects to](wiring-map/board-pads.svg)

Small numbers count pads from the USB-C end, as in the case's wire list (which calls the top row
"inner" and the bottom row "outer"). The board has four GND pads and two 3V3 pads. E4 = nRF24 CE
(SubGHz RF-switch parked off it → nRF24 + SubGHz coexist). A9 = nRF24 CSN, doubles as
Wiegand-emulate drive.

<details>
<summary>Same pads as tables</summary>

**Top row · 3V3 side**

| # | Pad | | Function |
|---|---|---|---|
| 1 | G | 🟧 S | GND · charger OUT− |
| 2 | G | 🟧 S | GND · buttons |
| 3 | 3V3 | 🟧 S | +3.3 V · splice |
| 4 | 3V3 | 🟧 S | +3.3 V · buttons, SD |
| 5 | B7 | 🟦 D | UART RX ← ESP32 TX |
| 6 | B6 | 🟦 D | UART TX → ESP32 RX |
| 7 | B5 | 🟧 S | SPI MOSI |
| 8 | B4 | 🟧 S | SPI MISO |
| 9 | B3 | 🟧 S | SPI SCK |
| 10 | A15 | 🟦 D | '165 Q7 (data) |
| 11 | A10 | 🟦 D | iButton |
| 12 | E4 | 🟦 D | nRF24 CE + LED |
| 13 | B1 | 🟦 D | Display DC (RS/A0) |
| 14 | B0 | 🟦 D | Display RST (RSE) |
| 15 | G | 🟧 S | GND · splice |

**Bottom row · 5V side**

| # | Pad | | Function |
|---|---|---|---|
| 1 | G | ⬜ ○ | GND · spare |
| 2 | 5V | ⬜ ○ | USB 5 V |
| 3 | 5V | 🟦 D | 5 V in ← charger OUT+ |
| 4 | VB | ⬜ ○ | VBAT |
| 5 | H3 | 🟪 ✕ | BOOT0 — leave open |
| 6 | B9 | 🟦 D | IR TX |
| 7 | B8 | 🟦 D | '165 PL (latch) |
| 8 | NR | ⬜ ○ | RESET |
| 9 | A0 | 🟦 D | IR RX |
| 10 | A1 | 🟦 D | CC1101 GDO0 |
| 11 | A2 | 🟦 D | NFC IRQ |
| 12 | A3 | 🟦 D | SD detect |
| 13 | A4 | 🟦 D | SD CS |
| 14 | A5 | 🟦 D | '165 CLK |
| 15 | A6 | 🟦 D | buzzer (NPN driver) |
| 16 | A7 | 🟦 D | CC1101 CS |
| 17 | A8 | 🟦 D | NFC CS |
| 18 | A9 | 🟦 D | nRF24 CSN |
| 19 | B2 | 🟦 D | Display CS |
| 20 | G | 🟧 S | GND · SD, iButton |

</details>

## Power

*battery → switch → charger/booster → 5 V*

| From | To | Notes |
| --- | --- | --- |
| Battery holder − | Charger B− |  |
| Battery holder + | Slide switch, middle pin | the switch cuts the battery |
| Slide switch, outer pin | Charger B+ |  |
| Charger OUT+ (5 V) | 🟦 WeAct 5V · bottom pad 3 · NFC 5V · ESP32 5V | the WeAct's regulator makes the 3V3 for everything else |
| Charger OUT− | 🟦 WeAct G · top pad 1 · ESP32 GND |  |

## Former add-on modules

*header removed · now hard-wired inside the case*

| Module | Own pins (direct) | Shared | Power |
| --- | --- | --- | --- |
| NFC · Elechouse ST25R3916B | `CS PA8 · IRQ PA2` | `SCK · MOSI · MISO · GND` | `5V ← charger OUT+` |
| nRF24L01+PA+LNA | `CSN PA9 · CE PE4` | `SCK · MOSI · MISO · 3V3 · GND` | `3V3` |
| ESP32-C5 Mini | `RX ← PB6 · TX → PB7` | — | `5V / GND ← charger OUT` |

Each module has its own section below. With the header gone, UHF and Wiegand have no connection
point: they would need PB6/PB7, which the ESP32 now uses.

## The 5 shared nets

*too many wires for one pad*

| Module | SCK<br>B3 | MOSI<br>B5 | MISO<br>B4 | 3V3 | GND | its own pins (direct) |
| --- | :---: | :---: | :---: | :---: | :---: | --- |
| CC1101 | ● | ● | ● | ● | ● | `CS A7 · GDO0 A1` |
| Display | ● | ● | – | ● | ● | `CS B2 · DC B1 · RST B0` |
| microSD | ● | ● | ● | ● | ● | `CS A4 · DET A3` |
| NFC | ● | ● | ● | 5V | ● | `CS A8 · IRQ A2` |
| nRF24 | ● | ● | ● | ● | ● | `CSN A9 · CE E4` |
| **wires on the pad** | **5** | **5** | **4** | **10+** | **10+** | **→ splice or strip** |

Each row's **own** pins (right column) solder **direct** — one wire each. Only the five columns
above are the problem: 3–5 wires meet on one pad, plus 3V3/GND feed everything (and every button
pull-up). The board gives you **two 3V3 and four GND pads**. Spread the load across them, but you
still need splices or a strip.

## Can you solder it all directly?

> ### Yes for 20 pins · No for 5
> Every per-device pin (CS, IRQ, buttons, IR, buzzer, iButton, UART) is one wire → solder straight
> to the header hole. The five shared nets (B3 · B5 · B4 · 3V3 · GND) can't take 3–5 wires on one
> hole — handle them one of three ways:

| | |
|---|---|
| **Option A · perfboard**<br>**Perfboard bus strip** | A ~2×5 cm perfboard with 5 rails (SCK/MOSI/MISO/3V3/GND). **One** wire from each shared pad → the strip; every module taps the rails. Tidy, easy to probe, matches how the real Flipper routes it. |
| **Option B · no extra board**<br>**Daisy-chain** | Hop the bus module-to-module: `B3 → CC1101 → LCD → SD → bay`. One wire leaves the pad; the rest are short module-to-module jumps. Works, but harder to debug than a strip. |
| **Option C · used in the NOX case**<br>**Y-splices** | Heat-shrunk splices just above the top row, one each for SCK, MOSI, MISO, 3V3 and GND. Each module's wire joins its splice, and one short lead runs from the splice to the pad. The microSD wires go straight to B3/B4/B5 beside those leads, so no pad takes more than two wires. |

## Always-on core

*soldered direct · permanent*

| Device | Pins |
| --- | --- |
| Display (ST7565/67) | `CS→PB2 · RS/A0 (DC)→PB1 · RSE (reset)→PB0 · SCL→PB3 · SI→PB5 · VDD→3V3 · VSS→GND · A/K=backlight · IC_* pins unused` |
| CC1101 SubGHz | `CS PA7 · GDO0 PA1 + SPI bus` |
| microSD | `D3 (CS)→PA4 · DET→PA3 · CLK→PB3 · CMD (SI)→PB5 · DO→PB4 · 3V→3V3 · GND→GND` |
| Buttons (SN74HC165) | `PL PB8 · CLK PA5 · Q7 PA15` |
| Infrared | `TX PB9 · RX PA0 (RX VCC = 3V3 only)` |
| Buzzer (passive, 12 mm) | `PA6 → 1 kΩ → NPN driver` · see [Buzzer](#buzzer--12-mm-passive) below |
| **iButton (DS9092)** | **`PA10`** — probe centre → PA10 + 1k pull-up to 3V3; shell → GND |

All seven are **permanent** — soldered to their own pads (D in the board grid). **iButton and IR
are core, always connected.** NFC, nRF24 and the ESP32 used to plug into a 14-pin add-on header.
That header is gone, so they are hard-wired too.

## Display · ST7565 / ST7567 LCD

*core · write-only (no MISO)*

| LCD pin | Role | MCU / net | Connects to |
| --- | --- | --- | --- |
| CS | chip select | `PB2` | 🟦 MCU pad B2 |
| RS / A0 | data / command (DC) | `PB1` | 🟦 MCU pad B1 |
| RSE | reset | `PB0` | 🟦 MCU pad B0 |
| SCL | SPI clock | `PB3` | 🟧 bus strip |
| SI | SPI data in | `PB5` | 🟧 bus strip |
| VDD | power | `3V3` | 🟧 3V3 rail |
| VSS | ground | `GND` | 🟧 GND rail |
| A | backlight + | `3V3` | 🟧 3V3 rail |
| K | backlight − | `GND` | 🟧 GND rail |
| IC_SCL · IC_CS · IC_SO · IC_SI | font-ROM chip | — | leave open |

## Buttons · SN74HC165 shift register

*core · 6 buttons, 3 MCU wires*

![SN74HC165 DIP-16 pinout with each pin's connection](wiring-map/sn74hc165.svg)

| '165 pin | Name | Connects to |
| --- | --- | --- |
| 16 | VCC | 🟧 3V3 rail |
| 8 | GND | 🟧 GND rail |
| 1 | SH/LD (PL) | 🟦 MCU pad B8 `PB8` |
| 2 | CLK (CP) | 🟦 MCU pad A5 `PA5` |
| 9 | QH (Q7) | 🟦 MCU pad A15 `PA15` |
| 15 | CLK INH (CE) | 🟧 GND rail |
| 10 | SER (DS) | 🟧 GND rail |
| 7 | QH̄ (Q7 inverted) | leave open |

| Input | '165 pin | Button | Wiring |
| --- | --- | --- | --- |
| A · D0 | 11 | **Up** | 10 kΩ to 3V3 · button to GND |
| B · D1 | 12 | **Down** | 10 kΩ to 3V3 · button to GND |
| C · D2 | 13 | **Right** | 10 kΩ to 3V3 · button to GND |
| D · D3 | 14 | **Left** | 10 kΩ to 3V3 · button to GND |
| E · D4 | 3 | **OK** | 10 kΩ to 3V3 · button to GND |
| F · D5 | 4 | **Back** | 10 kΩ to 3V3 · button to GND |
| G · D6 | 5 | unused | tie to 3V3 |
| H · D7 | 6 | unused | tie to 3V3 |

- **Each button:** one leg to its '165 input, other leg to GND. The 10 kΩ pull-up holds the input HIGH; a press pulls it LOW.
- **Never leave D6/D7 floating.** A floating input reads as random presses.
- **CLK is on PA5**, not PB9. PB9 now belongs to IR TX.

## microSD breakout

*SD-native pin names → SPI mode*

| Breakout pin | SPI role | MCU / net | Connects to |
| --- | --- | --- | --- |
| 3V | VCC | `3V3` | 🟧 3V3 rail |
| GND | GND | `GND` | 🟧 GND rail |
| CLK | SCK | `PB3` | 🟧 bus strip |
| CMD (SI) | MOSI | `PB5` | 🟧 bus strip |
| DO | MISO | `PB4` | 🟧 bus strip |
| D3 | CS | `PA4` | 🟦 MCU pad A4 |
| DET | card detect | `PA3` | 🟦 MCU pad A3 |

- **Pull-ups:** if the breakout has no resistors, add 10 kΩ from 3V3 to each of DO, CMD and D3. Without them the card can fail to start.
- **DET polarity:** the firmware reads PA3 HIGH as "card present". Check the switch with a meter, power off, card in. DET to GND ≈ 0 Ω means the switch pulls low, so leave DET open and tie PA3 to 3V3. DET to 3V ≈ 0 Ω means wire DET to PA3 and add 10 kΩ from PA3 to GND. If unsure, tie PA3 to 3V3.
- **Card:** FAT32, 32 GB or smaller. SD has no power switch on this board; it runs straight off 3V3.

## SubGHz · CC1101

*core · 3.3 V only*

![CC1101 8-pin module pinout with each pin's connection](wiring-map/cc1101.svg)

| Module pin | Role | MCU / net | Connects to |
| --- | --- | --- | --- |
| VCC | power | `3V3` | 🟧 3V3 rail |
| GND | ground | `GND` | 🟧 GND rail |
| SCK | SPI clock | `PB3` | 🟧 bus strip |
| MOSI / SI | SPI data in | `PB5` | 🟧 bus strip |
| MISO / SO | SPI data out | `PB4` | 🟧 bus strip |
| CSN | chip select | `PA7` | 🟦 MCU pad A7 |
| GDO0 | data / IRQ | `PA1` | 🟦 MCU pad A1 |
| GDO2 | unused (pin 8) | — | leave open |
| ANT | antenna | — | 17 cm wire (433 MHz) or SMA antenna |

- **Never 5 V** on VCC. The CC1101 is a 3.3 V part.
- **Test:** Sub-GHz → Frequency Analyzer, hold a 433 MHz remote about 10 cm away. It should lock the frequency and show RSSI.

## 2.4 GHz · nRF24L01+PA+LNA

*hard-wired · through its breakout adapter*

![nRF24 breakout adapter pinout with each pin's connection](wiring-map/nrf24-adapter.svg)

| Adapter pin | Role | MCU / net | Connects to |
| --- | --- | --- | --- |
| VCC | power | `3V3` | 🟧 3V3 rail |
| GND | ground | `GND` | 🟧 GND rail |
| SCK | SPI clock | `PB3` | 🟧 bus strip |
| MO (MOSI) | SPI data in | `PB5` | 🟧 bus strip |
| MI (MISO) | SPI data out | `PB4` | 🟧 bus strip |
| CSN | chip select | `PA9` | 🟦 MCU pad A9 |
| CE | enable | `PE4` | 🟦 MCU pad E4 |
| IRQ | unused | — | leave open |

- **Power:** this adapter takes 3.3 V, so VCC goes to the 3V3 rail. The PA+LNA draws current spikes on transmit; if it drops packets, solder a 10–100 µF capacitor across VCC and GND at the module.
- **Firmware:** the nRF24 pins (CSN on PA9, CE on PE4) are set in the firmware, so wiring alone isn't enough. The board must run the NOX build, not stock Unleashed.
- **Sharing:** it runs alongside SubGHz on the shared bus but not at the same moment. It can't run while Wiegand-emulate is active (PA9 is shared).

## NFC · Elechouse ST25R3916B

*hard-wired · 13.56 MHz · SPI · JST-GH cable*

| Cable pin | Role | MCU / net | Connects to |
| --- | --- | --- | --- |
| 5V | power in | `5V` | 🟦 charger OUT+ |
| GND | ground | `GND` | 🟧 GND rail |
| SCLK / SCL | SPI clock | `PB3` | 🟧 bus strip |
| MOSI | SPI data in | `PB5` | 🟧 bus strip |
| MISO / SDA | SPI data out | `PB4` | 🟧 bus strip |
| CS | chip select | `PA8` | 🟦 MCU pad A8 |
| IRQ | interrupt | `PA2` | 🟦 MCU pad A2 |

- **5 V in, 3.3 V logic.** Elechouse rates this board for 5 V on its cable, and its SPI lines run at 3.3 V, so they connect straight to the MCU. A different ST25R3916 board may be 3.3 V only, so check yours before powering it.
- **SPI is the default mode.** There is no mode pin on the cable; I2C needs the board reconfigured first.
- **IRQ is PA2**, not PA0. PA0 now belongs to IR RX.
- **Placement:** the board lies flat on the case floor at the top end, under the IR seat. Keep its antenna clear of the CC1101 antenna and of metal.
- **Test:** NFC → Read, then hold a bank card or transit card on the antenna.

## iButton · DS9092 probe

*core · 1-Wire on PA10*

| Probe wire | Contact | MCU / net | Connects to |
| --- | --- | --- | --- |
| **Red** | centre contact (data) | `PA10` | 🟦 MCU pad A10 + 1 kΩ to 3V3 |
| **Black** | outer ring (ground) | `GND` | 🟧 GND rail |

- **Two wires is correct.** 1-Wire keys take power from the data line, so the probe has no VCC.
- **Pull-up:** 1 kΩ between PA10 and 3V3, at the board end. Without it, reads fail.
- **Check colours with a meter** before soldering: the black wire should beep to the outer ring, the red wire to the centre.
- **Test:** iButton → Read, then touch a Dallas key to the probe.

## Infrared · receiver + transmitter

*core · stock Flipper IR pins*

| Module | Pin | MCU / net | Connects to |
| --- | --- | --- | --- |
| Receiver (KY-022 / VS1838B / TSOP38238) | S (signal) | `PA0` | 🟦 MCU pad A0 |
|  | + (middle) | `3V3` | 🟧 3V3 rail · never 5 V |
|  | − | `GND` | 🟧 GND rail |
| Transmitter (KY-005) | S (signal) | `PB9` | 🟦 MCU pad B9 |
|  | + (middle) | `3V3` | 🟧 3V3 rail |
|  | − | `GND` | 🟧 GND rail |

- **Receiver on 3V3 only:** powered from 5 V, its output swings to 5 V on PA0.
- **Transmitter range:** the KY-005 drives its LED straight off PB9, about 1–2 m. For more range, use a bare 940 nm IR LED with a 2N3904 (or BC547) driver: PB9 → 1 kΩ → base, emitter → GND, LED cathode → collector, LED anode → 100 Ω → 5V.
- **Test:** Infrared → Learn New Remote, then press a TV remote button at the receiver.

## Buzzer · 12 mm passive

*core · PA6 · NPN low-side driver*

| Part | Pin | Connects to |
| --- | --- | --- |
| 1 kΩ resistor | one end | 🟦 MCU pad A6 `PA6` |
|  | other end | NPN base |
| NPN (2N3904 / BC547) | emitter | 🟧 GND · second wire on the SD board's GND pin |
|  | collector | buzzer − |
| Buzzer | + | 🟧 3V3 · second wire on the SD board's 3V pin |
| Diode (1N4148) | across the buzzer | cathode (band) to buzzer +, anode to buzzer − |

- **Passive, not active.** PA6 plays tones on it (timer TIM16), so use a plain magnetic or piezo buzzer, not one with a built-in oscillator.
- **Driver on the buzzer's legs.** The resistor, transistor and diode are soldered to the buzzer's pins and heat-shrunk, so only three wires run to it: PA6, 3V3 and GND.
- **Fit:** the buzzer (Ø12.1 × 10 mm) sits face-down in a cup on the case floor, over seven sound holes.

## ESP32-C5 Mini

*hard-wired · UART on PB6 / PB7 · sits on top of the charger*

| ESP32 pin | Role | MCU / net | Connects to |
| --- | --- | --- | --- |
| RX | UART in | `PB6` | 🟦 MCU pad B6 · top pad 6 |
| TX | UART out | `PB7` | 🟦 MCU pad B7 · top pad 5 |
| 5V | power in | `5V` | 🟦 charger OUT+ |
| GND | ground | `GND` | 🟦 charger OUT− |

- **Cross TX and RX.** The STM32 sends on PB6 into the ESP32's RX, and listens on PB7 to the ESP32's TX.
- **Power:** two short wires drop onto the charger's OUT pads right beside it. The ESP32's own regulator makes its 3.3 V.
- **No USB opening.** The case has no hole for the ESP32's USB-C, so flash it before fitting it. RX and TX alone can't put it into download mode. To reflash it in place, open the case, hold its BOOT button while you reset it, then use the ESP Flasher app ([setup.md](../setup.md)) over the existing wires.

## Can't run together

*reused pins*

- **nRF24 + SubGHz now coexist** ✓ — the SubGHz RF-switch GPIO was parked on non-bonded PC3 (no switch chip on this board), so **PE4 is nRF24 CE alone**. Both modules plug in and run; the shared SPI1 bus just time-slices (interleave, no simultaneous TX on both radios).
- **A9 + E4** = nRF24 (CSN/CE) + Wiegand-emulate drive → **not nRF24 and Wiegand-emulate at once.**
- **B6 / B7** = one serial device only. In this build they go to the **ESP32**, so UHF and Wiegand have nowhere to connect.
- **SPI bus** is shared by up to 5 devices → fine to have all connected; the firmware picks one via its CS.

---

<sub>NOX · WeAct STM32WB55CGU6 (UFQFPN48) · Flipper Zero (Unleashed) retarget · colour-blind-safe palette · pin map post-IR revision · display, buttons, microSD, CC1101, nRF24, NFC, iButton and IR module wiring added · updated 2 Nov 2026 for the case build</sub>
