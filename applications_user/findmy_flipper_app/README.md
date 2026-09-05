# FindMy Flipper (NOX port)

Turns the Flipper into an Apple **FindMy** / AirTag (or Samsung SmartTag) beacon — broadcasts a
registered key so the tag shows up in the FindMy network. Import your own keys.

- **Upstream:** [MatthewKuKanich/FindMyFlipper](https://github.com/MatthewKuKanich/FindMyFlipper)
- **Hardware:** on-board BLE (light stack). Bay: none.

**NOX port:** appid renamed to `findmy_flipper` (avoids the core `findmy` app collision) with the
icon header include switched to the appid-named header; the firmware-origin guard fixed so
`furi_hal_bt_reverse_mac_addr` is always declared, and a local implementation added (the stock HAL
doesn't export it); removed a bundled stock-named icon that collided at link.
