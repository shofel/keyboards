# QMK External Userspace

This repository contains QMK keyboard keymaps and userspace code, structured as an external userspace repository.

## Structure

This repository follows the [official QMK userspace template](https://github.com/qmk/qmk_userspace) structure:

- `keyboards/` - Keymap files organized by keyboard
- `users/shofel/` - Shared userspace code (oneshot, unicode, etc.)
- `qmk.json` - Build targets configuration

## Setup

1. Configure QMK to use this userspace:
   ```bash
   qmk config user.overlay_dir="$(realpath /home/slava/workspaces-one/keyboards)"
   ```

2. Build a keymap:
   ```bash
   qmk compile -kb cantor -km keymap
   ```

3. Flash a keymap:
   ```bash
   qmk flash -kb cantor -km keymap
   ```

## Keyboards

- **cantor** - Custom keymap with BOO layout, Russian layer, and custom oneshot implementation

## Userspace Code

The `users/shofel/` directory contains:
- `oneshot.c` / `oneshot.h` - Custom oneshot modifier implementation (inspired by Callum's one)
- `unicode.c` - Russian unicode support

## Development

This repository is configured for LSP support with clangd. The `.clangd` and `.clang-format` files are based on the [official QMK userspace template](https://github.com/qmk/qmk_userspace).

To regenerate `compile_commands.json`:
```bash
qmk compile -kb cantor -km keymap --compiledb
```

## Reference

This repository is based on the [official QMK userspace template](https://github.com/qmk/qmk_userspace). For more information about external userspaces, see the [QMK documentation](https://docs.qmk.fm/).

