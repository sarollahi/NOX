# ProtoView (NOX port)

Raw Sub‑GHz signal viewer/analyzer by antirez — detect, visualize, decode, edit and replay
digital RF signals sample-by-sample. Excellent for reversing unknown OOK/2‑FSK protocols.

- **Upstream:** [antirez/protoview](https://github.com/antirez/protoview)
- **Bay:** none — uses the on-board CC1101.

**NOX port:** the firmware dropped `furi_hal_subghz_load_preset(enum)`; added a local shim mapping
ProtoView's presets (OOK270/650, 2FSK dev2.38/47.6) to the exported `subghz_device_cc1101_preset_*_regs[]`
tables via `furi_hal_subghz_load_custom_preset`. Also stubbed the removed `lib/toolbox/random_name.h`
with a local `set_random_name` (uses `furi_hal_random_get`).
