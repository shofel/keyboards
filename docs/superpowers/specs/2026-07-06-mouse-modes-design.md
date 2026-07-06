# Mouse modes — design

Date: 2026-07-06

## Goal

Give the mouse layer three switchable **pointer modes**, selected from three
top-row keys of the left hand's strong fingers (`,` `u` `c`, comfort u > c > ,):

- `,` (ring)  → **stock**  — the current QMK mousekeys (`MS_*`).
- `u` (mid)   → **radial** — [getreuer's Orbital Mouse][om] community module (`OM_*`).
- `c` (index) → **bisect** — binary-search absolute positioning via the QMK digitizer.

**Default is bisect**: `leader,m` enters bisect every time (mode is not persisted).

[om]: https://getreuer.info/posts/keyboards/orbital-mouse/index.html

## Modes as three toggle sub-layers

Stock, radial and bisect each want their own *native* keycodes live on the layer
(`MS_*`, `OM_*`, custom), so each mode is its own layer rather than one layer with
a runtime flag:

    L_MOUSE          stock  (existing MS_* layer, unchanged except mode keys added)
    L_MOUSE_RADIAL   radial (OM_* keys)
    L_MOUSE_BISECT   bisect (custom KK_BI_* keys)

Layer count goes 6 → 8, which exactly fits `LAYER_STATE_8BIT`.

These are ordinary **toggle layers** driven by the existing single-active-toggle
system (`toggle_enable` / `toggle_disable`, one at a time, leader-masked). The three
mode keys are custom keycodes present on **all three** sub-layers (top-left cols
2/3/4, currently empty), each calling `toggle_enable(<its layer>)`. Switching mode
therefore swaps the one active toggle layer. `leader,m` calls
`toggle_enable(L_MOUSE_BISECT)`. Exit stays as today: Esc (shift+space combo) or
`leader,space` → `toggle_disable` / `toggle_reset`.

## Bisect — algorithm

Keep a normalized bounding box `{x0,x1,y0,y1}` in digitizer coordinates (0..1,
origin top-left). Each halving key discards the far half along one axis and moves
the absolute pointer to the **center of the surviving box** ("halve & re-center" —
literal binary search):

    left   : x1 = (x0+x1)/2
    right  : x0 = (x0+x1)/2
    up     : y1 = (y0+y1)/2      (origin is top, so "up" keeps the smaller y)
    down   : y0 = (y0+y1)/2
    reset  : box = full screen
    (after every one) digitizer_set_position((x0+x1)/2, (y0+y1)/2)

Key placement on `L_MOUSE_BISECT` mirrors the stock arrow cluster (right hand):
up / left / down / right for the four halvings, index = **click**, plus a **reset**
key. Click holds the digitizer tip switch (down on press, up on release) — a real
click duration, drag-capable.

The box geometry is pure integer-free float math extracted into a header-only
module `bisect_geom.h`, unit-tested off-target (see Testing).

## Enter / exit hook

`layer_state_set_user` watches `L_MOUSE_BISECT`:

- became active   → `digitizer_in_range_on()`, reset box, center pointer.
- became inactive → `digitizer_in_range_off()`.

This fires however the layer is left (mode switch, Esc, `leader,space`), so the
digitizer is always released when leaving bisect.

## Radial — Orbital Mouse integration

- Vendor `getreuer/qmk-modules` as a git submodule at `modules/getreuer`
  (upstream-prescribed install; the repo's own `shofel/*` modules stay in-tree).
- Register `"getreuer/orbital_mouse"` in `keymap.json` `modules`.
- `L_MOUSE_RADIAL` uses `OM_U/OM_D` (move along heading), `OM_L/OM_R` (steer),
  `OM_BTN1/2`, wheel `OM_W_*`, mapped onto the mouse layout.
- Keep `MOUSEKEY_ENABLE = yes` (stock mode needs it); orbital coexists.
- Requires QMK community modules (≥ 0.28.0). Build note: `git submodule update
  --init` after clone.

## Testing

- **Unit (off-target, test-first):** `tools/test_bisect_geom.c` compiles
  `bisect_geom.h` with plain `gcc` and asserts halving sequences + centers,
  including axis direction (catches an inverted up/down). Run: `make test-bisect`
  (or `gcc` one-liner documented in the test).
- **Compile:** `make build` must compile cleanly (verification bar for this PR).
- **Hardware QA (owner: user):** flash (`make flash`) and physically verify on the
  Cantor. Automated tests cannot validate real pointer behavior.

## Known risks — resolve at hardware QA

1. **Digitizer tap == left click?** GNOME may treat a digitizer tip tap differently
   from a mouse button click in some apps. If so, swap the click key to `MS_BTN1`
   (one line). Flagged, not pre-solved.
2. **Multi-monitor:** the host maps 0..1 across the whole virtual desktop, so
   "center" is the middle of all displays, not the focused one.
3. **Orbital feel / speed curve** is tunable in `config.h` after trying it.
4. **Flash size:** mousekeys + orbital + digitizer together; LTO is on. `make build`
   confirms it fits.

## Out of scope

- No auto-reset of the bisect box after a click (reset is one key; keeps it
  predictable). Revisit if QA wants it.
- No persistence of the last-used mode (always start in bisect, by request).
