# Mouse modes — design

Date: 2026-07-06

## Goal

Give the mouse layer switchable **pointer modes**, selected from top-row keys of
the left hand's strong fingers:

- `,` (ring)  → **polar**  — Orbital Mouse (`getreuer/orbital_mouse`), a heading-based pointer (`OM_*`).
- `c` (index) → **bisect** — binary-search absolute positioning via the QMK digitizer.

**Default is polar**: `leader,m` enters polar every time (mode is not persisted).
It was bisect originally; see "Why polar became the default" below.

> **Polar mode replaced the original "stock" QMK mousekeys.** See "Polar (Orbital Mouse) — shipped" below.

## Modes as toggle sub-layers

Polar and bisect each want their own native keycodes live on the layer (`OM_*`
vs custom), so each mode is its own layer:

    L_MOUSE          polar  (Orbital Mouse OM_* layer, + mode keys)
    L_MOUSE_BISECT   bisect (custom KK_BI_* keys)

Layer count goes 6 → 7 (fits `LAYER_STATE_8BIT`).

These are ordinary **toggle layers** on the existing single-active-toggle system
(`toggle_enable` / `toggle_disable`, one at a time, leader-masked). The two mode
keys are custom keycodes (`KK_MM_POLAR`, `KK_MM_BISECT`) present on **both**
sub-layers (top-left cols 2/4, `,` and `c`), each calling `toggle_enable(<its
layer>)`. Switching mode swaps the one active toggle layer. `leader,m` calls
`toggle_enable(L_MOUSE)`. Exit is unchanged: Esc (shift+space combo) or
`leader,space` → `toggle_disable` / `toggle_reset`.

## Bisect — algorithm

Keep a normalized bounding box `{x0,x1,y0,y1}` in digitizer coordinates (0..1,
origin top-left). Each halving key discards the far half along one axis and moves
the absolute pointer to the **center of the surviving box** ("halve & re-center"
— literal binary search):

    left   : x1 = (x0+x1)/2
    right  : x0 = (x0+x1)/2
    up     : y1 = (y0+y1)/2      (origin is top, so "up" keeps the smaller y)
    down   : y0 = (y0+y1)/2
    reset  : box = full screen
    (after every one) digitizer_set_position((x0+x1)/2, (y0+y1)/2)

Keys mirror the stock arrow cluster (right hand): up / left / down / right for
the four halvings, index = **click**, plus a **reset** key. Click holds the
digitizer tip switch (down on press, up on release) — a real click duration,
drag-capable. The box math lives in header-only `bisect_geom.h`, unit-tested
off-target (see Testing).

## Enter / exit hook

`layer_state_set_user` watches `L_MOUSE_BISECT`:

- became active   → `digitizer_in_range_on()`, reset box, center pointer.
- became inactive → release a held tip, then `digitizer_in_range_off()`.

Fires however the layer is left (mode switch, Esc, `leader,space`), so the
digitizer is always released when leaving bisect. Outside bisect the digitizer
is out of range, so the host ignores it and normal input is unaffected.

## Polar (Orbital Mouse) — shipped

`L_MOUSE` is now **polar** control via getreuer's Orbital Mouse community module
(`getreuer/orbital_mouse`, vendored under `modules/getreuer/`). The pointer moves
along a heading: `OM_U`/`OM_D` go forward/backward, `OM_L`/`OM_R` steer, and
`OM_SLOW`/`OM_FAST` change speed while held; buttons and wheel are `OM_BTN*` /
`OM_W_*`.

Orbital's `OM_*` keycodes are **aliases of the `MS_*` mousekeys** (`OM_U == MS_UP`,
…) and its `qmk_module.json` sets `"mousekey": false` — it *replaces* QMK
mousekeys. So stock mousekeys and orbital cannot both be live; adopting polar
meant dropping stock. Accordingly `MOUSEKEY_ENABLE = no`, and the old `MK_*`
tuning in `config.h` was removed. Bisect is unaffected (it uses the digitizer,
not mousekeys).

## Why polar became the default

Bisect was the original default, but it does nothing on this host. The Cantor
presents its digitizer as `usb:feed:0000`, which libwacom has no entry for, so
Linux never binds it as a tablet and the hover motion drives no cursor. The
firmware side was instrumented and proven correct end to end — keycodes, box
math, digitizer calls and the USB endpoint budget all check out — so this is
purely a host-side gap.

Landing in a dead mode on every `leader,m` is a bad default, so `leader,m` now
enters **polar**, which works today. Bisect stays one keypress away on `c`, and
becomes a viable default again the moment the host recognises the device.

## Testing

- **Unit (off-target, test-first):** `tools/test_bisect_geom.c` compiles
  `bisect_geom.h` with plain `gcc` and asserts halving sequences + centers,
  including axis direction. Run: `make test-bisect`.
- **Compile:** `make build` compiles cleanly (verification bar for this PR).
- **Hardware QA (owner: user):** flash (`make flash`) and physically verify on the
  Cantor. Automated tests cannot validate real pointer behavior.

## Known risks — resolve at hardware QA

1. **Digitizer tap == left click?** GNOME may treat a digitizer tip tap differently
   from a mouse button click in some apps. If so, swap the click key to `MS_BTN1`
   (one line). Flagged, not pre-solved.
2. **Multi-monitor:** the host maps 0..1 across the whole virtual desktop, so
   "center" is the middle of all displays, not the focused one.
3. **Flash size:** mousekeys + digitizer together; LTO is on. `make build` confirms
   it fits (≈37.8 KB).

## Out of scope

- No auto-reset of the bisect box after a click (reset is one key; predictable).
- No persistence of the last-used mode (always start in polar).
- `u` is unused on the mouse layers (was to be radial).
