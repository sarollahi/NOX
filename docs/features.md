# Features

Every app below is installed on the SD build. Browse the full catalogue with
hardware notes in [app-catalog.md](app-catalog.md); wire swappable modules per
[hardware/plug-in-headers.md](hardware/plug-in-headers.md).


## Sub-GHz

300–928 MHz via the CC1101. Core read/replay/analyze plus:

- **Allstar Firefly** (`allstar_firefly`) — Allstar Firefly 318ALD31K gate remote - 9-bit trinary OOK decoder/emulator
- **Chief Cooker** (`chief_cooker`) — A simple app
- **Enhanced Sub-Ghz Chat** (`esubghz_chat`) — Send text messages over Sub-GHz radio to another Flippers
- **Flipper Share** (`flipper_share`) — Direct file transfer between flippers via Sub-GHz
- **Music to Sub-GHz Radio** (`fmf_to_sub`) — Converts Flipper music files (.FMF and .TXT) into Sub-GHz files (.SUB)
- **FM TX** (`fmtx`) — Experimental MP3 audio over CC1101 FM
- **Frequency Analyzer** (`freq_analyzer_ext`) — Frequency Analyzer
- **Genie Door Recorder** (`genie_record`) — This application extracts the codes from a Genie garage door remote into a .GNE file.  It also plays back a .GNE file to a Genie garage door opener
- **Restaurant Pager Trigger** (`meal_pager`) — This app triggers restaurant pagers in a brute force manner, useful to test if devices are still functional
- **POCSAG Pager** (`pocsag_pager`) — App can capture POCSAG 1200 messages on CC1101 supported frequencies
- **ProtoPirate** (`proto_pirate`) — Decode car key fob signals from Sub-GHz
- **ProtoView** (`protoview`) — ProtoView
- **Radio Scanner** (`radio_scanner`) — Scanner for Flipper
- **Subghz Rolling Flaws** (`rolling_flaws`) — Rolling code receiver (version 1.5), used to learn about rolling code flaws.  Watch video at https://youtu.be/gMnGuDC9EQo
- **ShapShup** (`shapshup`) — ShapShup
- **Spectrum Analyzer** (`spectrum_analyzer`) — Displays a spectrogram chart to visually represent RF signals around you
- **Sub Analyzer** (`sub_analyzer`) — Analyze SubGhz .sub files to extract all signal properties
- **Sub Duplicate Finder** (`sub_dup_finder`) — Identify and clean up duplicate .sub files in storage
- **Sub-GHz Playlist** (`subghz_playlist`) — App works with list of sub-ghz files from .txt file that contains paths to target files
- **Sub-GHz Playlist Creator** (`subghz_playlist_creator`) — App for creating or editing SubGhz playlists
- **Sub-GHz RAW Edit** (`subghz_raw_edit`) — Open a recorded RAW .sub capture, find the real signal in a sea of noice, pick start/end with the arrows and save just the trimmed burst
- **subghz_remote** (`subghz_remote`) — subghz_remote
- **Sub-GHz Scheduler** (`subghz_scheduler`) — Send a Sub-GHz signal repeatedly at a given interval
- **SubGHz Signal Generator** (`subghz_signal_gen`) — SubGHz Signal Generator for Flipper Zero
- **Sub-GHz WarDriving** (`subghz_wardriving`) — Sub-GHz WarDriving
- **TPMS Reader** (`tpms`) — Use SubGHz and RFID to read or activate TPMS sensors
- **TPMS Bridge** (`tpms_bridge`) — Reads car tyre pressure sensors: 39 protocols, pressure, temperature, signal
- **Weather Station** (`weather_station`) — Receive weather data from a wide range of supported Sub-1GHz remote sensor

## Infrared

38 kHz IR via a plug-in module (IR LED + NPN on PB9 for TX, TSOP38238/KY-022 demodulator on PA0
for RX). These are the stock-Flipper IR pins (TIM1/TIM2), so the full IR app runs unmodified:

- **Infrared** (`infrared`) — learn, save, and replay remotes; universal remote database (TV / AC /
  audio / projector) from the SD card; per-button capture and send.
- **cli_ir** (`cli_ir`) — infrared CLI plugin (`ir rx` / `ir tx`).

Point a TV/AC remote at the receiver to learn it; save and blast back with the transmitter. Range
depends on the IR LED drive (KY-005-style modules reach ~1–2 m; add a driver transistor + brighter
LED for more).

## NFC

13.56 MHz via the ST25R3916 (plug-in). Core read/emulate plus:

