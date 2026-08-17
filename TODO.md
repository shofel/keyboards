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

## Ranked — 2026-08-17

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.55 | Russian backend selection under one leader prefix — **needs a design decision** (the specced combo approach misfires in normal typing; see Layout) | Layout |
| 2 | 2.00 | Sturdier cantor case with quieter sound | Hardware |

Scores are estimates. The **timeoutless leader** (was #1) shipped and moved to **Done** (PRs
#30–#33): fire-on-unique-match, a prefix-free sequence set, a blocking prefix-collision lint, and
an off-target-tested matcher + capture FSM. Completing it changed the picture for #1 (RU backend):
the leader now handles 3-key sequences with no clock, so `l,r,{c|v|w}` is viable as a **combo-free**
alternative to the specced combos — which matters because the combos misfire (see Layout). #2 is
hardware.

## Layout

- **Russian backend selection under one leader prefix.** Goal: pick the Russian backend from a
  single `l,r`-family gateway — `compose` (default), `vim`, and `windows` (new). Today this is two
  separate keys: `l,r`/`l,c` → compose, `l,v` → vim.

  **The specced combo approach is blocked — it misfires in normal typing.** The plan was `l,r` tap
  vs `l,(r+c)` / `l,(r+v)` / `l,(r+w)` chords, so the tap and chords emit distinct keycodes and the
  set stays prefix-free. A 2026-08-14 hardware spike confirmed the *leader* captures a combo's
  output keycode (`l,(r+c)` → `SPK:RC`, `l,(r+v)` → `SPK:RV`, vs `l,r` → bare-`r`). **But** those
  combos are **global base-layer combos** (`COMBO_ONLY_FROM_LAYER 0`, no per-context restriction),
  so they also fire during ordinary typing: `r+c` eats "a**rc**", `r+v` eats "se**rv**e"/"cu**rv**e".
  That is the same roll-dump class as `docs/known-limitations.md`; the spike only tested deliberate
  chords, so it never surfaced it. Verified 2026-08-17 against the live combo config.

  **Recommended alternative — combo-free, enabled by the now-shipped timeoutless leader.** The
  leader now fires on unique match and handles 3-key sequences, so `l,r,c` / `l,r,v` / `l,r,w` are a
  clean prefix code **with no combos and no misfire** — *provided bare `l,r` is dropped* (require the
  3rd key; `r` becomes a pure prefix). Cost: retrains the current `l,r`/`l,v` muscle memory; a
  non-repeat default key is needed (the doc generator rejects a repeated-key sequence like `l,r,r`).

  **Decision needed before building** (pick one): (a) combo-free sequences `l,r,{c|v|w}`, dropping
  bare `l,r` — recommended; (b) keep combos but choose rare-bigram / vertical same-column 2nd-token
  pairs that won't misfire; (c) leave Russian selection as-is (`l,r`/`l,v`). Independently, the
  **`windows` backend is unbuilt** — it needs a Windows unicode input method (QMK facility vs.
  vendor into the `unicode_ru` module), a separate feature from the selection UX.

## Hardware

- A sturdier case with quieter sound.

## Done

- **2026-08-17** — timeoutless leader (was ranked #1): the leader now fires the instant a sequence
  is uniquely matched, with no per-key timeout (QMK's stock leader is off, `LEADER_ENABLE=no`).
  Shipped in four PRs: prefix-collision lint as a blocking gate (`bcc5e9c`, PR #30); the
  fire-on-unique-match matcher + capture FSM, off-target unit-tested (`0979dc1`, PR #31);
  prefix-free set — currencies moved off the `M`/`.` mouse prefix to `l,u,{l|r|e}` (`e1dbaee`,
  PR #32); and the cutover — a custom capture in `process_record_user` driving the matcher, with
  the reset combo / `leader,space` as the abort escape hatch (`cc64160`, PR #33). A code review
  caught a stuck-key defect (releases were being swallowed during capture) that was fixed before
  merge. **Hardware QA still pending** (batch) — the `process_record` integration can't be verified
  off-target.
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
