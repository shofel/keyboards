# TODO

## Triage method (reproducible — for humans and agents)

Each task scores **S = Impact×0.45 + Ease×0.30 + Leverage×0.15 + Clarity×0.10**, every factor an integer 1–5:

- **Impact** — daily friction removed if done (5 = bites every day; 1 = never noticed).
- **Ease** — cheapness, i.e. inverse effort (5 = minutes; 1 = weeks, hardware, or a rewrite).
- **Leverage** — how much it unblocks or simplifies other tasks (5 = foundational; 1 = self-contained dead-end).
- **Clarity** — how well-defined the work is (5 = exact fix known; 1 = vague idea needing its own design).

Procedure: score the four factors per task, take the weighted sum, round to 2 decimals, sort
descending; on ties the earlier-listed task stays first. Re-rank whenever a task is added or an
estimate changes. Bugs tend to top the list because Impact carries the most weight on a daily driver.

## Ranked — 2026-08-27

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.55 | Optimal Russian layout — `L_RUSSIAN` is stock ЙЦУКЕН, the one undesigned layer | Layout |
| 2 | 2.30 | Numbers and their punctuation — re-derive punctuation on a real corpus | Layout |

Scores are estimates; the Layout items entered on 2026-08-26 as `Russian I=3 E=2 L=2 C=3` and
`numbers I=2 E=3 L=2 C=2`. **Both measurement items shipped this cycle** (PRs #43, #44) and moved to
Done, so the two consumers are now the table.

**#1 is unblocked and #2 is not.** The Cyrillic corpus exists (531 prompts, 78,902 characters), which
is all the Russian layout needs. The number/punctuation redesign additionally needs *key-level* facts
— combo-misfire risk and pinned-hand reach — so it stays blocked until the keylogger has actually
been **flashed and run**: it compiles and is merged, but it has never recorded a keystroke. **One QA
gap carries forward:** the `windows` RU backend is UNVERIFIED — it needs a Windows host +
`EnableHexNumpad=1` (un-QA-able on NixOS); compose and vim are QA'd on-device.

## Layout

- **Optimal Russian layout.** `L_RUSSIAN` is stock **ЙЦУКЕН** — the only layer on this board nobody
  designed. The base is BOO (Dvorak-derived, rollover-tuned); Russian inherited GOST 6431-52 (1953),
  a standard whose actual constraint was keeping typewriter typebars from jamming. It shows:
  `какие` is `к`(index-top) `а`(index-home) `к`(index-top) `и`(inner-bottom) `е`(inner-top) —
  **five presses, all left index**, 4/4 same-finger bigrams on one of the commonest words in the
  language. `никаких` 4/6, `теперь` 3/5, `который` 3/6 are the same shape.

  Prior art is mature and the tooling is off-the-shelf: **oxeylyzer-2** (Rust, `.dof` layouts, ships
  a Russian corpus config and a Russian layout), **genkey**, **klavarog/OPT** (simulated annealing,
  Pareto multi-corpus). Published scores, genkey / oxeylyzer: **Вестник** 49.60 / −0.247,
  **Kharlamak** 52.00 / −0.492, Зубачёв 75.12 / −0.792, Диктор 95.82 / −1.676, **ЙЦУКЕН 268.36 /
  −7.044** — stock is ~5.4× worse than the best, a far bigger gap than QWERTY→Colemak for English.
  Those are one author's numbers, but an independent carpalx run agrees on direction (Диктор 2.550 <
  ЙЦУКЕН 3.000 < Rulemak 3.230 < Яверты 4.122). Вестник-vs-Kharlamak is ~5% apart — inside corpus
  noise, treat as tied. Note the two transfer layouts, Rulemak and Яверты, score *worse* than
  ЙЦУКЕН: mapping Cyrillic onto Latin positions optimises for the other alphabet's muscle memory,
  not for Russian.

  **Вестник is natively 3×10**, i.e. already split-shaped, and collapses the three rarest letters
  into derived forms (`щ→ш`, `ъ→ь`, `ё→е`) — a mechanism the compose backend already implements.
  Adopting that shape frees the outer pinky column now holding `ё х ъ э`.

  The historical reason nobody leaves ЙЦУКЕН — OS config, hardware, stickers — **does not apply
  here**: Russian is emitted from firmware (compose/vim/windows backends), not an OS layout, so this
  is a `keymap.c` edit plus `make gen-docs`. The only real cost is retraining, and it cannot
  interfere with BOO because it is a separate layer.

  **Unblocked** (the corpus shipped 2026-08-27, PR #43): `make corpus-ru` gives 531 prompts /
  78,902 Cyrillic characters. Do not adopt Вестник off the shelf — use it as the baseline to beat, running
  the optimiser against a *Russian* corpus of hand-typed text with the Cantor's own geometry and BOO
  pinned as constraints.

- **Numbers and their punctuation.** Reopens the punctuation half of **Findings — number entry**.
  The digit half is closed and should stay closed: adjacent digits carry only 0.235 bits of mutual
  information, so **no bigram-optimised digit layer, no binary chording, no steno home row** — those
  four designs were costed and rejected, and the conclusion is a property of numbers, not of the
  sample. What does *not* survive is everything about **punctuation**: `;` swung **70×** across
  corpora and the sample was repo code and markdown, i.e. LLM-written. The "L_NUM punctuation serves
  ~0.1% of keystrokes" figure inherits that flaw, so it cannot be used to justify leaving it alone
  either — the question is open in both directions.

  Re-derive `.` `-` `:` `/` `,` placement and the base-vs-`L_NUM` split from the real corpus. Score
  against **pinned-hand geometry, not the comfort map** in `keymap.c`: while `T+D` is held the
  reachable keys are `n` `l` `r`, then `f` `h`, which contradicts what the map predicts.

  **Still blocked** — unlike the Russian layout, this one is not satisfied by character
  frequencies alone: the rejected designs turned on combo-misfire risk and pinned-hand reach, which
  only the keylogger measures. The keylogger shipped (PR #44) but has **never been flashed**, so it
  has recorded nothing — flash it and collect a window before reopening this. Design thread lives in
  the `binary-digits` session.

## Findings — number entry (2026-08-25)

A design study of number/punctuation entry. **No keymap change was made** — four designs were
costed and all rejected. Recorded so they are not re-proposed, and so the measurements are not
re-derived.

### L_NUM is two layers, and the activation combo picks which

The combo that turns the layer on **pins the finger it is made of**:

- `rlt2 = {KC_T, KC_D}` is the *right middle* finger (home+top). While held, right mid-top/home/bottom
  (`↑` `⏎` `↓`) are unreachable → **digits mode**, left hand free.
- `llt2 = {KC_E, KC_U}` is the *left middle* finger. While held, `8` `5` `2` are unreachable →
  **nav mode**, right hand free.
- `leader,t` (sticky) pins nothing — the full layer is available.

**Measured by hand, the comfortable keys while `T+D` is held are `n` `l` `r`, then `f` `h`** — right
index-home, ring-top, ring-home, then index-top and index-bottom. Note this contradicts the static
comfort map in `keymap.c`, which scores `n`=9, `r`=7, `l`=6 and would predict `n` `r` `l`. With the
middle finger splayed up to reach `D`, the ring finger prefers the **top** row too. **The comfort
map does not model pinned-hand geometry** — treat its scores as base-layer only.

### Digits have no exploitable sequence structure

Mutual information between adjacent digits I(prev;next) = **0.235 bits**, vs **0.584** for letters in
the same corpus. Prose-only digits fall to 0.084 bits. Also: **71%** of numbers are a single digit,
**16%** of digit bigrams are same-digit repeats no layout can help, and the residual signal is
domain-unstable (prose 0.084 / shell 0.174 / code 0.684). **Do not build a bigram-optimised digit
layer.**

The same uniformity has a second consequence: **there is nowhere to hide a combo.** Every left
home-row digit pair is 1.2–2.5% of digit bigrams, and the *rarest digit pair anywhere* is 0.98%.
Letters offer dead zones (`qz`, `jx`); digits do not.

### Rejected designs

- **Binary chording** (4 home keys → 16 combos → 10 digits). Chording pays only when one chord emits
  *more than one* character; this is exactly 1.0, the worst possible ratio. Stenography's number bar
  wins because one chord emits a whole digit run.
- **Steno-linear home row** (`1 2 3 4 5 | 0 6 7 8 9`). Scores 6.14 vs the current grid's 6.03 on
  frequency-weighted comfort — ascending order puts `1` on the pinky and `0`/`5` on the comfort-3
  inner column. Permuting the *existing* grid by frequency would gain +19%, five times more than
  moving to the home row — but it discards transferable numpad muscle memory, which Armand, Redick &
  Poulsen (2014, *Applied Ergonomics* 45(4):917–22, n=57) found measurably improves accuracy. That
  study's one portable result: middle-row keys were faster and more accurate than top/bottom row.
- **Punctuation on L_NUM combos.** Vertical slots are gone — 9 of 10 left-hand same-column combos are
  already mods or brackets, and the 10th is unusable (base left pinky-bottom is `XX`). Horizontal
  adjacent-finger combos have no safe pair (above), and their failure mode *silently corrupts a
  number* — unlike prose, digits carry no redundancy to make the error visible. Only **same-finger
  horizontal** survives on mechanism (index column + inner column; `{KC_S, KC_G}` is unused) — but it
  would bridge `lctl`/`square_left` with `angle_left`/`dquo` through shared keys, the same pattern
  that dumped literal `cs` before (see `keymap.c` Esc note).
- **Better L_NUM punctuation slots.** Only **2–10%** of `.` and **~1%** of `-` `:` `/` are typed
  *inside* a number — the rest come from base. The entire L_NUM punctuation set therefore serves
  ~0.1% of keystrokes. Not worth the churn.

### Why nothing shipped: the corpus was wrong

Every frequency number above came from repo code + markdown + shell history. **The repos are largely
LLM-written**, so those character frequencies measure what Claude generates, not what fingers do.
Actual hand-typed input is almost entirely **Claude Code prompts** — natural-language English with
paths and identifiers, very little of the `; { } = (` that dominated the code sample.

So: **all punctuation-frequency conclusions here are suspect** (`;` already showed a 70× swing across
corpora — a clear artifact). The *digit-structure* conclusions survive, being properties of numbers
themselves rather than of the sample. The two measurement items that existed to fix this both
shipped on 2026-08-27 (PRs #43, #44) — see Done. The corpus confirmed the worry and then some: the
transcript premise was itself wrong by ~5×, and only 21.1% of raw user-message mass is hand-typed.

## Done

- **2026-08-27** — QMK keylogger (was ranked #2; PR #44): key-level instrument as
  `modules/shofel/keylog` — per-key presses, per-layer usage, an exact combo-firing total, and the
  per-key **type-then-correct rate** (presses undone by an immediate backspace), which no text
  corpus can see because the evidence is deleted before it reaches the text. Read out with
  `leader,s,d` / `leader,s,c` over the HID console; `s` is prefix-only so the leader set stays a
  prefix code (lint now reports 39 sequences, up from 37). **Counts, never sequences** — state is
  counters plus one slot of history, and a test asserts two different typing orders leave
  byte-identical state, so no password is recoverable. Two traps caught while wiring it: a combo
  emits its keycode from key position `(0,0)`, a *real* key here, which would have silently inflated
  key index 0 (filtered on `IS_KEYEVENT`); and combos are counted as an exact total rather than a
  per-combo table, because QMK's only fire-time hook is called per candidate key and would overcount
  — an exact total beats a plausible-looking wrong breakdown. **NOT hardware-QA'd**: it compiles and
  is merged but has never been flashed, so it has recorded nothing yet.
- **2026-08-27** — hand-typed character corpus (was ranked #1; PR #43): `tools/typing_corpus.py`
  over `~/.claude/projects/**/*.jsonl`. **The premise recorded here was wrong by ~5×.** `type=user`
  is a channel, not an author: 3353 of 4187 user records in one project are tool results in a
  user-shaped envelope; of the rest, 719 JSON-ish prompts carried 54.7% of the character mass and
  the top 8% of prompts carried 78%; the longest "prompts" are session-resume scaffolds and
  autonomous-loop classifiers, repeated byte-identically. Filtering keeps 9182 of 12369 prompts and
  **21.1% of the raw mass** — after which `"` falls out of the top 20 (it was 10th at 3.09%) and the
  top bigrams become `th on at in re he to an`. Positive control: the Cyrillic slice tracks published
  Russian prose (о 10.59 vs 10.98, а 8.38 vs 8.01, е 8.20 vs 8.45). Emits **counts, never
  sequences** — unigrams and bigrams only, enforced by a test — which is also the native input format
  for layout optimisers. `make corpus` / `make corpus-ru`.
- **2026-08-25** — case sound/feel (was the only open item, ranked #1): applied the **EVA + poron
  foam mod** to the Cantor. This was the last remaining hardware task; the ranked list is now
  measurement-only.
- **2026-08-17** — Russian backend selection (was #1 after the leader): pick the emission backend
  from the leader — `leader,r` (tap) → compose (default, rolling-safe, host-wide), `leader,(r+v)`
  (chord) → vim (`i_CTRL-V U`), `leader,(r+w)` (chord) → Windows (Alt+numpad hex; needs
  `EnableHexNumpad`). Shipped as PR #35 (combo-free `l,r,{c|v|w}` 3-key gateway) → PR #36 (bare
  single keys) → **PR #37, the final form**: compose is the bare tap, vim/Windows are the `r+v`/`r+w`
  chords captured by the armed leader and gated by `combo_should_trigger` so they never misfire in
  normal typing (`se**rv**e`, `cu**rv**e`). Adds a 3rd userspace backend, `RU_BACKEND_WINDOWS`, a
  faithful port of QMK's `UNICODE_MODE_WINDOWS`. **compose + vim QA'd on-device 2026-08-17; `windows`
  UNVERIFIED** — needs a Windows host + `EnableHexNumpad=1` (BMP-only, which covers Cyrillic and
  ₺/₽/€; astral emoji stay on compose).
- **2026-08-17** — `leader,k` now drops Russian (PR #37): opening kitty (`Gui+T`) first calls
  `toggle_disable()`, so terminal typing lands in Latin instead of Cyrillic — reversing the
  documented "keep the layer" decision (its section removed from `known-limitations.md`). Also wired
  `test_gen_layer_schemes.py` into `make test` (`test-schemes-unit`); it had silently rotted across
  PRs #32/#36.
- **2026-08-17** — reset-combo cleanup (PR #36): unmapped the `,+c` / `f+l` soft-reset chords and
  removed the orphaned `KK_RESET_STATE` keycode + its dead handlers. Soft-reset (drop toggle layers,
  cancel one-shots) is now solely `leader,space` (also `Esc` / `leader,e`); the `QK_BOOT` /
  `QK_REBOOT` thumb combos are untouched.
- **2026-08-17** — timeoutless leader (was ranked #1): the leader now fires the instant a sequence
  is uniquely matched, with no per-key timeout (QMK's stock leader is off, `LEADER_ENABLE=no`).
  Shipped in four PRs: prefix-collision lint as a blocking gate (`bcc5e9c`, PR #30); the
  fire-on-unique-match matcher + capture FSM, off-target unit-tested (`0979dc1`, PR #31);
  prefix-free set — currencies moved off the `M`/`.` mouse prefix to `l,u,{l|r|e}` (`e1dbaee`,
  PR #32); and the cutover — a custom capture in `process_record_user` driving the matcher, with
  the reset combo / `leader,space` as the abort escape hatch (`cc64160`, PR #33). A code review
  caught a stuck-key defect (releases were being swallowed during capture) that was fixed before
  merge. **Hardware QA passed 2026-08-17** — flashed to the left half: fires on unique match, no
  stuck keys on fast rolls, silent abort on non-match, `leader,space` reset all confirmed on-device.
- **2026-08-14** — symmetric combos reorg (was ranked #1): restored `=>`, mirrored `<=`/`<-`,
  relocated reset→`,+c`/`f+l`, rewrote every combo comment in the finger+row vocabulary, and added
  the "strange but consistent" symmetry note that `make gen-docs` renders into `docs/reference.md`.
  Shipped to `main` (`1cdf3cc`, PR #25). **Esc placement corrected in a follow-up:** the reorg first
  put Esc on `c+u`/`f+d`, but `c+u` bridged the Ctrl (`s+c`) and Nav (`e+u`) combos and dumped a
  literal `cs` on a fast roll — so Esc moved to the vertical same-column combo `q+b` (`9466211`,
  PR #26), and the roll-dump pitfall is documented in `docs/known-limitations.md` (`aae18a9`,
  PR #27). This subsumed the old "combos are positional" and "docs: symmetry" inbox items.
- **2026-08-14** — removed the bisect mouse mode (was ranked #3): the digitizer binary-search mode
  never drove a host cursor (libwacom won't bind `usb:feed:0000`). Orbital/polar is the sole mouse
  mode now; the "why it never worked" history is kept in `docs/known-limitations.md`. Shipped to
  `main` (`d010720`, PR #21).
- **2026-08-14** — README keymap-reference section (was ranked #5): a dedicated section linking to
  the generated `docs/reference.md`, with a clickable TOC into its sections. Shipped to `main`
  (`201eff2`, PR #22).
- **2026-08-14** — ingested the inbox into the timeoutless-leader item as requirement 4 (cancel /
  abort a half-typed sequence via a both-hands soft-reset combo, `x+w` / `h+k`, retiring the last
  `->` arrow). Shipped to `main` (`4679a29`, PR #20).
- **2026-08-13** — one-shot: a *second* press of an armed one-shot now releases it (the mod is
  held eagerly from the first tap, so the release is a bare modifier tap — double-tapping the Gui
  combo opens the launcher). Uniform across Ctrl/Alt/Gui/Sft. Shipped to `main` (`ff022e1`). Was
  the inbox item "osm module: second press means release".

## Removed as obsolete — 2026-08-13

- **Absolute-mouse HID descriptor** (was ranked #1) — moot once the bisect mouse mode is removed;
  the digitizer/host-binding history is preserved in `docs/known-limitations.md`.
- **Cantor/dactyl keymap transform program** (was ranked #2) — superseded: the dactyl overlay
  (`keyboards/handwired/dactyl_manuform/5x6_5/info.json`) already shares the single `split_3x6_3`
  keymap via `community_layouts`, so there is nothing to "transform". The remaining dactyl blocker
  (stock qmk's `find_info_json` ignores the userspace `keyboards/` dir, so dactyl is not a CI build
  target) is an upstream-QMK PR / standalone-keyboard-definition task — deferred, not a keymap
  change.

## References

https://github.com/possumvibes/keyboard-layout?tab=readme-ov-file#code-influences-alphabetically-and-non-comprehensively
  - callum
  - drashma
