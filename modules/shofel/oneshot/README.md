# Oneshot Modifiers Community Module

Eager oneshot modifier implementation inspired by Callum's oneshot. Allows stacking multiple oneshot modifiers and carrying them between layers.

## Documentation

- [QMK Community Modules Documentation](https://docs.qmk.fm/features/community_modules#writing-a-qmk-community-module)

## Usage

Add this module to your `keymap.json`:

```json
{
    "modules": [
        "shofel/oneshot"
    ]
}
```

## Interface

The module provides the following interface (via `introspection.h`):

- `oneshot_state_entry_t` - Structure for oneshot state entries
- `oneshot_state_entries[]` - Array of oneshot state entries (user-defined)
- `oneshot_process_event()` - Callback for state changes (user-defined)
- `is_oneshot_cancel_key()` - Callback to determine cancel keys (user-defined)
- `is_oneshot_ignored_key()` - Callback to determine ignored keys (user-defined)

The module automatically processes keycodes via the `process_record_oneshot` hook.

