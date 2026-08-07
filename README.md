# Cantor / Dactyl keymap — the BOO layout

A personal QMK keymap for a **Cantor** choc split and a **Dactyl Manuform**, both
driven by *one* shared keymap (via QMK's [layouts feature](https://docs.qmk.fm/feature_layouts)).
The base is the [BOO layout](https://ballerboo.github.io/boolayout/) — a Dvorak
variant tuned for rollover — on a 3×6+3 split.

**→ [Full keymap reference](docs/reference.md)** — every layer, combo, leader
sequence and emoji, generated from the firmware so it never drifts.

What's interesting about it:

- **Shiftless symbols.** A dedicated Symbol layer gives every punctuation mark its
  own key, biased to the Space hand so a `symbol → Space` sequence is a one-hand roll.
- **Vertical combos.** Mods and brackets are same-column two-key chords —
  comfortable and misfire-free on choc switches.
- **Leader sequences.** One leader key drives layer toggles, text edits, currency
  (₺ ₽ €) and emoji.
- **Russian, host-wide.** Cyrillic is typed on a firmware layer and emitted as X
  Compose sequences, so it works in any app and survives fast rolls.
- **Mouse layers.** Orbital-Mouse polar steering, plus an experimental digitizer
  "bisect" mode.

This is a QMK [external userspace](https://github.com/qmk/qmk_userspace) — keymaps
and modules that build against upstream `qmk_firmware` without living inside it.

## Structure

This repository follows the [official QMK userspace template](https://github.com/qmk/qmk_userspace) structure:

- `keyboards/` - Keymap files organized by keyboard
- `layouts/` - Contains a 6x3_3 keymap, which is used for both Cantor and Dactyl (see [QMK layouts documentation](https://docs.qmk.fm/feature_layouts#supporting-a-layout))
- `modules/shofel/` - QMK community modules (oneshot, unicode_ru)
- `qmk.json` - Build targets configuration
- `docs/reference.md` - generated keymap reference

## Setup

Tooling (`qmk`, `make`, python) comes from a [devenv](https://devenv.sh) shell,
loaded automatically by [direnv](https://direnv.net). Entering the repo
configures qmk for this userspace — `overlay_dir` → the repo, `qmk_home` → an
upstream `qmk_firmware` checkout — and clones that firmware on first use, so
there is nothing to wire up by hand.

1. Enter the environment (from the repo root):
   ```bash
   direnv allow          # auto-loads on cd; without direnv, run `devenv shell`
   ```
   The upstream `qmk_firmware` checkout defaults to `~/qmk_firmware`; to reuse an
   existing one elsewhere, export `S_QMK_FIRMWARE` before entering the shell.

2. Build a keymap:
   ```bash
   make build            # Cantor — shorthand for `qmk compile -kb cantor -km shofel`
   # Or Dactyl, with the shared keymap:
   qmk compile -kb handwired/dactyl_manuform/5x6_5 -km shofel
   ```

3. Flash a keymap. The Cantor is a split — whichever half is plugged into USB is
   the one flashed, so run this once per half, connected directly rather than
   over TRRS.

   `make flash` polls for the bootloader, so start it first and enter the
   bootloader after:

   - **Boot combo** — all three thumb keys of that half at once
     (`boot_combo_left` / `boot_combo_right` in `keymap.c`).
   - **Half with no firmware** — the combo needs firmware to run, so hold
     `BOOT0` while plugging in USB instead.

   ```bash
   # Build + flash one half (Cantor only):
   make flash

   # Or manually — compile first, then:
   dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D $(qmk config -ro user.qmk_home | cut -d= -f2)/cantor_shofel.bin

   # Note: `qmk flash -kb cantor -km shofel` also works but requires
   # the keyboard to be in bootloader mode before the command finishes compiling.
   ```

## Modules

The `modules/shofel/` directory contains QMK community modules:

- **oneshot** - Custom oneshot modifier implementation (inspired by Callum's one). Allows stacking multiple oneshot modifiers and carrying them between layers.
- **unicode_ru** - Russian Unicode support with unicode_map definitions for Cyrillic alphabet, plus a Compose-sequence emission backend (see below).

## Russian unicode input (compose mode)

Russian is typed on a firmware layer and emitted as keystrokes. Backends, by leader:

- `leader,r` / `leader,c` — **compose mode (default)**: each glyph (Cyrillic, `« »`,
  `— № §`) is emitted as one X Compose sequence (`Compose` + a private code),
  which is robust to rolling/overlapping keystrokes.
- `leader,v` — vim mode (vim-native unicode); `leader,e` — back to English.
- `leader,a,<sel>` / `leader,i,<sel>` — **emoji** (host-wide, via the same Compose
  backend). `a`/`i` are a home-row mirror pair, so either hand works:
  `t`🌷 `r`🌹 `c`🌸 `h`🌺 `s`🌻 `d`🌼 · `u`👍 `o`👌 `k`🤔 `m`🧐 `n`🤝.
- All leader sequences, combos and layer schemes: see
  [docs/reference.md](docs/reference.md) (generated).

The old ibus-hex backend (`UNICODE_MODE_LINUX`) is retired — it corrupted when
letters rolled. Host setup is a home-manager module in this repo, imported by the
dotfiles flake:

```nix
# dotfiles flake.nix inputs:
cantor-kb = { url = "github:shofel/keyboards"; flake = false; };
# dotfiles home-manager module:
imports = [ "${inputs.cantor-kb}/nix/cantor-compose-ru.nix" ];
cantor.composeRu = {
  enable = true;
  baseXkbOptions = [ "terminate:ctrl_alt_bksp" "lv3:ralt_switch" ];
};
```

It installs `~/.XCompose` and sets the xkb `compose:sclk` option (Scroll Lock =
Compose). The Compose table, `~/.XCompose`, and a cheatsheet are all generated by
`tools/gen_unicode_compose.py` (single source of truth). Assumes the OS layout is
Latin (`us`) while typing.

### Applying a Compose-table change to the host

`~/.XCompose` is generated by home-manager from `tools/XCompose.generated`, read
through the `cantor-kb` flake input — which is **pinned** to a revision. So after
this repo changes the table (new glyphs, emoji), the host keeps the old table
until you re-lock the input and rebuild, then reload Compose:

```bash
# in your dotfiles flake:
nix flake update cantor-kb                    # re-lock cantor-kb to the new commit
sudo nixos-rebuild switch --flake .#<host>    # (or: home-manager switch --flake .#<name>)
ibus restart                                  # reload ~/.XCompose (or re-login), then reopen apps
```

Symptom of a stale table: pressing an unmapped sequence (e.g. a new emoji) leaves
a pending Compose indicator "stuck" — the glyph isn't in the table the IM loaded.

## Development

This repository is configured for LSP support with clangd. The `.clangd` and `.clang-format` files are based on the [official QMK userspace template](https://github.com/qmk/qmk_userspace).

To regenerate `compile_commands.json`:
```bash
qmk compile -kb cantor -km shofel --compiledb
```

### Conventions

- Layer schemes — the in-`LAYOUT` comments in `keymap.c` and
  [docs/reference.md](docs/reference.md) — are GENERATED from the `LAYOUT`
  arrays by `tools/gen_layer_schemes.py`. After changing a layer, run
  `make gen-docs` in the same PR; `make test` (and CI) fail on drift.

See [TODO.md](TODO.md) for planned improvements, and
[docs/known-limitations.md](docs/known-limitations.md) for accepted quirks.

## Reference

This repository is based on the [official QMK userspace template](https://github.com/qmk/qmk_userspace). For more information about external userspaces, see the [QMK documentation](https://docs.qmk.fm/).
