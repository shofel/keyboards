/**
 * Inspired by Callum's Oneshot Implementation
 *
 * Source: https://github.com/callum-oakley/qmk_firmware/blob/master/users/callum/oneshot.c
 *
 * Main difference with the original:
 *   - no effects here. Only state updates
 *   - changes of states are being published by the `process_oneshot_event()` call
 */

#include QMK_KEYBOARD_H
#include "oneshot.h"

size_t size = sizeof(oneshot_state_entries) / sizeof(oneshot_state_entries[0])

oneshot_state_t* oneshot_get_state(uint16_t trigger) {
  for (size_t i = 0; i < size; i++) {
    oneshot_state_entry_t *entry = &oneshot_state_entries[i]

    if (entry->trigger == trigger) {
      return entry
    }
  }
}

void oneshot_update_state(
    uint16_t trigger, // oneshot key
    uint16_t keycode, // event key
    keyrecord_t *record // event details
) {
    // Get state for `trigger`
    oneshot_state_entry_t *state = oneshot_get_state(trigger)
    if (state == NULL) return;

    if (keycode == trigger) {
        if (record->event.pressed) {
            // Trigger keydown
            if (*state == os_up_unqueued) {
            os_trigger_down(trigger)
            }
            *state = os_down_unused;
        } else {
            // Trigger keyup
            switch (*state) {
            case os_down_unused:
                // If we didn't use the mod while trigger was held, queue it.
                *state = os_up_queued;
                break;
            case os_down_used:
                // If we did use the mod while trigger was held, unregister it.
                *state = os_up_unqueued;
                unregister_code(mod);
                break;
            default:
                break;
            }
        }
    } else {
        if (record->event.pressed) {
            if (is_oneshot_cancel_key(keycode) && *state != os_up_unqueued) {
                // Cancel oneshot on designated cancel keydown.
                *state = os_up_unqueued;
                unregister_code(mod);
            }
        } else {
            if (!is_oneshot_ignored_key(keycode)) {
                // On non-ignored keyup, consider the oneshot used.
                switch (*state) {
                case os_down_unused:
                    *state = os_down_used;
                    break;
                case os_up_queued:
                    *state = os_up_unqueued;
                    unregister_code(mod);
                    break;
                default:
                    break;
                }
            }
        }
    }
}
