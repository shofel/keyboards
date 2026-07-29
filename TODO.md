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

## Ranked — 2026-07-29

| # | Score | Task | Where |
|---|-------|------|-------|
| 1 | 3.35 | Apply the libwacom hotswap (already written + build-verified in the dotfiles flake) and verify the cursor moves in bisect | Layout |
| 2 | 2.60 | Make ucis and unicodemap work together (→ then emoji via ucis) | Modules |
| 2 | 2.60 | Generate clean schemes from layer definitions (kill manual drift) | Dactyl |
| 4 | 2.55 | oneshot: press two oneshots at a time to schedule both | Modules |
| 5 | 2.30 | Shared layout for cantor & dactyl (keymap transform program + tests) | Dactyl |
| 6 | 2.25 | Cleanup readme (nix flake run, initial flash guide L/R) | Dactyl |
| 6 | 2.25 | Draw the layers diagram by hand | Ideas |
| 8 | 2.05 | Big dream: employ zig for modules / whole keymap | Big dream |
| 9 | 2.00 | Sturdier cantor case with quieter sound | Hardware |
| 10 | 1.55 | Make animations to explain tricks | Ideas |

## Layout

- Bisect needs a host-side libwacom entry for `usb:feed:0000`. The `.tablet`
  file is written and the NixOS hotswap is build-verified in the dotfiles flake
  (`nixos/modules/libwacom-cantor.nix`); what remains is `nixos-rebuild switch`,
  a log out/in (Wayland mutter won't reload libwacom mid-session), then checking
  whether the cursor actually moves. If Mutter still refuses, the fallback is a
  firmware absolute-mouse descriptor.
  See [docs/known-limitations.md](docs/known-limitations.md).

## Modules

- oneshot: allow press two oneshots at a time to schedule both

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
