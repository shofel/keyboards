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

## Ranked — 2026-08-13

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.55 | Combos are positional: strip key-label names from combo comments + enforce in the generator | Docs |
| 2 | 2.50 | Remove bisect mouse mode; document its history (existed, never worked) in known-limitations | Layout |
| 3 | 2.45 | Russian backend selection under a single leader prefix (`l,r` / `l,r,c` / `l,r,w` / `l,r,v`) | Layout |
| 4 | 2.40 | README: dedicated keymap-reference section (header links to the file, body a clickable TOC) | Docs |
| 5 | 2.00 | Sturdier cantor case with quieter sound | Hardware |

Scores are estimates applying the rubric above — re-weight freely; the cluster (2.40–2.55) means
these are close in priority.

## Docs

- **Combos are positional — enforce it.** Combo comments must not name key labels; every combo
  is defined by *position*, not the legend on the key. Make this a strict rule both in the source
  (rename any label-naming combo comments) and in the generator (`tools/gen_layer_schemes.py` — a
  check that fails on a label-named combo). Keeps the comment from drifting from the positional
  reality.
- **README keymap-reference section.** Add a dedicated section: a header that links to the full
  generated reference (`docs/reference.md`), and a body that is a clickable table of contents into
  it.

## Layout

- **Remove bisect mouse mode.** The digitizer binary-search mode never drove a cursor on the host
  (libwacom won't bind `usb:feed:0000`; the absolute-mouse descriptor is QMK-core-only). Remove
  `L_MOUSE_BISECT` and its glue (`bisect_geom.h`, the digitizer arming in `layer_state_set_user`,
  the mode-switch key, `tools/test_bisect_geom.c` + its Makefile target), leaving Polar/Orbital as
  the sole mouse mode. Document the history — that it existed and why it never worked — in
  `docs/known-limitations.md` (much of it is already there under the bisect section).
- **Russian backend under one leader prefix.** Collapse backend selection into a nested leader
  prefix instead of separate sequences:
  - `l,r`   → compose (default)
  - `l,r,c` → compose
  - `l,r,w` → windows  (open question: use QMK's facility, or vendor its piece into our module?)
  - `l,r,v` → vim

  Needs a small design pass on nested-prefix leader handling.

## Hardware

- A sturdier case with quieter sound.

## Done

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
