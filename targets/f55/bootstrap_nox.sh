#!/usr/bin/env bash
# End-to-end build bootstrap for the NOX f55 target (WeAct STM32WB55CGU6).
#
# This repo copy has EMPTY submodules and is not a git checkout, so it cannot build
# here. Run this on a machine with git + internet: it clones a complete Unleashed
# tree (with submodules), drops in the f55 target, registers TARGET_HW=55, and builds
# a flashable .dfu.
#
#   ./bootstrap_nox.sh [dest_clone_dir]   # default: ~/unleashed-nox
set -euo pipefail

F55_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"   # this script's dir = the f55 target
CLONE="${1:-$HOME/unleashed-nox}"
REPO="https://github.com/DarkFlippers/unleashed-firmware.git"

if [ ! -d "$CLONE/.git" ]; then
  echo ">> cloning Unleashed (recursive) into $CLONE"
  git clone --recursive "$REPO" "$CLONE"
else
  echo ">> using existing clone $CLONE ; syncing submodules"
  git -C "$CLONE" submodule update --init --recursive
fi

echo ">> installing f55 target"
mkdir -p "$CLONE/targets/f55"
cp -R "$F55_SRC/." "$CLONE/targets/f55/"

echo ">> registering TARGET_HW=55 in site_scons/commandline.scons"
SC="$CLONE/site_scons/commandline.scons"
if ! grep -q '"55"' "$SC"; then
  python3 - "$SC" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
needle='            "7",\n            "18",\n'
if needle in s:
    s=s.replace(needle, needle+'            "55",\n', 1)
    open(p,'w').write(s)
    print("   patched enum")
else:
    sys.exit("   !! could not find TARGET_HW enum; add \"55\" to allowed_values manually")
PY
fi
grep -q '"55"' "$SC" || { echo "!! TARGET_HW=55 not registered"; exit 1; }

echo ">> patching shared input.c for SN74HC165 button scan (poll instead of EXTI)"
# The '165 button input lives partly in the SHARED input service, which bootstrap
# does NOT copy (only targets/f55 is copied). Patch it in the clone. Idempotent,
# and gated on FURI_HAL_RESOURCES_INPUT_165 (defined only in the f55 header) so f7
# builds are byte-identical.
python3 - "$CLONE/applications/services/input/input.c" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
if "FURI_HAL_RESOURCES_INPUT_165" in s:
    print("   input.c already patched"); sys.exit(0)
edits=[
("""    const FuriThreadId thread_id = furi_thread_get_current_id();
    FuriPubSub* event_pubsub = furi_pubsub_alloc();""",
"""    const FuriThreadId thread_id = furi_thread_get_current_id();
#ifdef FURI_HAL_RESOURCES_INPUT_165
    UNUSED(thread_id); // '165 path polls, no per-button EXTI callback registered
#endif
    FuriPubSub* event_pubsub = furi_pubsub_alloc();"""),
("#define GPIO_Read(input_pin) (furi_hal_gpio_read(input_pin.pin->gpio) ^ (input_pin.pin->inverted))",
"""#ifdef FURI_HAL_RESOURCES_INPUT_165
// NOX: read the latched '165 byte by key index instead of a live GPIO.
#define GPIO_Read(input_pin) \\
    (((furi_hal_resources_165_state >> (input_pin).pin->key) & 1U) ^ (input_pin).pin->inverted)
#else
#define GPIO_Read(input_pin) (furi_hal_gpio_read(input_pin.pin->gpio) ^ (input_pin.pin->inverted))
#endif"""),
("""    for(size_t i = 0; i < input_pins_count; i++) {
        furi_hal_gpio_add_int_callback(input_pins[i].gpio, input_isr, thread_id);
        pin_states[i].pin = &input_pins[i];""",
"""#ifdef FURI_HAL_RESOURCES_INPUT_165
    furi_hal_resources_165_scan(); // prime latched state before first read
#endif
    for(size_t i = 0; i < input_pins_count; i++) {
#ifndef FURI_HAL_RESOURCES_INPUT_165
        furi_hal_gpio_add_int_callback(input_pins[i].gpio, input_isr, thread_id);
#endif
        pin_states[i].pin = &input_pins[i];"""),
("""    while(1) {
        bool is_changing = false;""",
"""    while(1) {
#ifdef FURI_HAL_RESOURCES_INPUT_165
        furi_hal_resources_165_scan();
#endif
        bool is_changing = false;"""),
("            furi_thread_flags_wait(INPUT_THREAD_FLAG_ISR, FuriFlagWaitAny, FuriWaitForever);",
"""#ifdef FURI_HAL_RESOURCES_INPUT_165
            // No button EXTI on the '165 path: poll every 10ms.
            furi_thread_flags_wait(INPUT_THREAD_FLAG_ISR, FuriFlagWaitAny, 10);
#else
            furi_thread_flags_wait(INPUT_THREAD_FLAG_ISR, FuriFlagWaitAny, FuriWaitForever);
#endif"""),
]
for old,new in edits:
    assert s.count(old)==1, "input.c seam not found/ambiguous: %r"%old[:50]
    s=s.replace(old,new,1)
