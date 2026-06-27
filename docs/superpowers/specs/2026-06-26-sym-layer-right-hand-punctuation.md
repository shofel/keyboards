# SYM layer — right-hand punctuation redesign

Status: **design complete, not yet implemented.** Re-places the symbols *within*
the existing SYM layer. No new keycodes, combos, overrides, or unicode entries.

## Goal

Move the bulk of SYM punctuation onto the **right hand**, reversing the current
left-hand placement.

Rationale (the user's): Space is the **right middle thumb**, so a symbol typed on
the right hand rolls into Space on the *same hand*. The cleanest sequence is:

```
left thumb (tap one-shot SYM, KK_SYMBO)  →  right-hand symbol  →  right thumb Space
```

Three distinct actuators, and the symbol→Space part is a same-hand right-side roll.
This supersedes the old rationale ("punctuation on the LEFT, opposite Space,
alternates and rolls cleanly" — keymap.c) which optimized for cross-hand
alternation instead.

## Final SYM layer

```
         pinky2 pinky ring  mid  index inner | inner index  mid  ring  pinky pinky2
 top       ·     `    —     $     ^     ·   |   ·     !     *     |     &     /
 home      ·          ~     @     #     ·   |   ·     ?     :     +     %     -
 bot       ·     ·    №     §     \     ⌦   |   ⌫     =     ;     ·     ·     ·
```

Right hand (the punctuation home): `! ? = * : ; + % | &`
Left hand (token-hugging specials): `` ` `` `— $ ^ ~ @ # № § \`
Pinned (base positions, identical on every layer): `/` top-R-outer, `-` home-R-outer.
Utility (unchanged): `⌫` `KC_BSPC` R-inner-bot, `⌦` `KC_DEL` L-inner-bot.

### Keycodes by position (`LAYOUT_split_3x6_3`, cols 0–11, `XX` = `KC_NO`)

```
top:  XX  KC_GRV  RU_MDASH  KC_DLR   KC_CIRC  XX | XX       KC_EXLM  KC_ASTR  KC_PIPE  KC_AMPR  KC_SLASH
home: XX  XX      KC_TILD   KC_AT    KC_HASH  XX | XX       KC_QUES  KC_COLN  KC_PLUS  KC_PERC  KC_MINUS
bot:  XX  XX      RU_NUM    RU_SECT  KC_BSLS  KC_DEL | KC_BSPC  KC_EQL   KC_SCLN  XX       XX       XX
```

Thumb row unchanged: `__ __ KK_SYMBO | __ __ __`.

20 symbols placed, none dropped. `RU_MDASH` / `RU_NUM` / `RU_SECT` already exist in
the `unicode_ru` module.

## Mnemonics baked in (do not "fix" these)

- **`? !` on the two strongest right keys** (index home `?`, index top `!`) — the
  sentence-enders are the symbols most reliably followed by Space, so they earn the
  keys that roll best into the right-thumb Space.
- **`=` below them** (index-bot). `:`(mid-home)→`=`(index-bot) is still a real
  cross-row roll, different fingers — so no `:=` combo.
- **`: ;` stacked** on the mid column (home `:` / bot `;`).
- **`| &` paired** on the top row (ring `|` / pinky `&`); `|` on the stronger ring
  key (pipe-heavy CLI use).
- **`/` `-` pinned** to the right-outer base positions — same key on every layer,
  keeps `_` = `Shift+-`.
- **backtick on left-pinky-top** = the same position as base `'` (and base
  `Shift+'` → `` ` ``). One consistent home for the quote/backtick key across base
  and SYM. Left-pinky-home is intentionally empty (all 20 symbols are placed).
- `⌫` backspace (R-inner-bot) and `⌦` delete (L-inner-bot) unchanged.

## Not on SYM (unchanged — all global, work while Russian is active)

- `"` — the G+V combo (`KC_DQUO`).
- `« »` / `< >` — the angle combos (`KK_LANGLE` / `KK_RANGLE`).
- `?` is suppressed from base `Shift+/`, so SYM is its one canonical home.

## Scope of change (small)

Pure re-placement inside the `[L_SYMBOLS]` block. Files/edits:

1. `layouts/shofel/split_3x6_3/shofel/keymap.c`
   - `[L_SYMBOLS]` keycode array → the table above.
   - Its doc-comment ASCII art + mnemonics → the grid + notes above (the current
     comment becomes stale).
   - The main design comment's "Punctuation lives mostly on the LEFT hand …"
     paragraph → now the RIGHT hand, with the new same-hand-Space rationale.

**No changes** to combos, key overrides, the `unicode_ru` module, or any other
layer. Every keycode referenced above already exists in the current keymap.

## Accepted trade-offs (don't re-litigate)

- **`=` on index-bot, not a home key.** `=` is a very common code symbol, but the
  prose `?`/`!` → Space rolls are prioritized over the code `=` roll. Deliberate.
- **No `:=` combo** — `:`→`=` is an acceptable cross-row roll.
- **Left index/mid home (comfort 9) hold `#` / `@`**, not top-frequency symbols —
  the cost of crowding punctuation onto the right for the Space-roll.
- **backtick on a weaker key** (pinky-top, comfort 4) — accepted for positional
  consistency with base `'`; backtick is also reachable as `Shift+'`.

## Reference

Comfort-score map (committed, kept as a comment above `keymaps[]`); symbol→key
assignment was the user's, ranked against it. Builds on
`2026-06-17-symbols-layer-design.md` (the frequency-first SYM redesign this adjusts).
