# Known limitations & pitfalls

Accepted quirks and non-obvious behaviors of the shofel keymap. These are
documented rather than fixed — each note says why.

## Kitty control keys come out Cyrillic while Russian is active

`leader,k` sends `Gui+T` (kitty's new-window shortcut) correctly, but the
control keys typed *after* it land on the Russian layer and come out Cyrillic.
This is not a leader bug — `leader,k` itself records correctly.

Left as-is: fixing it would mean `leader,k` also dropping the active toggle
layer first, which we don't want.

## Bisect mouse mode does nothing on Linux

The bisect mode (digitizer binary search) is correct in firmware — keycodes,
box math, digitizer calls and the USB endpoint budget were all instrumented and
verified — but the pointer never moves on this host.

Why: the Cantor presents its digitizer as `usb:feed:0000`, and libwacom has no
entry for that ID, so Linux never binds the device as a tablet. Hover motion
from an unbound digitizer drives no system cursor.

Polar (Orbital Mouse) is therefore the default mouse mode; bisect stays one
keypress away on `c`.

The libwacom route is exhausted: a matching `.tablet` entry did not get the
cursor moving. What is left is to stop presenting as a digitizer at all and emit
an absolute-mouse HID descriptor instead — the pointer type a VM's "USB Tablet"
uses, which every OS drives without configuration.

That descriptor, however, lives in QMK **core** (`tmk_core/protocol/usb_descriptor.c`)
as a non-weak `const` with no override hook, and QMK 0.33.13 has no absolute-mouse
build knob (its only absolute pointer *is* the digitizer). So it cannot be done
from this external userspace without patching core — which would reintroduce the
fork this repo deliberately dropped. It is an upstream-QMK-PR (or local-fork)
task, not a keymap one. Before any firmware work, it is cheaper to re-test a
*correct* libwacom `.tablet` entry — the earlier attempt may have been an
ID/descriptor mismatch rather than the lane being truly dead. See TODO.md #1.

## Oneshot layers do not stack (only oneshot modifiers do)

You can stack oneshot **modifiers**: tap `OS_CTL` then `OS_GUI` and both apply
to the next key, even across a layer. You cannot stack oneshot **layers** —
there is no way to hold two `OSL(...)` layers queued at once.

Why: the custom oneshot module (`modules/shofel/oneshot`) only manages the
modifier entries in `oneshot_state_entries[]` (Ctrl/Alt/Gui/Shift). Layers ride
on QMK's native `OSL`, which is single-slot and has no stacking concept — and
stacking layers has no meaningful semantics anyway (a keypress resolves against
one layer, whereas chorded modifiers genuinely combine).

The module lists the `OSL(...)` keys in `is_oneshot_ignored_key`
(`layouts/split_3x6_3/shofel/keymap.c`) only so a *pending modifier* is
not consumed when you tap into an OSL layer — i.e. the modifier penetrates the
layer. That is about mods-through-layers, not layer stacking.

## A tapped oneshot modifier is held until the next key (no timeout)

Tapping a oneshot mod (`OS_CTL/ALT/GUI/SFT`) presses the modifier eagerly and
holds it until you press another key. There is no timeout: a stray tap with
nothing after it leaves the modifier physically held (harmless for Gui, but a
held Ctrl/Alt can catch the next mouse click). The next keypress always releases
it, so in normal typing this is invisible.

Why: the custom module (`modules/shofel/oneshot`) is a pure, event-driven FSM
(`oneshot_fsm.h`) with no timer or matrix-scan hook — that is what keeps it
unit-testable off-target. QMK's `ONESHOT_TIMEOUT` does not apply here (this is a
custom `register_code16` module, not QMK's built-in OSM). To release without
typing a key, press the same one-shot again — a second press drops the mod (see
below) — or call `oneshot_cancel()`.

## Double-tapping a one-shot sends the bare modifier (second press releases)

Pressing an already-armed one-shot a second time releases it instead of
re-arming. Because the mod is held eagerly from the first tap, releasing it with
no key in between is a bare modifier tap to the host: **double-tapping the Gui
combo (`A`+`'` or `I`+`Y`) sends a lone Gui press — e.g. it opens the launcher /
Start menu.** This is uniform across all four one-shots, so double-tapping `OS_ALT`
sends a bare Alt (focuses the menu bar on some apps), etc. — expected, not a bug.
A single tap still arms for the next key as before; only a *second* press of the
same trigger, while still armed, releases. Implemented purely in the FSM
(`oneshot_fsm.h`, `os_trigger_down`); no timer, so there is no double-tap window
to tune.

## A fast roll after a one-shot Shift can still shift the second key (`CA`)

Tap the one-shot Shift and then roll two letters fast — `Shift` then `c a` with
`a` going down before `c` comes up — and the host can print `CA` (both shifted)
instead of the intended `Ca` (only the first).

The pure FSM *does* drop the modifier on the second rolled key — this is the
"88 bug" case, and `tools/test_oneshot_fsm.c` locks it (the second key gets no
mod in the modelled event order). The gap is below the FSM: on hardware, under a
fast enough roll, the eager modifier's release does not reliably land before the
second key's HID report, so the host still sees Shift held when that key is sent.
This is a timing gap between the FSM and the wire, not a logic error — which is
why the unit test (idealised event order) stays green.

Left as-is: it only bites on a deliberately fast two-letter roll immediately
after a one-shot Shift, which is rare in normal typing. A real fix would mean
holding back the second key's report until the one-shot state settles — more
machinery than the occasional stray capital is worth. (Confirmed on real
hardware 2026-08-13; pre-existing — not introduced by the second-press change.)

## Combos that share a key can dump literal characters on a fast roll

Two combos that share a key — or a combo whose two keys each belong to a
*different* combo, "bridging" them — can misfire on a fast roll: instead of
firing, QMK's combo engine gives up on the ambiguous buffer and sends the keys
as literal taps. Concretely, when Esc briefly lived on `c+u`, it bridged the
Ctrl combo (`s+c`) and the Nav combo (`e+u`); chording Ctrl then Nav in quick
succession (`c-s-u-e`) made `{c,s,u}` ambiguous between `s+c` and `c+u`, so the
board typed a literal `cs`.

Why: with the default combo settings (no `COMBO_MUST_HOLD`, `COMBO_TERM` 50 ms),
a key held across two overlapping combos has no unambiguous resolution during a
roll, and the engine falls back to emitting the raw keys.

Mitigated, not eliminated: Esc moved to a **vertical same-column** combo (`q+b`)
that shares no key with any cross-finger combo — same-column pairs are safe
because a single finger's column is never rolled in normal typing (this is why
the default combo class here is vertical). A few horizontal combos still share a
key on purpose (the reset combos `,+c` / `f+l` share with the Ctrl combos), so an
unlucky roll through those could still dump; they sit on rare bigrams, so it is
left as-is. Rule of thumb when adding a combo: a key landing in 3+ combos, or a
combo bridging two independent ones, is a roll-dump risk. (Root-caused and fixed
2026-08-14, PR #26.)
