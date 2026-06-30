# TODO

## Layout

- leader seq for gui+L_NUM : leader,w

### Known / accepted
- kitty control doesn't work while Ru active: leader,k sends win+T fine, but the
  follow-up control keys are typed on the Russian layer (come out Cyrillic).
  Not a leader bug (leader,k itself records correctly). Left as-is — fixing would
  mean leader,k also dropping the toggle layer first.

## Modules

- oneshot: allow press two oneshots at a time to schedule both
- 8 mouse
  - bisect with digitizer. I see a digitizer in gnome settings. It should work now!
  - radial mouse by gautrier
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