open(p,"w").write(s); print("   input.c patched (4 seams)")
PY

echo ">> enabling subghz_remote fap for f55 (SubGHz works; costs 0 firmware flash)"
python3 - "$CLONE/applications/main/subghz_remote/application.fam" <<'PY2'
import sys
p=sys.argv[1]
try: s=open(p).read()
except FileNotFoundError: sys.exit(0)
if 'targets=["f7", "f55"]' in s: print("   already enabled"); sys.exit(0)
parts=s.split('targets=["f7"]')  # last occurrence = active (uncommented) block
if len(parts)>=2:
    s='targets=["f7"]'.join(parts[:-1])+'targets=["f7", "f55"]'+parts[-1]
    open(p,'w').write(s); print("   subghz_remote enabled for f55")
PY2

# NOTE: do NOT bulk-enable apps that declare targets=["f7"] (NFC parsers, CLI
# plugins, etc). Adding them grew firmware 812K -> 864K, which overruns the BLE
# radio stack near the top of the 1MB flash (linker does not reserve it) and the
# board hardfaults at boot with no USB. Keep firmware <= ~812K unless you first
# confirm the real radio-stack address and trim to fit below it.

echo ">> building (.dfu) — first run downloads the ARM toolchain"
cd "$CLONE"

# Accept SDK API pending marks (?/v -> +) if a build stops on API finalization.
# Adding a target with its own headers shifts the tracked SDK API once; this is the
# documented Flipper firmware-dev step, not a code error.
finalize_api() {
  for csv in targets/f55/api_symbols.csv targets/f7/api_symbols.csv; do
    [ -f "$csv" ] && sed -i '' -E 's/^([A-Za-z]+),\?,/\1,+,/; s/^Version,v,/Version,+,/' "$csv" 2>/dev/null \
      || { [ -f "$csv" ] && sed -i -E 's/^([A-Za-z]+),\?,/\1,+,/; s/^Version,v,/Version,+,/' "$csv"; }
  done
}

if ! ./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package; then
  echo ">> build stopped (likely SDK API finalization) — accepting API marks and retrying once"
  finalize_api
  ./fbt TARGET_HW=55 COMPACT=1 DEBUG=0 updater_package
fi

echo
echo ">> BUILD OK. Artifacts:"
ls -1 "$CLONE"/dist/f55-update-*/*.dfu 2>/dev/null || echo "   (look in $CLONE/dist/f55-update-*/)"
cat <<'NEXT'

Next (hardware, per targets/f55/README_NOX.md):
  1) OTP -> 0x1FFF7000  (qFlipper-OTP tool + STM32CubeProgrammer, hold BOOT0 at connect)
  2) Flash the .dfu above (qFlipper "Install from file" or STM32CubeProgrammer)
  3) Copro stack:  ./fbt TARGET_HW=55 copro_dist   (or flash stock radio stack via qFlipper)
  4) microSD: FAT32 + Unleashed resources
  5) Wire modules to the pinout table in README_NOX.md
NEXT
