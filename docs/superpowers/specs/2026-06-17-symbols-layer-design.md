# Symbols layer redesign — design handout

Status: **design complete, not yet implemented.** This document is a handout for
a fresh session to implement the agreed design. Everything below is decided
unless a line says "open".

## Goal

Three original asks:

1. The dedicated `_` on the BOO base layer (left-pinky-bottom) is unused — remove it.
2. Assess the punctuation living in the NUM layer.
3. Make punctuation easy to reach on the SYM layer, specifically for typing Russian.

This grew into a full **frequency-first redesign of the SYM layer**, plus the
supporting changes that fall out of it.

## Files in play

- `layouts/shofel/split_3x6_3/shofel/keymap.c` — layers, combos, overrides, `process_record_user`.
- `modules/shofel/unicode_ru/introspection.h` — unicode name enum + `RU_*` macros.
- `modules/shofel/unicode_ru/introspection.c` — `unicode_map[]` codepoints.

## Architecture you must know first

- **SYM** is `OSL(L_SYMBOLS)` (left thumb). One-shot: **tap** = next key comes from
  SYM then reverts; **hold** = SYM stays active while held.
- Layer order (low→high priority): BOO(0), RUSSIAN(1), SYMBOLS(2), NUM_NAV(3),
  FKEYS(4), MOUSE(5). SYM overlays Russian, so SYM keys win while Russian is on.
- **Combos use `COMBO_ONLY_FROM_LAYER 0`** → they resolve keycodes from the base
  layer **by position**, so every combo fires on every layer, **including Russian**
  (where those physical keys otherwise emit Cyrillic). This is what makes the
  guillemet and `"` combos work while typing Russian.
- In **Russian**, base-layer latin keys are shadowed by Cyrillic. So latin
  punctuation must come from **SYM** or from a **global combo**.
- Unicode via the `unicode_ru` module (QMK unicodemap, `UP()` pairs). `register_unicode(cp)`
  sends a codepoint using the current input mode (Linux/Vim), set via leader seqs.
- One-shot mods are in use (`OS_SFT` etc.) — relevant to the guillemet shift test.

## Organizing principle

**Frequency-first (option B), ranked by SYM-context frequency** — i.e. how often a
key is pressed *while SYM is active*, not overall symbol frequency. Consequences:

- Base-shared symbols (`-`, `/`) are **demoted**: in English you type them from the
  base layer, so SYM only needs them for Russian. They are **pinned to their base-layer
  positions** (right-outer column) so the position never changes between layers, which
  also keeps `_` = `Shift+-` consistent.
- **Shiftless** philosophy kept: every symbol on SYM has its own key, no Shift needed.
  There is enough space (20 symbols + 2 utility keys into ~30 usable slots).

### Key comfort score map (already committed: `fa3c92e`)

Layout-wide 0–9 weights (higher = easier), kept as a comment above `keymaps[]`.
User-tuned values:

```
        pinky2 pinky ring  mid  index inner | inner index  mid  ring  pinky pinky2
 top      2     4     6     8     6     2   |   2     6     8     6     4     2
 home     4     5     7     9     9     3   |   3     9     9     7     5     3
 bot      1     2     4     5     6     3   |   3     6     5     4     2     1
```

## Final SYM layer

```
         pinky2 pinky ring  mid  index inner | inner index  mid  ring  pinky pinky2
 top       ·     ·    #     —     |     ·   |   ·     &     ?     ^     ·     /
 home      ·     @    ;     *     :     ·   |   ·     =     !     +     %     -
 bot       ·     ·    ·     $     ~     ·   |   ⌫     \     №     ·     ⌦     ·
```

Keycodes by position (`LAYOUT_split_3x6_3`, cols 0–11, `XX` = `KC_NO`):

```
top:  XX  XX        KC_HASH  RU_MDASH KC_PIPE  XX | XX  KC_AMPR KC_QUES KC_CIRC XX      KC_SLASH
home: XX  KC_AT     KC_SCLN  KC_ASTR  KC_COLN  XX | XX  KC_EQL  KC_EXLM KC_PLUS KC_PERC KC_MINUS
bot:  XX  XX        XX       KC_DLR   KC_TILD  XX | KC_BSPC KC_BSLS RU_NUM XX     KC_DEL  XX
thumb:            __  __  KK_SYMBO              |  __  __  __        (unchanged)
```

Notes:
- 20 symbols on SYM. `"`, `« »`, `` ` `` are **not** on SYM (see below).
- **Pinned:** `/` top-R-outer, `-` home-R-outer (base positions); `⌫` `KC_BSPC` at
  R-inner-bottom; `⌦` `KC_DEL` at R-bottom-pinky (moved off the inner column per request).
- `—` and `№` are unicode keys (`RU_MDASH`, `RU_NUM`) — need unicodemap entries.

### Mnemonics baked in (do not "fix" these)
- `! ?` vertical stack on right-middle (home `!` / top `?`) — sentence-enders.
- `& |` mirrored on the index-tops — logical pair.
- `:` (left-index home) → `=` (right-index home) — the `:=` alternating-hand roll
  (this is *why* the `:=` combo is removed).
- `-` `/` at base positions — same key on every layer.

## Quotes, backtick, question mark

- `'` — base tap (top-left-pinky), unchanged.
- `` ` `` (backtick) — base **`Shift+'`** via a new key override (`KC_QUOT` shifted → `KC_GRV`).
  This removes `"` from the base layer.
