# Uniform layer toggles — design

## Problem

Sticky layer toggles are driven through `leader,<key>` sequences, but the code
treats them inconsistently:

- **Russian** is tracked with a bespoke `ru_enabled` flag plus a
  `ru_suspend_depth` counter and its own `ru_apply/ru_suspend/ru_resume/
  ru_enter/ru_exit` functions.
- **FKEYS_SYS / MOUSE / NUM_NAV** are plain `layer_on(...)` calls with no
  tracked state.
- **Disabling** is fragmented across three sequences: `leader,space`
  (`layer_move(L_BOO)`), `leader,e` (`ru_exit`), and `leader,j` / `leader,_`
  (`oneshot_cancel` + `ru_exit`).

This causes a latent bug: `leader,space → layer_move(L_BOO)` clears the visible
Russian layer but **not** `ru_enabled`, so after the next mod press+release
`ru_apply()` turns Russian back on.

## Goals

1. Treat every sticky toggle layer uniformly with one mechanism.
2. Russian suspends while a mod is held, then resumes (unchanged behavior).
3. Each layer is enabled by its own leader sequence.
4. One leader sequence disables whatever toggle layer is active.
5. Play nice with native QMK `OSL()` — never stomp a pending one-shot layer.

## Non-goals

- Stacking multiple toggle layers at once. Model is strictly **one active
  toggle layer at a time** (enabling one replaces the previous).
- Changing `OSL()` access to NUM_NAV / FKEYS_SYS / SYMBOLS. Those remain native
  QMK one-shot layers, driven by combos / the symbols thumb, untouched by this
  work.

## Background: OSL is native QMK

Confirmed and double-checked: `OSL(L_NUM_NAV)`, `OSL(L_FKEYS_SYS)`, and
`OSL(L_SYMBOLS)` are stock QMK keycodes. The custom Callum-style oneshot module
(`modules/shofel/oneshot/oneshot.c`) only drives the **mod** oneshots
(`OS_CTL/ALT/GUI/SFT`); `is_oneshot_ignored_key()` returns `true` for the three
`OSL(...)` keycodes so they flow through to QMK's built-in OSL handling
untouched. No custom set/clear-oneshot-layer code exists anywhere.

The "play nice with OSL" requirement is therefore satisfied structurally: the
new mechanism only ever calls `layer_on`/`layer_off` on the single sticky layer
it owns, and never `layer_move` (which would wipe a pending OSL layer and reset
the default layer).

## Design — single active-layer state machine

### State

```
active_toggle    : NONE | L_RUSSIAN | L_FKEYS_SYS | L_MOUSE | L_NUM_NAV
leader_active    : bool     // a leader sequence is in progress: mask ANY active layer
mod_ru_suspend_depth: uint8_t   // nested mod holds: mask Russian only
applied_layer    : NONE | <layer>   // the sticky layer physically on; we own it
```

There are two distinct reasons to mask the active layer, with different scope:

