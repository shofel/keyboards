# Userspace Unicode Community Module (`unicode_ru`)

Emits Unicode from userspace, bypassing QMK's own input-mode machinery. Despite
the `unicode_ru` name it covers Russian Cyrillic **and** the typographic symbols
(« » — № §) and currency signs (₺ ₽ €) the Cantor emits. Cyrillic is driven by
`unicode_map`; standalone glyphs go through `ru_emit_glyph`.

## Backends

Which one is live is held in `ru_backend`, selected by the leader (`leader,r,c`
compose · `leader,r,v` vim · `leader,r,w` windows):

| | `RU_BACKEND_COMPOSE` (default) | `RU_BACKEND_VIM` | `RU_BACKEND_WINDOWS` |
|---|---|---|---|
| Emits | `Compose` + a private 2-char code | `i_CTRL-V U <8 hex>` | `Alt`+numpad hex (BMP only) |
| Works in | anything, host-wide | vim / neovim only | anything, on Windows |
| Needs | host xkb `compose:sclk` + `~/.XCompose` | nothing | `EnableHexNumpad=1` (+reboot) |

Compose is rolling-safe: its commit boundary doesn't depend on modifier timing,
unlike the ibus hex handshake it replaced. Vim mode needs no host setup at all,
which is the whole point of having it. Windows mode is a faithful port of QMK's
`UNICODE_MODE_WINDOWS` for hosts without a compose key — it is **unverified on
Linux** (needs a Windows host to QA) and BMP-only, which suffices for Cyrillic
and ₺/₽/€ (astral emoji stay on compose).

All are emitted here in userspace, so neither `UNICODE_MODE_LINUX` nor the
out-of-tree `UNICODE_MODE_VIM` patch is involved.

The backend is per-session state, not a persisted setting: the keymap selects it
when entering the Russian layer and resets it to compose on the way out. That
matters because a few `unicode_map` keys (`—` `№` `§`) live on the symbol layer
and stay reachable with Russian off — a sticky vim backend would type raw
`Ctrl-V` escapes into ordinary apps.

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
- `ru_backend` / `ru_backend_t` - the active emission backend (see above)
- `ru_unicode_process(keycode, record)` - call first in `process_record_user`; returns true when it consumed a `unicode_map` key, in which case stop processing
- `ru_emit_glyph(compose_code, codepoint)` - emit one standalone glyph via whichever backend is active
- `ru_compose_emit_code(code)` / `ru_vim_emit_codepoint(cp)` - the per-backend emitters, if you need to force one

Use these in your keymap layers like:

```c
[L_RUSSIAN] = LAYOUT(
    RU_A, RU_B, RU_V, ...
)
```