- `"` — the **G+V combo** (reusing the freed backspace-combo slot), emits `KC_DQUO`.
  Global on every layer. Rationale: `"` is frequent + would otherwise be SYM-only on
  a weak key; a vertical combo gives one-chord global access and joins the
  bracket/delimiter combo family. (User accepted that `"` is no longer a simple key.)
- `?` — add a key override to **suppress `Shift+/`** (emit plain `/`). `?` then lives
  only on SYM (top-R-middle). One canonical home.
- `_` — `Shift+-`, no dedicated key.

## Guillemets « »

Shifted angle combos. Convert the existing angle combos to custom keycodes:

- `angle_left_combo` ({KC_G,KC_Z}) → `KK_LANGLE`; `angle_right_combo` ({KC_B,KC_P}) → `KK_RANGLE`.
- In `process_record_user`, on press:
  - unshifted → `tap_code16(KC_LABK)` / `tap_code16(KC_RABK)` (i.e. `<` / `>`).
  - shifted → strip shift, `register_unicode(0x00AB)` / `register_unicode(0x00BB)`, restore mods.
- **Shift test must be `get_mods() | get_oneshot_mods()`** — one-shot shift is in use,
  and `get_mods()` alone misses it. This is the easy-to-miss bug; cover it.
- Guillemets via `register_unicode` need **no** unicodemap entry (it takes a codepoint).

Why a key override can't do this: the combo emits `KC_LABK`, which already carries an
intrinsic Shift, so an extra held Shift collapses into the same mod state — an override
keyed on `Shift+KC_LABK` can't distinguish `<` from shifted-`<`. The custom keycode reads
the held mods directly, so it can.

## Combos: remove / keep

Remove (with their keycodes/cases):
- `:=` — `go_declaration_combo` ({KC_H,KC_I}) and `KK_GO_DECLARATION` keycode + its
  `process_record_user` case. Now an easy `:`→`=` roll.
- `!=` — `not_equal_combo` ({KC_H,KC_X}) and `KK_NOT_EQUAL` keycode + its case.
- `G+V` backspace — `lbspc_combo` ({KC_G,KC_V}); slot repurposed for `"`.

Keep:
- `=>` (`fat_right_arrow_combo`, {KC_H,KC_M}, `KK_FAT_RIGHT_ARROW`) and
  `->` (`right_arrow_combo`, {KC_H,KC_K}, `KK_RIGHT_ARROW`) — they still span the
  combo-only `>`.
- square/brace combos, mod combos, esc/boot/reset, fkeys.

Add:
- G+V → `KC_DQUO` (the `"` combo).
- `KK_LANGLE` / `KK_RANGLE` custom keycodes + the angle combos pointing at them.

Remember: combos are added in 3 places (array, `enum combos`, `key_combos[]`).

## Key overrides: add

- `ko_make_basic(MOD_MASK_SHIFT, KC_SLSH, KC_SLSH)` — suppress `?` (Shift+/ → /).
- `ko_make_basic(MOD_MASK_SHIFT, KC_QUOT, KC_GRV)` — Shift+' → `` ` ``.
- Keep `comma_override`, `dot_override`. These overrides act on base keycodes
  (`KC_SLSH`, `KC_QUOT`); they do not touch SYM's `KC_QUES` or the `"` combo.

## Unicode additions (`unicode_ru`)

- `introspection.h`: add `U_MDASH`, `U_NUMERO` to the enum; add macros
  `#define RU_MDASH UP(U_MDASH, U_MDASH)` and `#define RU_NUM UP(U_NUMERO, U_NUMERO)`.
- `introspection.c`: `[U_MDASH] = 0x2014,` and `[U_NUMERO] = 0x2116,`.
- Guillemets need nothing here (sent via `register_unicode`).

## BOO base and NUM

- **BOO:** left-pinky-bottom `KC_UNDS` → `XX`. (`_` is `Shift+-`.)
- **NUM:** no change. `' , / : .` stay (justified: dates/times/decimals/thousands).

## Also

- Rewrite the SYM layer's doc comment: the current ASCII art + frequency buckets are
  **stale** (don't match the live keycodes). Replace with the new grid + rationale.

## Current repo state

- Worktree: `.claude/worktrees/syms`, branch `worktree-syms`, based on `main` (`ec6967b`).
- Committed: `fa3c92e` — the key comfort score map only.
- **Not yet implemented:** everything else in this document.

## Accepted trade-offs (don't re-litigate)

- `"` is no longer a simple key (combo only) — accepted for global one-chord access.
- `?` and `"` are SYM/combo-only (suppressed from base Shift) — chosen for "one
  canonical home, consistent across languages".
- `~/` is now a cross-hand alternation (since `/` moved to the right-outer base
  position) rather than a same-hand roll — accepted.
- Shift-pairs are scattered across the layer — inherent to frequency-first.
- Mnemonics beyond the four listed were considered and declined ("good enough").