- **`leader_active`** — while a leader sequence is being typed, mask *whatever*
  toggle layer is active so the sequence keys resolve on the base BOO layout.
  Leader sequences are letter-based (`leader,t`, `leader,f`, …); without this,
  with NUM_NAV/MOUSE/FKEYS active the sequence key resolves on that overlay
  (e.g. the key at T's position is not `KC_T`) and the activator silently fails.
- **`mod_ru_suspend_depth`** — while a mod (Ctrl/Alt/Gui oneshot) is held, mask
  *Russian only*, so mod combos fall through to the Latin base while NUM_NAV
  etc. stay put (Ctrl+arrow keeps working inside num/nav).

`active_toggle` is the user's intent ("I want Russian on"). `applied_layer` is
what is physically toggled on right now. Keeping them separate is what makes the
reconcile surgical and OSL-safe.

`NONE` is represented by a sentinel value that is not a valid layer id (e.g.
`0xFF`).

### Reconcile

```
toggle_apply():
    want = active_toggle
    if leader_active:
        want = NONE                     // leader seq reads keys from base
    elif active_toggle == L_RUSSIAN and mod_ru_suspend_depth > 0:
        want = NONE                     // mods mask only Russian
    if applied_layer == want:
        return                          // nothing changed
    if applied_layer != NONE:
        layer_off(applied_layer)        // only ever our own layer
    if want != NONE:
        layer_on(want)
    applied_layer = want
```

`toggle_apply()` only ever toggles the one layer it manages, so any native OSL
momentary layer on top is left intact. The default layer (`L_BOO`, layer 0) is
never moved — base is always present underneath.

The two masks are checked in order: `leader_active` wins (mask everything),
otherwise the mod path masks Russian only. Only Russian has the
suspend-under-mods property, encoded directly in `toggle_apply()`
(`active_toggle == L_RUSSIAN`) rather than a per-layer table, since exactly one
layer needs it.

### Operations

```
toggle_enable(layer):
    active_toggle = layer
    toggle_apply()                      // replaces any previously active toggle

toggle_disable():                       // Esc
    active_toggle = NONE
    mod_ru_suspend_depth = 0
    toggle_apply()

toggle_reset():                         // leader,space — full reset
    oneshot_cancel()
    toggle_disable()

mod_ru_suspend():                          // mod down (oneshot Ctrl/Alt/Gui)
    mod_ru_suspend_depth += 1
    toggle_apply()

mod_ru_resume():                           // mod up
    if mod_ru_suspend_depth > 0:
        mod_ru_suspend_depth -= 1
    toggle_apply()

leader_suspend():                          // leader sequence start
    leader_active = true
    toggle_apply()

leader_resume():                           // leader sequence end
    leader_active = false
    toggle_apply()
```

The old `ru_enter/ru_exit/ru_apply/ru_suspend/ru_resume` functions are replaced
by the above. `mod_ru_suspend`/`mod_ru_resume` are called from
`oneshot_process_event` (mods, Russian-only mask); `leader_suspend`/
`leader_resume` are called from `leader_start_user`/`leader_end_user` (mask any
active layer).

### Leader sequence mapping

Enable (each layer its own seq):

```
leader,r  -> toggle_enable(L_RUSSIAN)                       // current unicode mode
leader,l  -> set_unicode_input_mode(LINUX); toggle_enable(L_RUSSIAN)
leader,v  -> set_unicode_input_mode(VIM);   toggle_enable(L_RUSSIAN)
leader,f  -> toggle_enable(L_FKEYS_SYS)
leader,m  -> toggle_enable(L_MOUSE)
leader,t  -> toggle_enable(L_NUM_NAV)
```

Disable (one for all):

```
leader,space -> toggle_reset()
```

Retired (now redundant — `leader,space` covers disable):

```
leader,e   (was ru_exit)
leader,j   (was oneshot_cancel + ru_exit)
leader,_   (was oneshot_cancel + ru_exit)
```

`leader,n` / `leader,s` (Esc), `leader,h` / `leader,w` (Ctrl+Esc), and all
non-layer sequences (text editing, kitty, print screen) keep their key actions.

> **Correction (later):** `leader,n` / `leader,s` were updated to call
> `toggle_disable()` before `tap_code(KC_ESC)` — see the corrected Esc note
> below.

### Esc behavior

`process_record_user` for `KC_ESC` (pressed) calls `toggle_disable()` — Esc
clears whatever toggle layer is active, not just Russian.

**Sanity-check note (corrected):** the `shift+space` combo resolves to the
`KC_ESC` *keycode*, which flows through `process_record` and so hits the
`toggle_disable()` path above. `leader,n` / `leader,s`, however, emit Esc via
`tap_code(KC_ESC)` — `register_code` writes the HID report directly and does
**not** re-enter `process_record`, so the original assumption that they would
exit the toggle was wrong: they only sent a bare Esc. Fixed by calling
`toggle_disable()` explicitly in those handlers before `tap_code(KC_ESC)`, so
"Esc always returns to base" now holds for the leader Esc sequences too, not
just the thumb combo.

### Suspend ordering inside leader

`leader_start_user` calls `leader_suspend()` (sets `leader_active`), so every
key captured during the sequence resolves on the base layout. `leader_end_user`
calls `leader_resume()` **first**, then evaluates the sequence handlers (so an
enable inside the sequence applies with `leader_active` already cleared).
Switching layers via leader (e.g. NUM_NAV active → `leader,f`) briefly
re-applies the old layer on `leader_resume()` then immediately replaces it —
all synchronous within `leader_end_user`, no host output in between, so it is
invisible.

Shift is intentionally excluded from the mod-suspend hook (capitals are valid in
Russian), exactly as today — only `OS_CTL`/`OS_ALT`/`OS_GUI` call
`mod_ru_suspend`/`mod_ru_resume` from `oneshot_process_event`.

## Affected code

All changes are in `layouts/shofel/split_3x6_3/shofel/keymap.c`:

- Replace the Russian state block (`ru_enabled`, `ru_suspend_depth`, `ru_apply`,
  `ru_suspend`, `ru_resume`, `ru_enter`, `ru_exit`) with the uniform state +
  `toggle_apply` / `toggle_enable` / `toggle_disable` / `toggle_reset` /
  `mod_ru_suspend` / `mod_ru_resume` / `leader_suspend` / `leader_resume`.
- Update `oneshot_process_event` to call `mod_ru_suspend`/`mod_ru_resume`
  (mods, Russian-only mask).
- Update `leader_start_user` / `leader_end_user` to call `leader_suspend`/
  `leader_resume` (mask any active layer) and the new enable/reset functions;
  retire `leader,e/j/_`.
- Update `process_record_user` `KC_ESC` to call `toggle_disable()`.

No changes to `config.h`, `rules.mk`, combos, key overrides, or the keymaps
themselves. No module changes.

## Testing / verification

- `make build` for the Cantor target compiles clean.
- Manual on-device checks:
  - `leader,r` enables Russian; typing produces Cyrillic.
  - Hold Ctrl (oneshot) → Russian suspends (Ctrl+C is Latin); release → Russian
    back. Shift still types Cyrillic capitals (not suspended).
  - `leader,f` while Russian active → FKEYS_SYS replaces Russian (one-at-a-time).
  - `leader,space` from any toggle → base, and queued oneshot mods cleared.
  - Esc from any toggle → base.
  - The previously-broken path: `leader,r`, then `leader,space`, then tap+release
    Ctrl → Russian stays **off** (no resurrection).
  - OSL still works: with Russian active, the NUM_NAV combo (OSL) momentarily
    overlays numbers, then reverts to Russian.
  - Leader works from inside an active overlay: `leader,t` to enable NUM_NAV,
    then (with NUM_NAV active) `leader,f` → FKEYS_SYS, and `leader,m` → MOUSE.
    Each activator fires even though a non-base toggle layer was already on.
