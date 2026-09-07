#include <furi_hal_resources.h>
#include <furi_hal_bus.h>
#include <furi.h>

#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_pwr.h>

#define TAG "FuriHalResources"

const GpioPin gpio_swdio = {.port = GPIOA, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_swclk = {.port = GPIOA, .pin = LL_GPIO_PIN_14};

const GpioPin gpio_vibro = {.port = VIBRO_GPIO_Port, .pin = VIBRO_Pin};
const GpioPin gpio_ibutton = {.port = iBTN_GPIO_Port, .pin = iBTN_Pin};

const GpioPin gpio_cc1101_g0 = {.port = CC1101_G0_GPIO_Port, .pin = CC1101_G0_Pin};
const GpioPin gpio_rf_sw_0 = {.port = RF_SW_0_GPIO_Port, .pin = RF_SW_0_Pin};

const GpioPin gpio_subghz_cs = {.port = CC1101_CS_GPIO_Port, .pin = CC1101_CS_Pin};
const GpioPin gpio_display_cs = {.port = DISPLAY_CS_GPIO_Port, .pin = DISPLAY_CS_Pin};
const GpioPin gpio_display_rst_n = {.port = DISPLAY_RST_GPIO_Port, .pin = DISPLAY_RST_Pin};
const GpioPin gpio_display_di = {.port = DISPLAY_DI_GPIO_Port, .pin = DISPLAY_DI_Pin};
const GpioPin gpio_sdcard_cs = {.port = SD_CS_GPIO_Port, .pin = SD_CS_Pin};
const GpioPin gpio_sdcard_cd = {.port = SD_CD_GPIO_Port, .pin = SD_CD_Pin};
const GpioPin gpio_nfc_cs = {.port = NFC_CS_GPIO_Port, .pin = NFC_CS_Pin};

// NOX (WeAct WB55CGU6) direct-GPIO buttons to GND, internal pull-up, active-low.
// EXTI lines 8/5/2/6/3/9 - no collision with NFC(EXTI0/PA0) or SubGHz GDO0(EXTI1/PA1).
const GpioPin gpio_button_up = {.port = GPIOB, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_button_down = {.port = GPIOA, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_button_right = {.port = GPIOA, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_button_left = {.port = GPIOA, .pin = LL_GPIO_PIN_2};
// NOX: OK moved OFF PH3. PH3 is the BOOT0 strap - latched at reset before firmware sets any
// pull, so a button on it risks an indeterminate/DFU boot. PA15 (EXTI15) is conflict-free.
// Leave PH3 unconnected (WeAct carrier's BOOT0 pull-down keeps it low = boot from flash).
const GpioPin gpio_button_ok = {.port = GPIOA, .pin = LL_GPIO_PIN_15};
const GpioPin gpio_button_back = {.port = GPIOB, .pin = LL_GPIO_PIN_9};

// ---- NOX SN74HC165 button input -------------------------------------------
// One PISO shift register reads all 6 buttons. Interface reuses the old button
// GPIOs (PL=PB8, CP=PA5, Q7=PA15); the buttons themselves hang off the '165
// parallel inputs D0..D5 (10k pull-up each, switch to GND, active-low):
//   D0=Up D1=Down D2=Right D3=Left D4=Ok D5=Back   (D6/D7 -> 3V3, unused)
// Read order: QH (=D7) first, so first-read lands in bit7, last (D0) in bit0.
const GpioPin gpio_165_pl = {.port = GPIOB, .pin = LL_GPIO_PIN_8}; // SH/LD pin1
const GpioPin gpio_165_cp = {
    .port = GPIOA,
    .pin = LL_GPIO_PIN_5}; // CLK pin2 (moved PB9->PA5, frees PB9 for IR TX)
const GpioPin gpio_165_q7 = {.port = GPIOA, .pin = LL_GPIO_PIN_15}; // QH    pin9

// 0xFF = all released. A missing/unwired '165 leaves Q7 pulled high -> reads
// 0xFF every scan -> no phantom keypresses (missing-hardware safe).
uint8_t furi_hal_resources_165_state = 0xFF;

void furi_hal_resources_165_init(void) {
    furi_hal_gpio_write(&gpio_165_pl, 1);
    furi_hal_gpio_init(&gpio_165_pl, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_165_cp, 0);
    furi_hal_gpio_init(&gpio_165_cp, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    // Pull-up: unwired '165 -> Q7 reads high -> all released.
    furi_hal_gpio_init(&gpio_165_q7, GpioModeInput, GpioPullUp, GpioSpeedLow);
}

// ponytail: fixed short bit delay (~sub-us). Bump the loop count if long wires
// to the '165 glitch the reads. HC165 needs only ~20ns/edge at 3V3.
static inline void furi_hal_resources_165_delay(void) {
    for(volatile uint32_t i = 0; i < 12; i++) {
    }
}

void furi_hal_resources_165_scan(void) {
    furi_hal_gpio_write(&gpio_165_pl, 0); // latch parallel inputs
    furi_hal_resources_165_delay();
    furi_hal_gpio_write(&gpio_165_pl, 1); // enable shifting
    furi_hal_resources_165_delay();

    uint8_t v = 0;
    for(int i = 7; i >= 0; i--) {
        if(furi_hal_gpio_read(&gpio_165_q7)) v |= (1u << i);
        furi_hal_gpio_write(&gpio_165_cp, 1); // shift next bit into QH
        furi_hal_resources_165_delay();
        furi_hal_gpio_write(&gpio_165_cp, 0);
        furi_hal_resources_165_delay();
    }
    furi_hal_resources_165_state = v;
}
// ---------------------------------------------------------------------------

const GpioPin gpio_spi_d_miso = {.port = SPI_D_MISO_GPIO_Port, .pin = SPI_D_MISO_Pin};
const GpioPin gpio_spi_d_mosi = {.port = SPI_D_MOSI_GPIO_Port, .pin = SPI_D_MOSI_Pin};
const GpioPin gpio_spi_d_sck = {.port = SPI_D_SCK_GPIO_Port, .pin = SPI_D_SCK_Pin};
const GpioPin gpio_spi_r_miso = {.port = SPI_R_MISO_GPIO_Port, .pin = SPI_R_MISO_Pin};
const GpioPin gpio_spi_r_mosi = {.port = SPI_R_MOSI_GPIO_Port, .pin = SPI_R_MOSI_Pin};
const GpioPin gpio_spi_r_sck = {.port = SPI_R_SCK_GPIO_Port, .pin = SPI_R_SCK_Pin};

const GpioPin gpio_ext_pc0 = {.port = GPIOC, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_ext_pc1 = {.port = GPIOC, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_ext_pc3 = {.port = GPIOC, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_ext_pb2 = {.port = GPIOB, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_ext_pb3 = {.port = GPIOB, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_ext_pa4 = {.port = GPIOA, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_ext_pa6 = {.port = GPIOA, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_ext_pa7 = {.port = GPIOA, .pin = LL_GPIO_PIN_7};

const GpioPin gpio_nfc_irq_rfid_pull = {.port = RFID_PULL_GPIO_Port, .pin = RFID_PULL_Pin};
const GpioPin gpio_rfid_carrier_out = {.port = RFID_OUT_GPIO_Port, .pin = RFID_OUT_Pin};
const GpioPin gpio_rfid_data_in = {.port = RFID_RF_IN_GPIO_Port, .pin = RFID_RF_IN_Pin};
const GpioPin gpio_rfid_carrier = {.port = RFID_CARRIER_GPIO_Port, .pin = RFID_CARRIER_Pin};

const GpioPin gpio_infrared_rx = {.port = IR_RX_GPIO_Port, .pin = IR_RX_Pin};
const GpioPin gpio_infrared_tx = {.port = IR_TX_GPIO_Port, .pin = IR_TX_Pin};

const GpioPin gpio_usart_tx = {.port = USART1_TX_Port, .pin = USART1_TX_Pin};
const GpioPin gpio_usart_rx = {.port = USART1_RX_Port, .pin = USART1_RX_Pin};

const GpioPin gpio_i2c_power_sda = {.port = GPIOA, .pin = LL_GPIO_PIN_10};
const GpioPin gpio_i2c_power_scl = {.port = GPIOA, .pin = LL_GPIO_PIN_9};

// No speaker on NOX board; PB8 is the UP button. Park on spare NC pin PE4.
const GpioPin gpio_speaker = {.port = SPEAKER_GPIO_Port, .pin = SPEAKER_Pin};

// NOX: no SD power-switch on this board (SD is fed straight from 3V3), so this stock
// "SD stepdown control" pin is vestigial. Parked on PE4 to free PA3 for the SD DET line.
const GpioPin gpio_periph_power = {.port = GPIOE, .pin = LL_GPIO_PIN_4};

const GpioPin gpio_usb_dm = {.port = GPIOA, .pin = LL_GPIO_PIN_11};
const GpioPin gpio_usb_dp = {.port = GPIOA, .pin = LL_GPIO_PIN_12};

const GpioPinRecord gpio_pins[] = {
    // NOX (WeAct WB55CGU6): the stock 18-pin header does not exist. Most pins are
    // consumed by the shared SPI1 bus (PB3/4/5), module CS/IRQ, and the 6 buttons.
    // Only debug pins are exposed here (debug=true => not driven to analog at boot).
    // PC0/PC1/PC3 are not bonded on UFQFPN48 and are intentionally omitted.
    {.pin = &gpio_swclk,
     .name = "PA14",
     .channel = FuriHalAdcChannelNone,
     .number = 10,
     .debug = true},
    {.pin = &gpio_swdio,
     .name = "PA13",
     .channel = FuriHalAdcChannelNone,
     .number = 12,
     .debug = true},
    {.pin = &gpio_usart_tx,
     .name = "PB6",
     .channel = FuriHalAdcChannelNone,
     .number = 13,
     .debug = true},
    {.pin = &gpio_usart_rx,
     .name = "PB7",
     .channel = FuriHalAdcChannelNone,
     .number = 14,
     .debug = true},
};

const size_t gpio_pins_count = COUNT_OF(gpio_pins);

const InputPin input_pins[] = {
    {.gpio = &gpio_button_up, .key = InputKeyUp, .inverted = true, .name = "Up"},
    {.gpio = &gpio_button_down, .key = InputKeyDown, .inverted = true, .name = "Down"},
    {.gpio = &gpio_button_right, .key = InputKeyRight, .inverted = true, .name = "Right"},
    {.gpio = &gpio_button_left, .key = InputKeyLeft, .inverted = true, .name = "Left"},
    {.gpio = &gpio_button_ok, .key = InputKeyOk, .inverted = true, .name = "OK"},
    {.gpio = &gpio_button_back, .key = InputKeyBack, .inverted = true, .name = "Back"},
};

const size_t input_pins_count = COUNT_OF(input_pins);

__attribute__((unused)) static void furi_hal_resources_init_input_pins(GpioMode mode) {
    for(size_t i = 0; i < input_pins_count; i++) {
        furi_hal_gpio_init(
            input_pins[i].gpio,
            mode,
            (input_pins[i].inverted) ? GpioPullUp : GpioPullDown,
            GpioSpeedLow);
    }
}

static void furi_hal_resources_init_gpio_pins(GpioMode mode) {
    for(size_t i = 0; i < gpio_pins_count; i++) {
        if(!gpio_pins[i].debug) {
            furi_hal_gpio_init(gpio_pins[i].pin, mode, GpioPullNo, GpioSpeedLow);
        }
    }
}

void furi_hal_resources_init_early(void) {
    furi_hal_bus_enable(FuriHalBusGPIOA);
    furi_hal_bus_enable(FuriHalBusGPIOB);
    furi_hal_bus_enable(FuriHalBusGPIOC);
    furi_hal_bus_enable(FuriHalBusGPIOD);
    furi_hal_bus_enable(FuriHalBusGPIOE);
    furi_hal_bus_enable(FuriHalBusGPIOH);

#ifdef FURI_HAL_RESOURCES_INPUT_165
    furi_hal_resources_165_init();
#else
    furi_hal_resources_init_input_pins(GpioModeInput);
#endif

    // Explicit, surviving reset, pulls
    LL_PWR_EnablePUPDCfg();
    // NOX: PA8=NFC_CS, PB8=UP button, PB9=BACK button - drop stock speaker/vibro/IR
    // survive-reset pulldowns that would fight those functions on this board.

    // NOX: not a stepdown control here - PE4 (on-board blue LED / park pin / nRF24 CE).
    // Open-drain + high leaves it high-Z, so a driver that claims it (nRF24 re-inits it as
    // push-pull) starts from a clean, undriven pin. furi_hal_power_*_external_3_3v() are
    // no-ops on this target so nothing fights that owner later.
    furi_hal_gpio_write(&gpio_periph_power, 1);
    furi_hal_gpio_init(&gpio_periph_power, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);

    // Display pins
    furi_hal_gpio_write(&gpio_display_rst_n, 0);
    furi_hal_gpio_init_simple(&gpio_display_rst_n, GpioModeOutputPushPull);
    LL_PWR_EnableGPIOPullUp(LL_PWR_GPIO_B, LL_PWR_GPIO_BIT_0); // gpio_display_rst_n
    furi_hal_gpio_write(&gpio_display_di, 0);
    furi_hal_gpio_init_simple(&gpio_display_di, GpioModeOutputPushPull);
    LL_PWR_EnableGPIOPullDown(LL_PWR_GPIO_B, LL_PWR_GPIO_BIT_1); // gpio_display_di

    // Hard reset USB
    furi_hal_gpio_write(&gpio_usb_dm, 1);
    furi_hal_gpio_write(&gpio_usb_dp, 1);
    furi_hal_gpio_init_simple(&gpio_usb_dm, GpioModeOutputOpenDrain);
    furi_hal_gpio_init_simple(&gpio_usb_dp, GpioModeOutputOpenDrain);
    furi_hal_gpio_write(&gpio_usb_dm, 0);
    furi_hal_gpio_write(&gpio_usb_dp, 0);
    furi_delay_us(5); // Device Driven disconnect: 2.5us + extra to compensate cables
    furi_hal_gpio_write(&gpio_usb_dm, 1);
    furi_hal_gpio_write(&gpio_usb_dp, 1);
    furi_hal_gpio_init_simple(&gpio_usb_dm, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_usb_dp, GpioModeAnalog);
    furi_hal_gpio_write(&gpio_usb_dm, 0);
    furi_hal_gpio_write(&gpio_usb_dp, 0);

    // External header pins
    furi_hal_resources_init_gpio_pins(GpioModeAnalog);
}

void furi_hal_resources_deinit_early(void) {
#ifndef FURI_HAL_RESOURCES_INPUT_165
    furi_hal_resources_init_input_pins(GpioModeAnalog);
#endif
    furi_hal_bus_disable(FuriHalBusGPIOA);
    furi_hal_bus_disable(FuriHalBusGPIOB);
    furi_hal_bus_disable(FuriHalBusGPIOC);
    furi_hal_bus_disable(FuriHalBusGPIOD);
    furi_hal_bus_disable(FuriHalBusGPIOE);
    furi_hal_bus_disable(FuriHalBusGPIOH);
}

void furi_hal_resources_init(void) {
#ifndef FURI_HAL_RESOURCES_INPUT_165
    // Button pins
    furi_hal_resources_init_input_pins(GpioModeInterruptRiseFall);
#endif

    // SD pins
    // NOX: Adafruit 4682 DET is LOW when empty, pulled to 3V when a card is inserted.
    // Pull-down so an UNWIRED detect line reads low = "no card" (never hangs on mount).
    furi_hal_gpio_init(&gpio_sdcard_cd, GpioModeInput, GpioPullDown, GpioSpeedLow);

    furi_hal_gpio_init(&gpio_ibutton, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    furi_hal_gpio_init(&gpio_nfc_irq_rfid_pull, GpioModeInterruptRise, GpioPullNo, GpioSpeedLow);

    furi_hal_gpio_init(&gpio_rf_sw_0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

    NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI0_IRQn);

    NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI1_IRQn);

    NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI2_IRQn);

    NVIC_SetPriority(EXTI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI3_IRQn);

    NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI4_IRQn);

    NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    FURI_LOG_I(TAG, "Init OK");
}

int32_t furi_hal_resources_get_ext_pin_number(const GpioPin* gpio) {
    for(size_t i = 0; i < gpio_pins_count; i++) {
        if(gpio_pins[i].pin == gpio) {
            return gpio_pins[i].number;
        }
    }
    return -1;
}

const GpioPinRecord* furi_hal_resources_pin_by_name(const char* name) {
    for(size_t i = 0; i < gpio_pins_count; i++) {
        const GpioPinRecord* record = &gpio_pins[i];
        if(strcasecmp(name, record->name) == 0) return record;
    }
    return NULL;
}

const GpioPinRecord* furi_hal_resources_pin_by_number(uint8_t number) {
    for(size_t i = 0; i < gpio_pins_count; i++) {
        const GpioPinRecord* record = &gpio_pins[i];
        if(record->number == number) return record;
    }
    return NULL;
}
