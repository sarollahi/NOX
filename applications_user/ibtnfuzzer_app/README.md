# iButton Fuzzer (NOX port)

Bruteforces / fuzzes 1‑Wire iButton IDs (Dallas/Cyfral/Metakom) — sweeps values or replays a list
against an iButton reader.

- **Upstream:** [xMasterX/ibutton-fuzzer](https://github.com/xMasterX/ibutton-fuzzer)
- **Hardware:** iButton contact on **PA10** (+ 1 kΩ pull-up).

**NOX port:** appid lowercased (`iBtn_Fuzzer` → `ibtn_fuzzer`; the build silently skips
non-lowercase appids) with `fap_icon_assets_symbol` pinned so the generated icon header still
resolves; `u_int8_t` → `uint8_t`; callback signatures updated to the current
`view_port_input_callback_set` / `furi_timer_alloc`; removed two `free()` calls on embedded
(non-heap) arrays.
