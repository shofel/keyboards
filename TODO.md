# TODO

## Cantor

- TO TEST: backspace @ vert combo mirror to fsyslayer
- TO TEST: 2 mouse: turn one btn1 to a sticky - for selection (activate on tap; deactivate on tap)
- TO TEST: 6 employ ucis for emoji: tulip and other flowers, tup=thumbup, ok, think, monocle
- leader layer activators : make them toggles
- leader seq for gui+L_NUM
- handier one-handed alt+`
- 4 sym: fill the gaps for some useful stuff: _
- 6 revise the sym layer: braces
- 2 caps word by double shift

## Modules

- oneshot: allow press two oneshots at a time to schedule both
- 8 mouse - bisect with digitizer. I see a digitizer in gnome settings. It should work now!
- 9 rawhid for seamless unicode modes in vim and linux
- 88 bug: fast seq [os_sft c a] resolves to [C A], while [C a] expected

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

