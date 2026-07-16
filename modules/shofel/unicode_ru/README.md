# Compose Unicode Community Module (`unicode_ru`)

Provides Compose-based Unicode emission for QMK keymaps. Despite the `unicode_ru`
name, it covers Russian Cyrillic **and** the typographic symbols (« » — № §) and
currency signs (₺ ₽ €) the Cantor emits — all as rolling-safe Compose sequences.
Cyrillic is driven by `unicode_map`; standalone glyphs (symbols, currency) are
emitted directly via the exported `ru_compose_emit_code`.

## Documentation

- [QMK Community Modules Documentation](https://docs.qmk.fm/features/community_modules#writing-a-qmk-community-module)
- [QMK Unicode Documentation](https://docs.qmk.fm/features/unicode)

## Usage

Add this module to your `keymap.json`:

```json
{
    "modules": [
        "shofel/unicode_ru"
    ]
}
```

The module automatically enables `UNICODE_COMMON` and `UNICODEMAP_ENABLE` features.

## Interface

The module provides (via `introspection.h`):

- `enum unicode_names` - Enumeration of Russian Unicode characters (RU_LC_* for lowercase, RU_UC_* for uppercase, U_DOT, U_COMMA)
- `unicode_map[]` - PROGMEM array mapping enum values to Unicode code points
- `RU_*` macros - Convenience macros for Russian letters (RU_A, RU_B, RU_V, etc.) that combine lowercase and uppercase variants using `UP()`. Also includes `RU_DOT` which combines U_DOT and U_COMMA.

Use these in your keymap layers like:

```c
[L_RUSSIAN] = LAYOUT(
    RU_A, RU_B, RU_V, ...
)
```

