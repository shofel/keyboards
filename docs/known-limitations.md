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
uses, which every OS drives without configuration. See TODO.md.

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
(`layouts/shofel/split_3x6_3/shofel/keymap.c`) only so a *pending modifier* is
not consumed when you tap into an OSL layer — i.e. the modifier penetrates the
layer. That is about mods-through-layers, not layer stacking.
