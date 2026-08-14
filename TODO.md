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

## Ranked — 2026-08-14

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.70 | Timeoutless leader: fire-on-unique-match, prefix-free sequence set + collision lint | Leader |
| 2 | 2.55 | Russian backend selection under one leader prefix, via **combos** on the 2nd token (`l,r` tap vs `l,(r+c/v/w)` chord) | Layout |
| 3 | 2.00 | Sturdier cantor case with quieter sound | Hardware |

Scores are estimates. The symmetric-combos reorg that used to top this list shipped and moved to
**Done** (PR #25/#26); it already carried the "combos are positional" finger+row comments and the
"docs: symmetry" note as part of the work. #1 (timeoutless leader) and #2 (RU backend) couple: a
bare `r` tap and an `r+c` chord emit distinct keycodes, so `l,r` and `l,(r+c)` are not a prefix
pair — the RU set stays prefix-free without dropping the standalone `l,r`, which is what keeps it
compatible with a timeoutless leader. #3 is hardware.

## Leader

- **Timeoutless leader.** Today the leader still terminates each sequence on a
  100 ms per-key timeout (`LEADER_TIMEOUT 100` + `LEADER_PER_KEY_TIMING`;
  `LEADER_NO_TIMEOUT` only removes the *initial* wait). Make it fire the instant a
  sequence is uniquely matched — consistent with the no-timeout one-shot design,
  snappier, and with no "too slow" misfire. Requires, in order:
  1. **Prefix-free sequence set.** A timeoutless leader only works if no sequence
     is a prefix of another. Current blockers: `M` (mouse layer) is a prefix of
     `M,L`/`M,R`/`M,E` (₺/₽/€), and `.` mirrors it. Relocate the mouse layer off
     bare `M`/`.` (or move the currencies) so the set is a prefix code.
  2. **Fire-on-unique-match.** Replace timeout-termination with a small custom
     matcher (same shape as the one-shot module) that tests the growing buffer
     against the sequence table and fires the moment a unique complete match is
     found; abort immediately on a non-matching key. Off-target testable.
  3. **Prefix-collision lint.** A generator/CI check that fails if any two
     sequences ever form a prefix pair — guards the invariant forever.
  4. **Cancel / abort a half-typed sequence.** A timeoutless leader has no clock
     to bail you out, so it needs an explicit escape hatch. The **reset combo**
     (`,+c` / `f+l`, top row) already ships from the symmetric-combos reorg and
     fires `KK_RESET_STATE` → `toggle_reset()` (disable toggle layers, cancel
     one-shots), with the reset logic already extracted into a reusable function.
     **Remaining:** extend that reset to also clear in-flight leader-sequence
     state once the fire-on-unique-match matcher (req 2) holds that state.

  Couples with the "Russian backend under one leader prefix" item: `l,r` plus
  `l,r,c`/`l,r,w`/`l,r,v` is *not* prefix-free, so the two are incompatible unless
  the bare `l,r` standalone is dropped (always require the 3rd key).

## Layout

- **Russian backend under one leader prefix — via combos.** The 2nd token after `l` is either a
  *tap* of `r` or a *chord*:
  - `l , r`      → Ru (default / compose)
  - `l , (r+c)`  → Ru compose
  - `l , (r+v)`  → Ru vim
  - `l , (r+w)`  → Ru windows   `()` = a combo

  A bare `r` tap and the `r+c` combo emit **distinct keycodes**, so `l,r` and `l,(r+c)` are not a
  prefix pair — the set is prefix-free **without** dropping the standalone `l,r`. This is why it no
  longer conflicts with the timeoutless leader. **Gate before building:** a hardware spike on the
  combo×leader interaction — stock QMK processes combos before the leader, so the combo's *output*
  keycode should reach the leader, but that can't be proven off-target. Confirm `l,(r+c)` captures
  the combo keycode (not a bare `r`) and `l,r` alone still fires Ru. (`windows` backend still open:
  QMK facility vs. vendor into our module.)

## Hardware

- A sturdier case with quieter sound.

## Done

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
