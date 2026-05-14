# TODO

## Cantor

- easier ctrl+esc
- lower vertical combos: use only the index and middle fingers
- TO TEST: backspace @ vert combo mirror to fsyslayer
- 2 mouse: turn one btn1 to a sticky - for selection (activate on tap; deactivate on tap)
- TO TEST: 6 employ ucis for emoji: tulip and other flowers, tup=thumbup, ok, think, monocle
- leader layer activators : make them toggles
- leader seq for gui+L_NUM
- handier one-handed alt+`
- 4 sym: fill the gaps for some useful stuff: _
- 6 revise the sym layer: braces
- 2 caps_word_on() on leader seq one key os_sft
- make `~/` an inward roll on the top row:
  - `~` = top right pinky
  - `/` = top right ring
  - -> and as a consequence, decide where to move `|`?

## Modules

- oneshot: allow press two oneshots at a time to schedule both
- 8 mouse - bisect with digitizer. I see a digitizer in gnome settings. It should work now!
- 9 rawhid for seamless unicode modes in vim and linux
- dream: smoother unicode input on Linux outside of vim
  - today: outside vim relies on ibus/UCIS sequences, which are slow and stateful
  - want: type unicode as fluently as in vim — no mode toggles, no hex codes
  - possible paths: rawhid signal to the host to switch IME, or per-app composer
- 88 bug: fast seq [os_sft c a] resolves to [C A], while [C a] expected
- make ucis and unicodemap work together

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

