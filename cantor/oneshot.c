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

/* Process record against all triggers, one by one. */
void oneshot_process_record(
    oneshot_state_entry_t state_entries[],
    size_t size,
    uint16_t keycode,
    keyrecord_t *record
) {
  for (size_t i = 0; i < size; i++) {
    oneshot_state_entry_t *entry = &oneshot_state_entries[i];
    oneshot_process_record_single(entry->trigger, entry->state, keycode, record);
  }
  return;
}

/* Process record against a single given oneshot trigger. */
void oneshot_process_record_single(
    uint16_t trigger,       // oneshot key
    oneshot_state_t *state, // oneshot key's state
    uint16_t keycode, // event key
    keyrecord_t *record // event details
) {
    if (keycode == trigger) {
        if (record->event.pressed) { // Trigger keydown
            /* What if two physical keys represent the same trigger?
             * As for me, it's all right. */
            *state = os_down_unused;
        } else { // Trigger keyup
            switch (*state) {
            case os_down_unused:
                // If we didn't use the mod while trigger was held, queue it.
                *state = os_up_queued;
                break;
            case os_down_used:
                // If we did use the mod while trigger was held, unregister it.
                *state = os_up_unqueued;
                break;
            default:
                break;
            }
        }
    } else { // when not a trigger
        if (record->event.pressed) {
            // Cancel oneshot on designated cancel keydown.
            if (is_oneshot_cancel_key(keycode)) {
                *state = os_up_unqueued;
            }
        } else {
            // On non-ignored keyup, consider the oneshot used.
            // We don't deactivate mod on keydown to let it do the job.
            if (!is_oneshot_ignored_key(keycode)) {
                switch (*state) {
                case os_down_unused:
                    *state = os_down_used;
                    break;
                case os_up_queued:
                    *state = os_up_unqueued;
                    break;
                default:
                    break;
                }
            }
        }
    }

    /* Publish event */
    // TODO deduplicate. Publish only when changed
    oneshot_process_event(trigger, *state);
}
