# NOX Wiring Map

Every pad on the WeAct STM32WB55CGU6, what it drives, and which nets are shared, for the reference
build: every module hard-wired inside the case, no add-on headers. Pin assignments are the
`targets/f55` ones (summary in [pinout.md](pinout.md)); the same pins also work on plug-in headers
for a bench build ([plug-in-headers.md](plug-in-headers.md)).

Legend: **D** direct, one wire to the pad · **S** shared, via splice / bus strip / rail ·
**○** optional / leave · **✕** never wire.

## Board pads

Pads are numbered from the USB-C end. The board has **four GND pads** (two per row) and **two 3V3
pads**.

### Top row · 3V3 side

| # | Pad | | Connects to |
|---|---|---|---|
| 1 | G | S | GND · charger OUT− |
| 2 | G | S | GND · buttons ('165 board) |
| 3 | 3V3 | S | 3V3 splice |
| 4 | 3V3 | S | buttons ('165 board), microSD |
| 5 | B7 | D | UART RX ← ESP32 TX |
| 6 | B6 | D | UART TX → ESP32 RX |
| 7 | B5 | S | SPI MOSI |
| 8 | B4 | S | SPI MISO |
| 9 | B3 | S | SPI SCK |
| 10 | A15 | D | '165 Q7 (data) |
| 11 | A10 | D | iButton |
| 12 | E4 | D | nRF24 CE (also the board's blue LED) |
| 13 | B1 | D | Display DC (RS/A0) |
| 14 | B0 | D | Display RST (RSE) |
| 15 | G | S | GND splice |

### Bottom row · 5V side

| # | Pad | | Connects to |
|---|---|---|---|
| 1 | G | ○ | spare |
| 2 | 5V | ○ | USB 5 V |
| 3 | 5V | D | 5 V in ← charger OUT+ |
| 4 | VB | ○ | VBAT |
| 5 | H3 | ✕ | BOOT0 — leave open |
| 6 | B9 | D | IR TX |
| 7 | B8 | D | '165 PL (latch) |
| 8 | NR | ○ | RESET |
| 9 | A0 | D | IR RX |
| 10 | A1 | D | CC1101 GDO0 |
| 11 | A2 | D | NFC IRQ |
| 12 | A3 | D | SD detect |
| 13 | A4 | D | SD CS |
| 14 | A5 | D | '165 CLK |
| 15 | A6 | D | Buzzer (through its NPN driver) |
| 16 | A7 | D | CC1101 CS |
| 17 | A8 | D | NFC CS |
| 18 | A9 | D | nRF24 CSN |
| 19 | B2 | D | Display CS |
| 20 | G | S | GND · microSD, iButton |

E4 is nRF24 CE alone: the SubGHz RF-switch GPIO is parked on non-bonded PC3, so nRF24 and SubGHz
coexist. The blue LED on E4 flickers while the nRF24 is active. A9 (nRF24 CSN) doubles as the
Wiegand-emulate drive.

## Power

| From | To | Notes |
|---|---|---|
| Battery holder − | Charger B− | |
| Battery holder + | Slide switch, middle pin | the switch cuts the battery |
| Slide switch, outer pin | Charger B+ | |
| Charger OUT+ (5 V) | WeAct 5V (bottom pad 3) · NFC 5V · ESP32 5V | the WeAct's regulator makes the 3V3 for everything else |
| Charger OUT− | WeAct G (top pad 1) · ESP32 GND | |

## The five shared nets

| Module | SCK B3 | MOSI B5 | MISO B4 | 3V3 | GND | Own pins (direct) |
|---|---|---|---|---|---|---|
| CC1101 | ● | ● | ● | ● | ● | CS A7 · GDO0 A1 |
| Display | ● | ● | – | ● | ● | CS B2 · DC B1 · RST B0 |
| microSD | ● | ● | ● | ● | ● | CS A4 · DET A3 |
| NFC | ● | ● | ● | 5V | ● | CS A8 · IRQ A2 |
| nRF24 | ● | ● | ● | ● | ● | CSN A9 · CE E4 |

Every per-device pin (CS, IRQ, buttons, IR, buzzer, iButton, UART) is one wire straight to its pad.
The five shared nets can't take 3–5 wires on one pad. Three ways to handle them:

- **Y-splices (used in the reference build).** Heat-shrunk splices just above the top row, one each
  for SCK, MOSI, MISO, 3V3 and GND. Each module's wire joins its splice, and one short lead runs
  from the splice to the pad. The microSD wires go straight to B3/B4/B5 beside those leads, so no
  pad takes more than two wires.
- **Perfboard bus strip.** A ~2×5 cm perfboard with five rails. One wire from each shared pad to
  the strip; every module taps the rails. Tidy and easy to probe.
- **Daisy-chain.** Hop the bus module to module (`B3 → CC1101 → LCD → SD → …`). Works, but harder
  to debug.

## Always-on core

| Device | Pins |
|---|---|
| Display (ST7565/67) | CS→PB2 · RS/A0 (DC)→PB1 · RSE (reset)→PB0 · SCL→PB3 · SI→PB5 · VDD→3V3 · VSS→GND · A/K = backlight · IC_* pins unused |
| CC1101 SubGHz | CS PA7 · GDO0 PA1 + SPI bus |
| microSD | D3 (CS)→PA4 · DET→PA3 · CLK→PB3 · CMD (SI)→PB5 · DO→PB4 · 3V→3V3 · GND→GND |
| Buttons (SN74HC165) | PL PB8 · CLK PA5 · Q7 PA15 |
| Infrared | TX PB9 · RX PA0 (RX VCC = 3V3 only) |
| Buzzer (passive, 12 mm) | PA6 → 1 kΩ → NPN driver (see [Buzzer](#buzzer--12-mm-passive)) |
| iButton (DS9092) | PA10 — probe centre → PA10 + 1 kΩ pull-up to 3V3; shell → GND |

NFC, nRF24 and the ESP32 used to plug into add-on headers. In the reference build they are
hard-wired too.

## Display · ST7565 / ST7567 LCD

Write-only, no MISO.

| LCD pin | Role | MCU / net |
|---|---|---|
| CS | chip select | PB2 |
| RS / A0 | data / command (DC) | PB1 |
| RSE | reset | PB0 |
| SCL | SPI clock | PB3 (shared) |
| SI | SPI data in | PB5 (shared) |
| VDD | power | 3V3 |
| VSS | ground | GND |
| A | backlight + | 3V3 |
| K | backlight − | GND |
| IC_SCL · IC_CS · IC_SO · IC_SI | font-ROM chip | leave open |

## Buttons · SN74HC165 shift register

Six buttons, three MCU wires.

| '165 pin | Name | Connects to |
|---|---|---|
| 16 | VCC | 3V3 |
| 8 | GND | GND |
| 1 | SH/LD (PL) | PB8 |
| 2 | CLK (CP) | PA5 |
| 9 | QH (Q7) | PA15 |
| 15 | CLK INH (CE) | GND |
| 10 | SER (DS) | GND |
| 7 | QH̄ | leave open |

| Input | '165 pin | Button |
|---|---|---|
| A · D0 | 11 | Up |
| B · D1 | 12 | Down |
| C · D2 | 13 | Right |
| D · D3 | 14 | Left |
| E · D4 | 3 | OK |
| F · D5 | 4 | Back |
| G · D6 | 5 | unused, tie to 3V3 |
| H · D7 | 6 | unused, tie to 3V3 |

- **Each button:** one leg to its '165 input, the other to GND, with a 10 kΩ pull-up from the input
  to 3V3. Released reads HIGH, pressed reads LOW.
- **Never leave D6/D7 floating.** A floating input reads as random presses.

## microSD breakout

SD-native pin names, used in SPI mode.

| Breakout pin | SPI role | MCU / net |
|---|---|---|
| 3V | VCC | 3V3 |
| GND | GND | GND |
| CLK | SCK | PB3 (shared) |
| CMD (SI) | MOSI | PB5 (shared) |
| DO | MISO | PB4 (shared) |
| D3 | CS | PA4 |
| DET | card detect | PA3 |

- **Pull-ups:** if the breakout has no resistors, add 10 kΩ from 3V3 to each of DO, CMD and D3.
- **DET polarity:** the firmware reads PA3 HIGH as "card present". With power off and a card in,
  check the switch with a meter. DET to GND ≈ 0 Ω: leave DET open and tie PA3 to 3V3. DET to 3V
  ≈ 0 Ω: wire DET to PA3 and add 10 kΩ from PA3 to GND. If unsure, tie PA3 to 3V3.
- **Card:** FAT32, 32 GB or smaller. The SD runs straight off 3V3 (no power switch on this board).

## SubGHz · CC1101

3.3 V only.

| Module pin | Role | MCU / net |
|---|---|---|
| 1 GND | ground | GND |
| 2 VCC | power | 3V3 (never 5 V) |
| 3 GDO0 | data / IRQ | PA1 |
| 4 CSN | chip select | PA7 |
| 5 SCK | SPI clock | PB3 (shared) |
| 6 MOSI | SPI data in | PB5 (shared) |
| 7 MISO | SPI data out | PB4 (shared) |
| 8 GDO2 | unused | leave open |
| ANT | antenna | 17 cm wire (433 MHz) or SMA antenna |

Square pad = pin 1. Odd pins sit on the board edge, even pins inside.

## 2.4 GHz · nRF24L01+PA+LNA

Through its 3.3 V breakout adapter. Wire by the printed labels: header order differs between
adapters.

| Adapter pin | Role | MCU / net | Bare 2×4 module pin |
|---|---|---|---|
| VCC | power | 3V3 | 2 |
| GND | ground | GND | 1 |
| CE | enable | PE4 | 3 |
| CSN | chip select | PA9 | 4 |
| SCK | SPI clock | PB3 (shared) | 5 |
| MO (MOSI) | SPI data in | PB5 (shared) | 6 |
| MI (MISO) | SPI data out | PB4 (shared) | 7 |
| IRQ | unused | leave open | 8 |

- **Power:** the PA+LNA draws current spikes on transmit. If it drops packets, solder 10–100 µF
  across VCC and GND at the module.
- **Sharing:** it runs alongside SubGHz on the shared bus, but not at the same instant.

## NFC · Elechouse ST25R3916B

13.56 MHz, SPI mode, JST-GH cable.

| Cable pin | Role | MCU / net |
|---|---|---|
| 5V | power in | charger OUT+ (5 V) |
| GND | ground | GND |
| SCLK / SCL | SPI clock | PB3 (shared) |
| MOSI | SPI data in | PB5 (shared) |
| MISO / SDA | SPI data out | PB4 (shared) |
| CS | chip select | PA8 |
| IRQ | interrupt | PA2 |

- **5 V in, 3.3 V logic.** Elechouse rates this board for 5 V on its cable, and its SPI lines run at
  3.3 V, so they connect straight to the MCU. A different ST25R3916 board may be 3.3 V only, so
  check yours before powering it.
- **SPI is the board's default mode.** There is no mode pin on the cable; I2C needs the board
  reconfigured first.
- **IRQ is PA2**, not PA0. PA0 belongs to IR RX.
- **Placement:** keep the antenna clear of the CC1101 antenna and of metal.

## iButton · DS9092 probe

1-Wire on PA10.

| Probe wire | Contact | MCU / net |
|---|---|---|
| Red | centre contact (data) | PA10 + 1 kΩ to 3V3 |
| Black | outer ring (ground) | GND |

- **Two wires is correct.** 1-Wire keys take power from the data line.
- **Pull-up:** 1 kΩ between PA10 and 3V3, at the board end. Without it, reads fail.
- **Check colours with a meter** before soldering: black to the outer ring, red to the centre.

## Infrared · receiver + transmitter

Stock Flipper IR pins.

| Module | Pin | MCU / net |
|---|---|---|
| Receiver (KY-022 / VS1838B / TSOP38238) | S (signal) | PA0 |
| | + (middle) | 3V3, never 5 V |
| | − | GND |
| Transmitter (KY-005) | S (signal) | PB9 |
| | + (middle) | 3V3 |
| | − | GND |

- **Receiver on 3V3 only:** powered from 5 V, its output swings to 5 V on PA0.
- **Transmitter range:** the KY-005 drives its LED straight off PB9, about 1–2 m. For more range,
  use a bare 940 nm IR LED with a 2N3904 (or BC547) driver: PB9 → 1 kΩ → base, emitter → GND,
  LED cathode → collector, LED anode → 100 Ω → 5V.

## Buzzer · 12 mm passive

PA6 (TIM16_CH1), low-side NPN driver.

| Part | Pin | Connects to |
|---|---|---|
| 1 kΩ resistor | one end | PA6 |
| | other end | NPN base |
| NPN (2N3904 / BC547) | emitter | GND |
| | collector | buzzer − |
| Buzzer | + | 3V3 |
| Diode (1N4148) | across the buzzer | cathode (band) to buzzer +, anode to buzzer − |

- **Passive, not active.** PA6 plays tones on it, so use a plain magnetic or piezo buzzer, not one
  with a built-in oscillator.
- **Driver on the buzzer's legs.** In the reference build the resistor, transistor and diode are
  soldered to the buzzer's pins and heat-shrunk, so only three wires run to it: PA6, 3V3 and GND.
  The 3V3 and GND are second wires on the microSD board's 3V and GND pins.

## ESP32-C5 Mini

WiFi / BLE co-processor on USART1, sitting on top of the charger.

| ESP32 pin | Role | MCU / net |
|---|---|---|
| RX | UART in | PB6 (top pad 6) |
| TX | UART out | PB7 (top pad 5) |
| 5V | power in | charger OUT+ |
| GND | ground | charger OUT− |

- **Cross TX and RX.** The STM32 sends on PB6 into the ESP32's RX and listens on PB7 to the
  ESP32's TX.
- **Power:** two short wires to the charger's OUT pads. The ESP32's own regulator makes its 3.3 V.
- **Flashing:** the case has no opening for the ESP32's USB-C, so flash it before fitting it. RX and
  TX alone can't put it into download mode. To reflash it in place, open the case, hold its BOOT
  button while you reset it, then use the ESP Flasher app ([setup.md](../setup.md)) over the
  existing wires.

## Can't run together

- **nRF24 + SubGHz coexist.** The SubGHz RF-switch GPIO is parked on non-bonded PC3 (no switch chip
  on this board), so PE4 is nRF24 CE alone. The shared SPI1 bus time-slices: no simultaneous
  transmit on both radios.
- **PB6 / PB7 = one serial device only.** In the reference build they go to the ESP32, so UHF-RFID
  and Wiegand have nowhere to connect. To use one of those instead, move the ESP32's two data wires
  to it.
- **Wiegand emulate** needs PA9/PE4 (nRF24 CSN/CE) as well as D0/D1 on PB6/PB7, so it can't run
  alongside the nRF24, and in the reference build it isn't wired at all.
- **SPI bus:** up to five devices share it; the firmware picks one at a time by its CS.