- **AmiTool** (`ami_tool`) — An NTAG215 toolkit for Flipper Zero to work with tags used with gaming consoles
- **ChaosID** (`chaos_id`) — Card forensics + risk classification + MIFARE Classic dictionary attack
- **Flipper Share NFC** (`flipper_share_nfc`) — Direct file transfer between flippers via NFC
- **ISO 15693-3 NFC Writer** (`iso15693_nfc_writer`) — ISO 15693-3 NFC Writer
- **Metroflip** (`metroflip`) — Read public transport cards from around the world
- **MIFARE Classic Editor** (`mfc_editor`) — Application for viewing and editing MIFARE Classic .nfc files
- **MFKey** (`mfkey`) — MIFARE Classic key recovery tool
- **MFP Reader** (`mfp_reader`) — MIFARE Plus SL3 reader, dumper and emulator
- **Mifare Fuzzer** (`mifare_fuzzer`) — App emulates Mifare Classic cards with various UIDs to check how reader reacts on them
- **NFC** (`nfc`) — NFC
- **NFC APDU Runner** (`nfc_apdu_runner`) — Run APDU commands from script files
- **NFC Canary** (`nfc_canary`) — Passive NFC skim detector: alarms when a reader interrogates the device
- **NFC Comparator** (`nfc_comparator`) — A tool to check if a physical NFC card is the same as a saved digital one
- **NFC Eink Tags** (`nfc_eink`) — Application for emulation and writing to NFC Eink tags
- **NFC Login** (`nfc_login`) — NFC based desktop login using USB HID or BLE HID
- **NFC Magic** (`nfc_magic`) — Application for writing to NFC tags with modifiable sector 0
- **NFC Maker** (`nfc_maker`) — Create NFC files for BT MACs, Contacts, Links, Emails, Phones, Text and WiFis
- **NFC Playlist** (`nfc_playlist`) — A Tool used to running through a list of NFC cards
- **NFC Stock Manager Alpha** (`nfc_stock_manager`) — Alpha build: manage inventory items via NFC tags
- **Passport Reader** (`passy`) — eMRTD Reader
- **PicoPass** (`picopass`) — App to communicate with PicoPass(iClass) tags
- **SaFlip** (`saflip`) — SaFlip Hospitality Tool for Flipper Zero
- **Seader** (`seader`) — Seader
- **Seos Compatible** (`seos`) — Seos compatible reader/emulator
- **Specter** (`specter`) — Counter-surveillance bug-sweep: passively senses active 13.56 MHz NFC reader/skimmer fields nearby (hidden POS skimmers, rogue door readers) using the onboard NFC chip. Sweep to find one, fingerprint its polling cadence, survey a room for a verdict, leave it in Watch mode to stand guard, and keep a timestamped logbook (txt + CSV) on the SD card. No extra hardware, never transmits
- **Sonicare Head ID** (`uk_mbirth_sonicare`) — Philips Sonicare brush head NFC reader + usage reset
- **UL-C Bruteforce** (`ulc_brute`) — Bruteforce Ultralight C authentication
- **UL-C Relay** (`ulc_relay`) — Relay Ultralight C authentication
- **ULCFKey** (`ulcfkey`) — Crack counterfeit ULC cards
- **ULCFKey (Next)** (`ulcfkey_next`) — Crack counterfeit ULC cards
- **VB Migration Assistant** (`vb_migrate`) — Makes transferring characters with VB Lab less cumbersome
- **VK Thermo** (`vk_thermo`) — Read temperature from your VivoKey Thermo or Temptress via NFC. Supports TMP112/117/119 sensors. Multi-device tracking, history, and graph views
- **Weebo** (`weebo`) — An NTAG215 parser, writer, emulator, remixer, duplicator
- **Xiaomi Filter Reset** (`xiaomi_filter_reset`) — Reset or check the filter-life counter on Xiaomi air purifier NFC filters

## Wireless — WiFi / BLE

On-board BLE (light stack) + ESP32 over the Serial bay (flash the ESP with **ESP Flasher**):

- **BLE Spam** (`ble_spam`) — Flood BLE advertisements to cause spammy and annoying popups/notifications
- **BT Trigger** (`bt_trigger`) — Control your smartphone camera via your Flipper Zero
- **BT Remote for Kodi** (`btremote_kodi`) — Application Bluetooth remote Keynote for Kodi (original app + feature: long press on OK to switch between Space and Return (useful for Kodi to navigate the menus))
- **Agentic Remote** (`claude_remote_ble`) — One-handed Bluetooth remote for AI coding agents (Claude Code). 5-button D-pad control, macros, offline manual + quiz
- **Droid Beacon** (`droidbeacon`) — Emulate Galaxy's Edge BLE park location beacons for Droid Depot droids
- **FindMy Flipper** (`findmy`) — BLE FindMy Location Beacon
- **FindMy Flipper** (`findmy_flipper`) — BLE FindMy Location Beacon
- **Bluetooth Remote** (`hid_ble`) — Use Flipper as a HID remote control over Bluetooth
- **PC monitor** (`pc_monitor`) — Application for monitoring PC resources
- **[ESP32] Ghost ESP** (`ghost_esp`) — [ESP32] Ghost ESP
- **[ESP32] Ghost ESP** (`ghost_esp`) — [ESP32] Ghost ESP
- **[ESP32] WiFi Marauder** (`esp32_wifi_marauder`) — [ESP32] WiFi Marauder
- **[ESP8266] Deauther** (`esp8266_deauther`) — DSTIKE Deauther module interface, based on ESP8266
- **IFTTT Btn** (`esp8266_ifttt_virtual_button`) — IFTTT Btn
- **Deauther v2** (`esp8266_wifi_deauther_v2`) — Works with ESP8266 Deauther v2 by @SpacehuhnTech (github)
- **ESP Flasher** (`esp_flasher`) — ESP Flasher
- **[ESP32] Evil Portal** (`evil_portal`) — [ESP32] Evil Portal
- **FlipDownloader** (`flip_downloader`) — Download apps and assets via WiFi directly to your Flipper Zero
- **FlipLibrary** (`flip_library`) — Utilize WiFi to retrieve data from 20 different APIs
- **FlipTDI** (`flip_tdi`) — Flipper FTDI232H emulator
- **FlipWeather** (`flip_weather`) — Use WiFi to get GPS and Weather information on your Flipper Zero
- **FlipWiFi** (`flip_wifi`) — FlipperHTTP companion app
- **FlipperHTTP** (`flipper_http`) — FlipperHTTP Companion App
- **WIFI Mapping** (`wifi_map`) — WIFI Mapping
- **[WiFi] Scanner** (`wifi_scanner`) — WiFi scanner module interface, based on ESP8266

