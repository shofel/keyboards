# TODO

## Modules

- oneshot: allow press two oneshots at a time to schedule both

TODO next
- mouse
  - come up with keys to enable bisect and radial mouse modes
    - use three top-row keys of the strong fingers on the left hand (, u c). The layer is mouse. comfort of the keys: u > c > ,
      - , -> stock mode
      - u -> radial mouse
      - c -> bisect
      - default is bisect
  - bisect with digitizer. I see a digitizer in gnome settings. It should work now!. I mean the feature is about position the pointer like a binary search
  - radial mouse by gautrier (plug his module)

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

