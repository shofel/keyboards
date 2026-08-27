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

## Ranked — 2026-08-25

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.80 | Hand-typed character corpus from Claude Code transcripts | Tooling |
| 2 | 2.10 | QMK keylogger for key-level data (backspace, mods, layers, combos) | Tooling |

Scores are estimates. Both remaining items score low on Impact (they remove no friction by
themselves) and high on Leverage — they are measurement, and every layout decision below is blocked
on them. See **Findings — number entry** for why. The hardware item shipped this cycle (EVA + poron
foam mod). **One QA gap carries forward:** the `windows` RU backend is UNVERIFIED — it needs a
Windows host + `EnableHexNumpad=1` (un-QA-able on NixOS); compose and vim are QA'd on-device.

## Tooling

- **Hand-typed character corpus.** `~/.claude/projects/**/*.jsonl` holds ~3,347 Claude Code
  sessions; the user-message fields are hand-typed prompts, which is close to 100% of what actually
  gets typed on this board. Extract them into a corpus and re-run the frequency analysis. Cheap —
  the data already exists.
- **QMK keylogger.** A corpus can only ever count characters that survived. It is structurally
  blind to **backspace, modifiers, layer switches, combo usage, thumb load, and the
  type-then-correct rate** — several of which are plausibly among the most-pressed keys on the
  board. `CONSOLE_ENABLE` + `uprintf` in `process_record_user` + host-side aggregation. Needs a
  privacy design first: it will see passwords.

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
themselves rather than of the sample. Ranked items #1 and #2 exist to fix this.

## Done

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
