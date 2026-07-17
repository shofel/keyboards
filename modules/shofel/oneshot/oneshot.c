/**
 * Oneshot Modifiers Community Module
 * Inspired by Callum's Oneshot Implementation
 *
 * Source: https://github.com/callum-oakley/qmk_firmware/blob/master/users/callum/oneshot.c
 *
 * You specify a key which controls (trigger), and a key under control (triggee).
 *   Tap trigger -> triggee pressed
 *   Tap another trigger -> now two triggees are pressed
 *   Tap another key -> on keyup all trigees released
 *
 * These are eager oneshots. That is, keydown is sent to the wire right away,
 * not waiting for the `other` key.
 */

#include QMK_KEYBOARD_H
#include "introspection.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

/* Forward declaration */
static void oneshot_process_record_single(
    oneshot_state_entry_t *oneshot,
    oneshot_event_t event
);

/* Process record against all triggers, one by one. */
void oneshot_process_record(
    oneshot_state_entry_t state_entries[],
    size_t size,
    uint16_t keycode,
    keyrecord_t *record
) {
  bool is_ignored_key = is_oneshot_ignored_key(keycode);
  bool pressed = record->event.pressed;

  for (size_t i = 0; i < size; i++) {
    oneshot_state_entry_t *oneshot = &state_entries[i];

    bool is_trigger = oneshot->trigger == keycode;

    // What happened?
    oneshot_event_t event;
         if (is_trigger && pressed)  { event = os_trigger_down; }
    else if (is_trigger && !pressed) { event = os_trigger_up; }
    else if (is_ignored_key)         { event = os_ignore; }
    else if (!pressed)               { event = os_other_up; }
    else                             { event = os_other_down; }

    oneshot_state_t saved_state = oneshot->state;

    /* Advance state of the machine. */
    oneshot_process_record_single(oneshot, event);

    /* Publish event */
    if (saved_state != oneshot->state) {
        oneshot_process_event(oneshot);
    }
  }
  return;
}

/* Process record against a single given oneshot trigger. The transition table
 * and effect rule are the pure oneshot_fsm.h (unit-tested off-target); this only
 * applies the resulting register/unregister to the wire. */
static void oneshot_process_record_single(
    oneshot_state_entry_t *oneshot,
    oneshot_event_t event
) {
    oneshot->state = oneshot_next_state(oneshot->state, event);

    switch (oneshot_effect(oneshot->state)) {
      case os_fx_register:   register_code16(oneshot->triggee);   break;
      case os_fx_unregister: unregister_code16(oneshot->triggee); break;
      case os_fx_none: break;
    }
}

void oneshot_cancel(void) {
  for (size_t i = 0; i < oneshot_state_entries_size; i++) {
    oneshot_state_entry_t *oneshot = &oneshot_state_entries[i];
    oneshot_state_t saved_state = oneshot->state;
    oneshot->state = os_up_unqueued;
    unregister_code16(oneshot->triggee);
    if (saved_state != oneshot->state) {
      oneshot_process_event(oneshot);
    }
  }
}

/* Community Module hook: process_record */
bool process_record_oneshot(uint16_t keycode, keyrecord_t *record) {
    // Get the user-defined state entries and size
    extern oneshot_state_entry_t oneshot_state_entries[];
    extern size_t oneshot_state_entries_size;

    oneshot_process_record(
        oneshot_state_entries,
        oneshot_state_entries_size,
        keycode,
        record
    );

    return true; // Continue processing
}

