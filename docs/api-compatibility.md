# FAP compatibility on `f55`

Why prebuilt Flipper apps from the stock catalogue do not load on NOX, what the `f55` API delta
actually is, and what it would take to change that.

## Two gates, not one

A `.fap` is checked twice before it runs. Both live in
[`lib/flipper_application`](../lib/flipper_application):

**1. Hardware target — exact match, no negotiation.**

```c
// lib/flipper_application/application_manifest.c
bool flipper_application_manifest_is_target_compatible(const FlipperApplicationManifest* manifest) {
    const Version* version = furi_hal_version_get_firmware_version();
    return version_get_target(version) == manifest->base.hardware_target_id;
}
```

`version_get_target()` returns the `TARGET` the *firmware* was built with (`lib/toolbox/version.c`
sets `.target = TARGET`, generated from `TARGET_HW`). A fap's `hardware_target_id` is the
`TARGET_HW` it was built with. So an `f55` firmware reports **55**, every fap in the stock
Unleashed/OFW catalogue reports **7**, and the comparison fails before anything else is looked at.

**2. API version — major must not be older than the firmware's.**

```c
// flipper_application_manifest_is_valid()
if(manifest->base.api_version.major < api_interface->api_version_major) return false;
```

Note the direction: a fap built against an *older* major is rejected. The minor check is
commented out upstream, so minor differences are ignored.

### Consequence

Pinning or freezing the `f55` API version **on its own changes nothing** for stock catalogue
faps. Gate 1 rejects them first. This is the correction to the obvious plan: the API delta is
not what is keeping the catalogue out.

To actually load stock `f7` faps, an `f55` build has to report `TARGET == 7`, *and* its API
major must be no newer than the faps'. Both are required; either alone is not enough.

## The `f55` API delta

`targets/f55/api_symbols.csv` is at **89.5**; `targets/f7/api_symbols.csv` is at **88.4**. The
whole difference is 68 CSV lines:

| Change | Entries | Effect on version |
|---|---|---|
| `infrared_signal_*` promoted from removed to active | 16 | minor |
| `infrared_brute_force_*` promoted from removed to active | 10 | minor |
| `furi_hal_resources_165_init` / `_scan` added | 2 | minor |
| `furi_hal_resources_165_state` added | 1 | minor |
| `cc1101_ext_interconnect.h` added | 1 | minor |
| `subghz_device_cc1101_ext_ep` **removed from active API** | 1 | **major** |

The major bump from 88 to 89 comes from exactly one line. `scripts/fbt/sdk/cache.py` bumps major
whenever a previously-active entry disappears:

```python
# If any of removed entries was a part of active API, that's a major bump
if update_version and any(...): self.version_action = VersionBump.MAJOR
```

`subghz_device_cc1101_ext_ep` went away because `applications/drivers/subghz/application.fam`
declares `targets=["f7"]`, so the external-CC1101 plugin is not built for `f55` (deliberately —
its pins are wrong for this board, see `targets/f55/README_NOX.md` §5). Everything else in the
delta is additive.

So `f55`'s API is a **superset of f7's 88.x plus one removal**. Keeping that one symbol in the
tracked surface would hold the major at 88 and make `f55` a clean superset.

## If you want stock faps to load

In rough order of effort. None of this is currently done; the project vendors and rebuilds its
apps instead, which is why `applications_user/` carries 230+ trees.

1. **Report target 7.** Build with the firmware's `TARGET` forced to 7 while keeping the `f55`
   HAL. Gate 1 then passes for every stock fap.
2. **Hold the API major at 88.** Keep `subghz_device_cc1101_ext_ep` in `api_symbols.csv` (it can
   resolve to a stub that reports "not present"), so nothing is *removed* relative to f7 and the
   major never bumps. Additive entries only move the minor, which is not checked.
3. **Accept the consequences.** Every fap will then believe it is on a real Flipper. Apps that
   drive hardware this board does not have will open and do nothing rather than being filtered
   out of the menu. Per `README_NOX.md` §7 that is already the behaviour for absent peripherals
   (they return errors instead of crashing), so it is survivable, but it is a real UX change:
   nothing will warn the user that LF RFID has no coil behind it.

The trade is: stop vendoring 230+ app trees, in exchange for a menu full of apps that cannot all
work. Worth it for a build aimed at running the public catalogue; not worth it for a curated
firmware.

## Keeping the delta stable

Whatever is decided above, the delta is only 68 lines and should stay that small:

- Do not add `"f55"` to a fam whose driver has wrong pins just to silence an API diff.
- After a change that shifts the SDK surface, `fbt` stops with *"SDK version is not finalized"*.
  That is the tracking prompt, not an error. Review what moved before accepting the marks:

  ```bash
  diff <(sort targets/f7/api_symbols.csv) <(sort targets/f55/api_symbols.csv)
  ```

- A line appearing as `Function,-,...` in `f55` but `Function,+,...` in `f7` is a **removal** and
  will bump the major. That is the one thing to look for.
