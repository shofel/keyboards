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

## Ranked — 2026-08-07

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.55 | Absolute-mouse HID descriptor so bisect needs no host config | Layout |
| 2 | 2.30 | Shared layout for cantor & dactyl (keymap transform program + tests) | Dactyl |
| 3 | 2.00 | Sturdier cantor case with quieter sound | Hardware |

## Layout

- Bisect needs the host to bind the digitizer. The proposed fix — stop
  presenting as a digitizer and emit an absolute-mouse HID descriptor (no host
  config on any OS) — turns out to be **core-only**: that descriptor lives in
  QMK's `usb_descriptor.c` (non-weak `const`, no userspace override, no
  absolute-mouse build knob), so it's an upstream-QMK-PR / local-fork task, not a
  keymap change. Cheaper first step: re-test a *correct* libwacom `.tablet` entry
  (the earlier attempt may have been an ID/descriptor mismatch, not a dead lane).
  See [docs/known-limitations.md](docs/known-limitations.md).

## Dactyl

- get the dactyl building on stock qmk. It's not in `qmk.json` build targets
  (so not in CI) because the shared keymap reaches it by adding the
  `split_3x6_3` community layout to the *mainline* `handwired/dactyl_manuform/5x6_5`
  via a userspace `keyboards/.../info.json` overlay — and stock qmk's
  `find_info_json` never reads the userspace keyboards dir (that was a fork-only
  patch, now dropped). Cantor is unaffected: its mainline `keyboard.json`
  already declares `split_3x6_3`. Paths forward: (a) upstream PR adding the
  `split_3x6_3` community layout to mainline dactyl, or (b) define the dactyl as
  a standalone userspace keyboard that declares the layout itself.
- make a shared layout for cantor and dactyl
  - transform dactyl keymap to a cantor's
    - make a text transform program
    - which removes keys not presented on cantor
      - from the comment
      - from the code
    - which is covered by tests

## TODO cantor hardware

- a sturdier case with quieter sound

## References

https://github.com/possumvibes/keyboard-layout?tab=readme-ov-file#code-influences-alphabetically-and-non-comprehensively
  - callum
  - drashma
