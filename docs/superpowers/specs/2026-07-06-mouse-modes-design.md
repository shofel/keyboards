# Mouse modes — design

Date: 2026-07-06

## Goal

Give the mouse layer switchable **pointer modes**, selected from top-row keys of
the left hand's strong fingers:

- `,` (ring)  → **stock**  — the existing QMK mousekeys (`MS_*`), unchanged.
- `c` (index) → **bisect** — binary-search absolute positioning via the QMK digitizer.

**Default is bisect**: `leader,m` enters bisect every time (mode is not persisted).

> **Radial mode (`u`) was dropped.** See "Radial — deferred" below.

## Modes as toggle sub-layers

Stock and bisect each want their own native keycodes live on the layer (`MS_*`
vs custom), so each mode is its own layer:

    L_MOUSE          stock  (existing MS_* layer, + mode keys)
    L_MOUSE_BISECT   bisect (custom KK_BI_* keys)

Layer count goes 6 → 7 (fits `LAYER_STATE_8BIT`).

These are ordinary **toggle layers** on the existing single-active-toggle system
(`toggle_enable` / `toggle_disable`, one at a time, leader-masked). The two mode
keys are custom keycodes (`KK_MM_STOCK`, `KK_MM_BISECT`) present on **both**
sub-layers (top-left cols 2/4, `,` and `c`), each calling `toggle_enable(<its
layer>)`. Switching mode swaps the one active toggle layer. `leader,m` calls
`toggle_enable(L_MOUSE_BISECT)`. Exit is unchanged: Esc (shift+space combo) or
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

## Radial — deferred

The plan was to add a radial/polar mode via getreuer's Orbital Mouse community
module (`getreuer/orbital_mouse`). Dropped after inspecting the module: its
`OM_*` keycodes are **aliases of the `MS_*` mousekeys** (`OM_U == MS_UP`, …) and
its `qmk_module.json` sets `"mousekey": false` — it *replaces* QMK mousekeys and
repurposes their keycodes. So "stock mousekeys" and "orbital radial" cannot both
be live modes in one firmware. Keeping true stock mousekeys (with the tuned
`MK_*` config) was chosen over radial. Revisit only if willing to drop stock, in
which case orbital's cardinal (`OM_CS_*`) keys can stand in for a stock-like feel.

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
- No persistence of the last-used mode (always start in bisect, by request).
- `u` is unused on the mouse layers (was to be radial).
