#include <core/common_defines.h>
#include <furi_hal_resources.h>
#include <furi_hal_light.h>
#include <stdint.h>
#include <notification/notification_app.h>

/* NOX (WeAct STM32WB55CGU6) override of the stock f7 light HAL.
 *
 * There is no LP5562 LED driver on this board. The LCD backlight is tied straight to 3V3 and
 * the only LED is the WeAct blue one on PE4, which belongs to other peripherals (see
 * furi_hal_power.c). The stock HAL drove the LP5562 unconditionally: once at boot and again on
 * every LED or backlight change, i.e. on every notification blink. furi_hal_i2c_* returns false
 * on NACK rather than asserting, so it never crashed, but each call still burned a full
 * LP5562_I2C_TIMEOUT per register write for nothing.
 *
 * Probing for the chip is not a usable alternative here: the power-I2C pins are PA9/PA10, which
 * this target repurposes as the parked external-SPI chip-select and the iButton line, and with
 * no bus pull-ups a presence probe has no defined result. So the LP5562 path is simply gone.
 *
 * What is kept is rgb_backlight_update(): the RGB backlight is a separate addressable strip on
 * its own GPIO (see applications/services/notification/notification_app.c), not an LP5562
 * channel, so the backlight keeps working exactly as before.
 *
 * If an LP5562 is ever fitted, copy targets/f7/furi_hal/furi_hal_light.c back over this file
 * and give it a real I2C bus with pull-ups.
 */

void furi_hal_light_init(void) {
    // Nothing to initialise. No log either: this runs from furi_hal_init_early(), before the
    // furi logging subsystem exists.
}

void furi_hal_light_set(Light light, uint8_t value) {
    // Red/green/blue have no hardware here; the backlight does.
    if(light & LightBacklight) {
        rgb_backlight_update(value / 255.0f);
    }
}

void furi_hal_light_blink_start(Light light, uint8_t brightness, uint16_t on_time, uint16_t period) {
    // LP5562 blink engines only ever drove the RGB LED, which this board does not have.
    UNUSED(light);
    UNUSED(brightness);
    UNUSED(on_time);
    UNUSED(period);
}

void furi_hal_light_blink_stop(void) {
}

void furi_hal_light_blink_set_color(Light light) {
    UNUSED(light);
}

void furi_hal_light_sequence(const char* sequence) {
    // Unchanged from f7: 'W'/'w' still drive the backlight, the delays still apply, and the
    // R/G/B characters fall through to a no-op in furi_hal_light_set().
    do {
        if(*sequence == 'R') {
            furi_hal_light_set(LightRed, 0xFF);
        } else if(*sequence == 'r') {
            furi_hal_light_set(LightRed, 0x00);
        } else if(*sequence == 'G') {
            furi_hal_light_set(LightGreen, 0xFF);
        } else if(*sequence == 'g') {
            furi_hal_light_set(LightGreen, 0x00);
        } else if(*sequence == 'B') {
            furi_hal_light_set(LightBlue, 0xFF);
        } else if(*sequence == 'b') {
            furi_hal_light_set(LightBlue, 0x00);
        } else if(*sequence == 'W') {
            furi_hal_light_set(LightBacklight, 0xFF);
        } else if(*sequence == 'w') {
            furi_hal_light_set(LightBacklight, 0x00);
        } else if(*sequence == '.') {
            furi_delay_ms(250);
        } else if(*sequence == '-') {
            furi_delay_ms(500);
        }
        sequence++;
    } while(*sequence != 0);
}
