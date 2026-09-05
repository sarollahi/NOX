# Security & responsible use

NOX is a research and education platform for radio, RFID/NFC, and hardware security. It bundles
**offensive tooling** — WiFi deauthentication, BLE advertisement spam, SubGHz capture/replay and
bruteforce, NFC/MIFARE attacks, nRF24 keystroke injection, and USB HID injection.

## Authorized use only

Use these tools **only** on devices, cards, radios, and networks that you **own** or have
**explicit written authorization** to test. Unauthorized interception, jamming, access, or
disruption of communications is illegal in most jurisdictions (in the EU/Sweden, radio and
computer-misuse law applies). You are solely responsible for how you use this project.

Deliberately excluded because they are hard to use lawfully or safely:

- **Broadband RF jammers** — illegal radio interference; not built.
- **125/134 kHz LF-RFID and Infrared** — no supporting hardware on this board (also removes a class of casual cloning).

## Scope of "security" here

This project has **no network services, accounts, or remote attack surface of its own**. It is
firmware for a personal device. "Vulnerabilities" in the traditional sense are limited to:

- Build/supply-chain issues in `scripts/` or the release workflow.
- Firmware defects that could brick a board or corrupt an SD card.
- Incorrect hardware guidance that could damage a component.

## Reporting

Found a defect in the build scripts, firmware target, or documentation? Open a
**private security advisory** on GitHub (`Security → Report a vulnerability`) or open a regular
issue for non-sensitive reports. Please include the board revision, firmware ref, and steps to
reproduce.

Do **not** file issues that are requests for help attacking a third party — they will be closed.

## Data & privacy

NOX runs entirely on-device. It does not phone home. Anything you capture (SubGHz `.sub` files,
NFC dumps, Evil Portal credential logs, PCAPs) is written to **your** SD card — treat those files
as sensitive and handle captured third-party data lawfully.
