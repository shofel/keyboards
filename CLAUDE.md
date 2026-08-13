# Cantor / shofel keymap — working notes for agents & contributors

## Flashing

- **Flash the LEFT half only.** The left half is the one we keep on USB, and the
  only half we build and flash firmware for; the right half is not flashed in
  normal work. After a keymap change, flash the left half once (`make flash`) —
  do not flash the right half unless explicitly asked.

## Build & test

- `make build` — `qmk compile` for `cantor:shofel`. Needs the devenv toolchain;
  from a plain shell run it via `direnv exec . make build`.
- `make test` — off-target host tests (bisect, oneshot, compose, scheme-drift).
- After changing the keymap or its combos, run `make gen-docs` to regenerate
  `docs/reference.md` and the in-`LAYOUT` scheme comments — otherwise `make test`
  (its `test-schemes` drift check) fails.
