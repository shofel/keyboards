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
| 1 | 2.00 | Sturdier cantor case with quieter sound | Hardware |

Scores are estimates. Both software items shipped to **Done** this cycle: the **timeoutless
leader** (was #1; PRs #30–#33) and **Russian backend selection** (was #1 after the leader; PR #35,
simplified in #36). The only open item is hardware. **One QA gap carries forward:** the `windows`
RU backend is UNVERIFIED — it needs a Windows host + `EnableHexNumpad=1` (un-QA-able on NixOS);
compose and vim are QA'd on-device.

## Hardware

- A sturdier case with quieter sound.

## Done

- **2026-08-17** — Russian backend selection (was #1 after the leader): pick the emission backend
  from the leader, one bare key each — `leader,r` → compose (default, rolling-safe, host-wide),
  `leader,v` → vim (`i_CTRL-V U`), `leader,w` → Windows (Alt+numpad hex; needs `EnableHexNumpad`).
  Shipped as PR #35 — first under a combo-free `l,r,{c|v|w}` 3-key gateway (the specced `r+c`/`r+v`/
  `r+w` combos would misfire in normal typing: `se**rv**e`, `cu**rv**e`, `a**rc**`), then simplified
  to bare single keys in PR #36 so `leader,r` is the fast default again. Adds a 3rd userspace
  backend, `RU_BACKEND_WINDOWS`, a faithful port of QMK's `UNICODE_MODE_WINDOWS`. **compose + vim
  QA'd on-device 2026-08-17; `windows` UNVERIFIED** — needs a Windows host + `EnableHexNumpad=1`
  (BMP-only, which covers Cyrillic and ₺/₽/€; astral emoji stay on compose).
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
