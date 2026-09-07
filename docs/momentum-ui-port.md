# Momentum UI features on NOX: what's already here, what isn't

> **Status update.** Four items harvested from this comparison are now implemented and are
> documented in [features.md](features.md#interface): lock-screen clock / seconds / date, a
> status-bar battery-icon toggle, shifted-digit symbols in text entry, and a recursive SD file
> search shipped as the `nox_file_search` `.fap`. Every one defaults to off, so the stock
> interface is unchanged until enabled. Menu styles and asset packs remain deferred for the
> flash reasons in the last section.

Status of the three low-coupling Momentum UI wins. One is already shipped by Unleashed and needs
no work; the other two are specced here rather than written, for the reason in the last section.

## 1. Battery icon styles — already present, nothing to port

Unleashed already ships this. `applications/settings/desktop_settings/scenes/desktop_settings_scene_start.c`
defines six styles and a picker under **Settings → Desktop**:

| Setting | Constant |
|---|---|
| Bar | `DISPLAY_BATTERY_BAR` |
| % | `DISPLAY_BATTERY_PERCENT` |
| Inv. % | `DISPLAY_BATTERY_INVERTED_PERCENT` |
| Retro 3 | `DISPLAY_BATTERY_RETRO_3` |
| Retro 5 | `DISPLAY_BATTERY_RETRO_5` |
| Bar % | `DISPLAY_BATTERY_BAR_PERCENT` |

The draw paths are in `power_draw_battery_callback()`
(`applications/services/power/power_service/power.c`), selected on
`DesktopSettings.displayBatteryPercentage`. Porting Momentum's version would replace working code
with equivalent code.

Worth knowing on this board: `furi_hal_power_get_pct()` returns a flat 100% because there is no
gauge and no ADC divider (see `targets/f55/README_NOX.md` §4), so every style renders "full"
regardless. The styles work; the input is a constant.

## 2. Menu styles — absent, specced

`applications/services/gui/modules/menu.c` (279 lines) has exactly one layout: a three-row
scrolling list drawn in `menu_draw_callback()`, with `elements_frame()` around the middle row.
There is no style enum anywhere in the tree.

Integration points for a list/grid toggle:

| File | Change |
|---|---|
| `applications/services/gui/modules/menu.c` | Second draw path; `menu_process_up/down` become row-wise vs cell-wise; left/right gain meaning in grid mode (`menu_input_callback` currently routes only up/down/ok) |
| `applications/services/desktop/desktop_settings.h` | New field on `DesktopSettings` |
| `applications/services/desktop/desktop_settings.c` | Load/save; bump the settings file version so old files migrate |
| `applications/settings/desktop_settings/scenes/desktop_settings_scene_start.c` | Picker item |
| `applications/services/gui/modules/menu.h` | Style enum, if exposed to callers |

The catch is navigation semantics, not drawing. Grid mode needs a different wrap rule at the row
ends, and the main menu is the one UI that must never be broken — a bad build here leaves a board
that flashes fine and is then unusable.

## 3. Asset packs (icon / font swap) — absent, not low-coupling

There is no `asset_pack` support of any kind in the tree. This one does not belong in the same
category as the other two: Momentum's asset packs replace icon and font *resolution* globally, so
it is not a feature added beside the UI, it is a change to how every icon in the firmware is
looked up. It needs a pack discovery/loader over the SD card, indirection in the icon and canvas
layer, animation-manager changes for the dolphin set, and a settings UI. Expect the whole
`assets/` pipeline plus `gui/` to be in scope.

## Why the last two are specced and not written

This firmware is at its flash ceiling, and overflow does not fail loudly — it bricks the board.

`targets/f55/README_NOX.md` §10.2 records it: 812,604 bytes boots and runs, enabling extra apps
took it to ~864K, and that image silently overwrote the BLE radio stack sitting at `0x080D7000`
(880,640 bytes in). The board then hardfaulted before USB init, with no way back in over USB. The
linker reports the radio stack's space as free.

Both remaining features add compiled-in firmware, and neither can be sized without a real build.
Writing an unverified rewrite of the main menu's draw and input handling, into an image that
cannot be measured, against a ceiling whose overflow mode is "dead board, no USB", trades a small
UI nicety for the device.

The precondition is now in place: the release workflow gates every build against that ceiling and
reports remaining headroom (`Check firmware fits below the BLE radio stack` in
`.github/workflows/release.yml`). Run a build, read the headroom number, and these become
ordinary work — menu styles first, since it is a few KB and self-contained; asset packs only if
there is real room.