## nRF24 & UHF-RFID

2.4 GHz nRF24L01+ (SPI bay) and UHF-RFID YRM100 (Serial bay):

- **NRF24 Batch** (`nrf24_batch`) — NRF24 Batch
- **[NRF24] Mouse Jacker** (`nrf24_mouse_jacker`) — App works with NRF24 Sniffer app to perform mousejack attacks
- **Mouse Jacker MS** (`nrf24_mouse_jacker_ms`) — Mouse Jacker MS
- **NRF24 Scanner** (`nrf24_scanner`) — NRF24 Scanner
- **[NRF24] Sniffer** (`nrf24_sniffer`) — App captures addresses to use with NRF24 Mouse Jacker app to perform mousejack attacks
- **Sniffer MS** (`nrf24_sniffer_ms`) — Sniffer MS
- **Channel Scanner** (`nrf24channelscanner`) — Scans 2.4Ghz frequency for usage data
- **NRF24 Tool** (`nrf24tool`) — Utility app for NRF24L01
- **NRF24 Batch** (`nrf24_batch`) — NRF24 Batch
- **[NRF24] Mouse Jacker** (`nrf24_mouse_jacker`) — App works with NRF24 Sniffer app to perform mousejack attacks
- **Mouse Jacker MS** (`nrf24_mouse_jacker_ms`) — Mouse Jacker MS
- **NRF24 Scanner** (`nrf24_scanner`) — NRF24 Scanner
- **[NRF24] Sniffer** (`nrf24_sniffer`) — App captures addresses to use with NRF24 Mouse Jacker app to perform mousejack attacks
- **Sniffer MS** (`nrf24_sniffer_ms`) — Sniffer MS
- **Channel Scanner** (`nrf24channelscanner`) — Scans 2.4Ghz frequency for usage data
- **NRF24 Tool** (`nrf24tool`) — Utility app for NRF24L01
- **Simultaneous UHF RFID Reader** (`simultaneous_rfid_reader`) — Simultaneous UHF RFID Reader that supports the M6E Nano, M7E Hecto, and YRM1000 series Readers. Read up to 150 UHF tags per second [Using ThingMagic Readers]!
- **[YRM100] UHF RFID** (`uhf_rfid`) — The app leverages the YRM100 module to enable UHF RFID functionality
- **Wiegand Reader** (`wiegand_reader`) — This application can be used to test Wiegand readers and keypads

## USB, iButton & security tools

USB device modes, 1-Wire, and assorted security utilities:

- **Bad USB** (`bad_usb`) — Bad USB
- **CCID Emu** (`ccid_test`) — Emulates a USB smart card reader (CCID) with an ISO7816 T=0 card
- **USB Keyboard & Mouse** (`hid_usb`) — Use Flipper as a HID remote control over USB
- **HID File Transfer** (`hidtransfer`) — HID File Transfer
- **LEGO Dimensions Toy Pad** (`ldtoypad`) — USB Toy Pad emulator for Lego Dimensions
- **U2F** (`u2f`) — U2F
- **USB HID Autofire** (`usb_hid_autofire`) — This is a simple Flipper Zero application to send left-clicks as a USB HID device
- **iButton Fuzzer** (`ibtn_fuzzer`) — iButton Fuzzer
- **iButton** (`ibutton`) — iButton
- **DTMF Dolphin** (`dtmf_dolphin`) — DTMF (Dual-Tone Multi-Frequency) dialer, Bluebox, and Redbox
- **FlipCrypt** (`flip_crypt`) — Encrypt, decrypt, and hash text using a wide variety of classic and modern crypto tools
- **Ford Radio Codes** (`fordradiocode`) — Ford Radio 'M' & 'V' Unlock Code Generator!
- **Key Copier** (`key_copier`) — @README.md
- **Authenticator** (`totp`) — Software-based TOTP/HOTP authenticator for Flipper Zero device
- **ZeroFIDO** (`zerofido`) — ZeroFIDO