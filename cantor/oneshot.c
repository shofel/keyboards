/**
 * Inspired by Callum's Oneshot Implementation
 *
 * Source: https://github.com/callum-oakley/qmk_firmware/blob/master/users/callum/oneshot.c
 *
 * You specify a key which controls (trigger), and a key under control (triggee).
 *   Tap trigger -> triggee pressed
 *   Tap another trigger -> now two triggees are pressed
 *   Tap another key -> on keyup all trigees released
 *
 * For some reason it doesn't work with MO() keys.
 *
 * These are eager oneshots. That is, keydown is sent to the wire right away,
 * not waiting for the `other` key.
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
  bool is_ignored_key = is_oneshot_ignored_key(keycode);
  bool is_cancel_key = is_oneshot_cancel_key(keycode);
  bool pressed = record->event.pressed;

  for (size_t i = 0; i < size; i++) {
    oneshot_state_entry_t *oneshot = &oneshot_state_entries[i];

    bool is_trigger = oneshot->trigger == keycode;

    // What happened?
    oneshot_event_t event;
         if (is_trigger && pressed)     { event = os_trigger_down; }
    else if (is_trigger && !pressed)    { event = os_trigger_up; }
    else if (is_cancel_key && pressed)  { event = os_cancel; }
    else if (is_ignored_key)            { event = os_ignore; }
    else if (!pressed)                  { event = os_other_up; }
    else                                { event = os_ignore; }

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

/* Process record against a single given oneshot trigger. */
void oneshot_process_record_single(
    oneshot_state_entry_t *oneshot,
    oneshot_event_t event
) {
    switch (event) {
      case os_trigger_down:
        oneshot->state = os_down_unused;
        break;
      case os_trigger_up:
        switch (oneshot->state) {
          case os_down_unused:
            oneshot->state = os_up_queued; break;
          case os_down_used:
            oneshot->state = os_up_unqueued; break;
          default: break;
        }
        break;
      case os_other_up:
        switch (oneshot->state) {
          case os_down_unused:
            oneshot->state = os_down_used; break;
          case os_up_queued:
            oneshot->state = os_up_unqueued; break;
          default: break;
        }
        break;
      case os_ignore:
        break;
      case os_cancel:
        oneshot->state = os_up_unqueued;
        break;
    }

    /* Perform effects: apply and release the triggee. */
    if (oneshot->state == os_down_unused) {
      register_code16(oneshot->triggee);
    }
    if (oneshot->state == os_up_unqueued) {
      unregister_code16(oneshot->triggee);
    }
}
