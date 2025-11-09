#pragma once

#include QMK_KEYBOARD_H

#define ONESHOT_STATE_SIZE 10

/**
 * State of a oneshot trigger
 *
 * ## Transitions
 * os_up_unqueued -> os_down_unused
 * os_down_unused -> os_down_used
 *                -> os_up_queued
 * os_up_queued -> os_up_unqueued
 */
typedef enum {
    os_up_unqueued,
    os_up_queued,
    os_down_unused,
    os_down_used,
} oneshot_state_t;

/* A trigger and its state */
typedef struct {
  uint16_t trigger;
  oneshot_state_t *state; // TODO ptr or a value?
} oneshot_state_entry_t;

/* */

void oneshot_process_record(
    oneshot_state_entry_t state_entries[],
    uint16_t keycode,
    keyrecord_t *record
);

void oneshot_process_record_single(
    uint16_t trigger,       // oneshot key
    oneshot_state_t *state, // oneshot key's state
    uint16_t keycode, // event key
    keyrecord_t *record // event details
);

/* Interface. These are to be defined by consumer. */

// State of all oneshots
oneshot_state_entry_t oneshot_state_entries[ONESHOT_STATE_SIZE];

// Handle change of state
bool oneshot_process_event(uint16_t trigger, oneshot_state_t state);

// Defines keys to cancel oneshot mods.
bool is_oneshot_cancel_key(uint16_t keycode);

// Defines keys to ignore when determining
// whether a oneshot mod has been used. Setting this to modifiers and layer
// change keys allows stacking multiple oneshot modifiers, and carrying them
// between layers.
bool is_oneshot_ignored_key(uint16_t keycode);
