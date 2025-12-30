# QMK External Userspace

This repository contains QMK keyboard keymaps and userspace code, structured as an external userspace repository.

## Structure

This repository follows the [official QMK userspace template](https://github.com/qmk/qmk_userspace) structure:

- `keyboards/` - Keymap files organized by keyboard
- `layouts/` - Contains a 6x3_3 keymap, which is used for both Cantor and Dactyl (see [QMK layouts documentation](https://docs.qmk.fm/feature_layouts#supporting-a-layout))
- `modules/shofel/` - QMK community modules (oneshot, unicode_ru)
- `qmk.json` - Build targets configuration

## Setup

1. Configure QMK to use this userspace:
   ```bash
   qmk config user.overlay_dir="$(realpath /home/slava/workspaces-one/keyboards)"
   ```

2. Build a keymap:
   ```bash
   qmk compile -kb cantor -km shofel
   # Or for Dactyl with the shared keymap:
   qmk compile -kb handwired/dactyl_manuform/5x6_5 -km shofel
   ```

3. Flash a keymap:
   ```bash
   qmk flash -kb cantor -km shofel
   # Or for Dactyl:
   qmk flash -kb handwired/dactyl_manuform/5x6_5 -km shofel
   ```

## Modules

The `modules/shofel/` directory contains QMK community modules:

- **oneshot** - Custom oneshot modifier implementation (inspired by Callum's one). Allows stacking multiple oneshot modifiers and carrying them between layers.
- **unicode_ru** - Russian Unicode support with unicode_map definitions for Cyrillic alphabet.

## Development

This repository is configured for LSP support with clangd. The `.clangd` and `.clang-format` files are based on the [official QMK userspace template](https://github.com/qmk/qmk_userspace).

To regenerate `compile_commands.json`:
```bash
qmk compile -kb cantor -km shofel --compiledb
```

See [TODO.md](TODO.md) for planned improvements and tasks.

## Reference

This repository is based on the [official QMK userspace template](https://github.com/qmk/qmk_userspace). For more information about external userspaces, see the [QMK documentation](https://docs.qmk.fm/).
