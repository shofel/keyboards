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

## Ranked — 2026-08-04

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 2.55 | Absolute-mouse HID descriptor so bisect needs no host config | Layout |
| 2 | 2.50 | Document leader sequences + emoji in one synced reference | Docs |
| 3 | 2.30 | Shared layout for cantor & dactyl (keymap transform program + tests) | Dactyl |
| 4 | 2.25 | Cleanup readme (nix flake run) | Dactyl |
| 5 | 2.25 | Draw the layers diagram by hand | Ideas |
| 6 | 2.00 | Sturdier cantor case with quieter sound | Hardware |
| 7 | 1.55 | Make animations to explain tricks | Ideas |

## Layout

- Bisect needs the host to bind the digitizer. libwacom is exhausted — a
  matching `.tablet` entry did not get the cursor moving, and that lane is
  closed. The remaining option is to stop presenting as a digitizer and emit an
  absolute-mouse HID descriptor instead, which needs no host config on any OS.
  See [docs/known-limitations.md](docs/known-limitations.md).

## Docs

- Document all leader sequences + emoji in one synced reference. Today they're
  scattered — README covers Russian + emoji, but the rest lives only in
  keymap.c's `leader_end_user`. Cover the mode toggles and the mirror pairs
  (s·n = Esc, w·h = Ctrl+Esc, a·i = emoji, m·. = mouse/currency), the text-edit
  seqs, and the emoji `@`-codes — and keep it in sync with keymap.c (extend the
  "resync the layer comment in the same PR" convention to this reference).

## Dactyl

- make a shared layout for cantor and dactyl
  - transform dactyl keymap to a cantor's
    - make a text transform program
    - which removes keys not presented on cantor
      - from the comment
      - from the code
    - which is covered by tests
- cleanup readme
  - nix flake run

## TODO cantor hardware

- a sturdier case with quieter sound

## Ideas

- Draw the layers diagram by hand
- Make animations to explain tricks

## References

https://github.com/possumvibes/keyboard-layout?tab=readme-ov-file#code-influences-alphabetically-and-non-comprehensively
  - callum
  - drashma
