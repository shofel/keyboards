# Cantor / shofel keymap — working notes for agents & contributors

## Flashing

- **Flash the LEFT half only.** The left half is the one we keep on USB, and the
  only half we build and flash firmware for; the right half is not flashed in
  normal work. After a keymap change, flash the left half once (`make flash`) —
  do not flash the right half unless explicitly asked.

## Build & test

- `make build` — `qmk compile` for `cantor:shofel`. Needs the devenv toolchain;
  from a plain shell run it via `direnv exec . make build`.
- `make test` — off-target host tests (oneshot, compose, scheme-drift).
- After changing the keymap or its combos, run `make gen-docs` to regenerate
  `docs/reference.md` and the in-`LAYOUT` scheme comments — otherwise `make test`
  (its `test-schemes` drift check) fails.

## Documentation — link, don't copy

Every fact has one home; everything else links to it. This is load-bearing, not
tidiness: the balanced-layout metrics and mnemonics once lived in four
hand-maintained copies (README, `keymap.c`, `docs/ru-balanced-layout.{md,html}`),
and a `leader,v` drifted out of sync in the README because the leader sequences
were restated there by hand.

- **Key-level facts** — what keys / combos / leader sequences do, the layout
  grids — are GENERATED into `docs/reference.md` from `keymap.c` by
  `make gen-docs`. Never restate them by hand; link to `reference.md`.
- **The narrative** — a layout's rationale, metrics, mnemonics — has ONE
  canonical home (`docs/ru-balanced-layout.md`, with the `.html` as its visual
  twin). README and code comments point to it; they do not duplicate it.
- A number appears in exactly one place. A code comment describes the present
  ("why the code is this way"), never the edit history — history lives in commits
  and PRs, and reads as noise to someone who never saw the previous revision.
