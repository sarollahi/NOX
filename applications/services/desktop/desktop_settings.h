#pragma once

#include <stdint.h>

#define DISPLAY_BATTERY_BAR              0
#define DISPLAY_BATTERY_PERCENT          1
#define DISPLAY_BATTERY_INVERTED_PERCENT 2
#define DISPLAY_BATTERY_RETRO_3          3
#define DISPLAY_BATTERY_RETRO_5          4
#define DISPLAY_BATTERY_BAR_PERCENT      5

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FavoriteAppLeftShort,
    FavoriteAppLeftLong,
    FavoriteAppRightShort,
    FavoriteAppRightLong,
    FavoriteAppOkLong,

    FavoriteAppNumber,
} FavoriteAppShortcut;

typedef enum {
    DummyAppLeftShort,
    DummyAppLeftLong,
    DummyAppRightShort,
    DummyAppRightLong,
    DummyAppUpLong,
    DummyAppDownShort,
    DummyAppDownLong,
    DummyAppOkShort,
    DummyAppOkLong,

    DummyAppNumber,
} DummyAppShortcut;

typedef struct {
    char name_or_path[128];
} FavoriteApp;

typedef struct {
    uint32_t auto_lock_delay_ms;
    uint8_t usb_inhibit_auto_lock;
    uint8_t displayBatteryPercentage;
    uint8_t dummy_mode;
    uint8_t display_clock;
    FavoriteApp favorite_apps[FavoriteAppNumber];
    FavoriteApp dummy_apps[DummyAppNumber];

    /* NOX additions. Every field below is deliberately "0 == the behaviour this firmware had
     * before the field existed", so a zeroed struct (fresh install, failed load, or a migration
     * from an older version) reproduces the stock UI exactly and nothing is lost. */

    // Lock screen extras. Stock draws nothing at all while idle-locked; 0 keeps it that way.
    uint8_t lock_show_clock; // 0 = blank lock screen, 1 = big centred clock
    uint8_t lock_show_seconds; // 0 = HH:MM, 1 = HH:MM:SS (only if lock_show_clock)
    uint8_t lock_show_date; // 0 = no date, 1 = date under the clock (only if lock_show_clock)

    // Status bar. Named "hide_*" so 0 = shown = stock.
    uint8_t statusbar_hide_battery; // 1 = drop the battery icon from the status bar
} DesktopSettings;

void desktop_settings_load(DesktopSettings* settings);
void desktop_settings_save(const DesktopSettings* settings);

#ifdef __cplusplus
}
#endif
