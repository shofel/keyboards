# TODO

## Triage — ranked (2026-07-17)

Ranked by a weighted composite score: **Impact×0.45 + Ease×0.30 + Leverage×0.15 + Clarity×0.10** (each factor 1–5). Impact = daily friction removed; Ease = cheapness to do; Leverage = unblocks other work; Clarity = how well-defined the task is. Reweight and the order moves — bugs float up because this is a daily driver.

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 3.75 | RU layer sometimes fails to activate (leader,r stays English until leader,e→leader,r) | Layout / BUGS |
| 2 | 3.40 | 88 bug: fast seq `[os_sft c a]` resolves to `[C A]` instead of `[C a]` | Modules |
| 3 | 3.30 | Mouse bisect broken — nothing happens; debug from the ground up | Layout / BUGS |
| 4 | 3.05 | Why is `uv.lock` in the repo? What for? | top |
| 5 | 3.00 | Per-layer: sync comment docs ↔ code (cheap haiku? per-PR follow-up?) | top |
| 6 | 2.80 | Remove visual rendering of the keymap (unused) | top |
| 7 | 2.65 | Document the oneshot-not-stacked pitfall + explain why | Modules |
| 8 | 2.60 | Make ucis and unicodemap work together (→ then emoji via ucis) | Modules |
| 8 | 2.60 | Generate clean schemes from layer definitions (kill manual drift) | Dactyl |
| 10 | 2.55 | oneshot: press two oneshots at a time to schedule both | Modules |
| 11 | 2.50 | Move "Known / accepted" (kitty-ctrl-on-Ru) into docs | Layout |
| 12 | 2.30 | Shared layout for cantor & dactyl (keymap transform program + tests) | Dactyl |
| 13 | 2.25 | Cleanup readme (nix flake run, initial flash guide L/R) | Dactyl |
| 13 | 2.25 | Draw the layers diagram by hand | Ideas |
| 15 | 2.05 | Big dream: employ zig for modules / whole keymap | Big dream |
| 16 | 2.00 | Sturdier cantor case with quieter sound | Hardware |
| 17 | 1.55 | Make animations to explain tricks | Ideas |

TODO: for each layer: sync comment docs and code
      - can we do it with a cheap and fast haiku?
      - shall we do it as a follow-up for each PR/feature?

TODO: remove visual rendering of the keymap. I don't use it
TODO: why uv.lock is in the repo? What for?

## Layout

BUGS to fix
- mouse bisect is broken — nothing happens. Let's debug it from the ground-up
- sometimes ru is failing to activate. I press leader,r many times in a row, and it's still english. Then i do leader,e -> leader,r -> ru is activated


### Known / accepted
TODO move to a docs

- kitty control doesn't work while Ru active: leader,k sends win+T fine, but the
  follow-up control keys are typed on the Russian layer (come out Cyrillic).
  Not a leader bug (leader,k itself records correctly). Left as-is — fixing would
  mean leader,k also dropping the toggle layer first.

## Modules

- oneshot: allow press two oneshots at a time to schedule both
TODO document a pitfall: the oneshot layer is not being stacked + explain why

- 88 bug: fast seq [os_sft c a] resolves to [C A], while [C a] expected

- make ucis and unicodemap work together
  - then: employ ucis for emoji: tulip and other flowers, tup=thumbup, ok, think, monocle

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
  - guide for initial flash for left and right
- generate clean schemes from layer definitions.
  - Now they are good, but manual and prone to be outdated.

## TODO cantor hardware

- a sturdier case with quieter sound

## Big dream: employ zig

- implement modules for keymap in zig
- make the whole keymap in zig
- from QMK use only low-level (keyboard support and matrix poll)
  - the rest features and a keymap implement in zig
- ? replace gcc with zig.build

## Ideas

- Draw the layers diagram by hand
- Make animations to explain tricks

## References

https://github.com/possumvibes/keyboard-layout?tab=readme-ov-file#code-influences-alphabetically-and-non-comprehensively
  - callum
  - drashma
