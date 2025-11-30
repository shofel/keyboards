#pragma once

#include <stdint.h>
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

typedef enum {
    os_trigger_down,
    os_trigger_up,
    os_other_up,
    os_ignore,
    os_cancel,
} oneshot_event_t;

typedef struct {
  uint16_t trigger; /* key to watch */
  uint16_t triggee; /* key to imitate */
  oneshot_state_t state;
} oneshot_state_entry_t;

/* */

void oneshot_process_record(
    oneshot_state_entry_t state_entries[],
    size_t state_entries_size,
    uint16_t keycode,
    keyrecord_t *record
);

void oneshot_process_record_single(
    oneshot_state_entry_t *oneshot,
    oneshot_event_t event
);

/* Interface. These are to be defined by consumer. */

// State of all oneshots
oneshot_state_entry_t oneshot_state_entries[ONESHOT_STATE_SIZE];

// Handle change of state
void oneshot_process_event(oneshot_state_entry_t *oneshot);

// Defines keys to cancel oneshot mods.
bool is_oneshot_cancel_key(uint16_t keycode);

// Defines keys to ignore when determining
// whether a oneshot mod has been used. Setting this to modifiers and layer
// change keys allows stacking multiple oneshot modifiers, and carrying them
// between layers.
bool is_oneshot_ignored_key(uint16_t keycode);
