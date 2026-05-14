# UCIS Emoji Community Module

Provides UCIS emoji input support with predefined emoji sequences for flowers and common reactions.

## Documentation

- [QMK Community Modules Documentation](https://docs.qmk.fm/features/community_modules#writing-a-qmk-community-module)
- [QMK UCIS Documentation](https://docs.qmk.fm/features/ucis)

## Usage

Add this module to your `keymap.json`:

```json
{
    "modules": [
        "shofel/ucis_emoji"
    ]
}
```

The module automatically enables `UCIS_ENABLE` and `UNICODE_COMMON` features.

## Interface

The module provides (via `introspection.h`):

- `ucis_symbol_table[]` - PROGMEM array of UCIS emoji sequences

## Available Emoji Sequences

### Flowers
- `tulip` → 🌷
- `rose` → 🌹
- `cherry` → 🌸 (cherry blossom)
- `hibiscus` → 🌺
- `sunflower` → 🌻
- `daisy` → 🌼

### Reactions
- `thumbsup` → 👍
- `ok` → 👌
- `think` → 🤔
- `monocle` → 🧐
- `handshake` → 🤝

## Usage in Keymap

To trigger UCIS input, you can use the `ucis_start()` function. For example, in a leader sequence:

```c
void leader_end_user(void) {
  if (leader_sequence_one_key(KC_U)) {
    ucis_start();
  }
}
```

Alternatively, you can use the `QK_UCIS` keycode with `tap_code16(QK_UCIS)` if you prefer the keycode-based approach.

Then type the sequence (e.g., `tulip`, `ok`) and press Space or Enter to complete.

