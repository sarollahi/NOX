#pragma once

#include <desktop/desktop_settings.h>
#include "../views/desktop_events.h"
#include <gui/view.h>
#include <datetime.h>

typedef struct DesktopViewLocked DesktopViewLocked;

typedef void (*DesktopViewLockedCallback)(DesktopEvent event, void* context);

void desktop_view_locked_set_callback(
    DesktopViewLocked* locked_view,
    DesktopViewLockedCallback callback,
    void* context);
void desktop_view_locked_update(DesktopViewLocked* locked_view);
View* desktop_view_locked_get_view(DesktopViewLocked* locked_view);
DesktopViewLocked* desktop_view_locked_alloc(void);
void desktop_view_locked_free(DesktopViewLocked* locked_view);
void desktop_view_locked_lock(DesktopViewLocked* locked_view, bool pin_locked);
void desktop_view_locked_unlock(DesktopViewLocked* locked_view);
void desktop_view_locked_close_doors(DesktopViewLocked* locked_view);
bool desktop_view_locked_is_locked_hint_visible(DesktopViewLocked* locked_view);

/** NOX: feed the idle lock screen the current time and the user's lock-screen preferences.
 *
 * Stock draws nothing while idle-locked. With settings->lock_show_clock set, the same state
 * instead shows a large centred clock (plus optional seconds and date). Call this once a second
 * from the desktop's clock timer; it is a no-op as far as the display goes when lock_show_clock
 * is 0, which is the default, so the stock blank lock screen is unchanged.
 */
void desktop_view_locked_set_clock(
    DesktopViewLocked* locked_view,
    const DateTime* datetime,
    bool format_12,
    const DesktopSettings* settings);
