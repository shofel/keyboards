#pragma once

#include <stdint.h>
#include QMK_KEYBOARD_H

/* oneshot_state_t / oneshot_event_t and the pure transition table (oneshot_fsm.h)
 * are header-only and QMK-free so the state machine can be unit-tested off-target
 * (see tools/test_oneshot_fsm.c). */
#include "oneshot_fsm.h"

#define ONESHOT_STATE_SIZE 10

typedef struct {
  uint16_t trigger; /* key to watch */
  uint16_t triggee; /* key to imitate */
  oneshot_state_t state;
} oneshot_state_entry_t;

/* Interface functions - these must be defined by the keymap */

// State of all oneshots
extern oneshot_state_entry_t oneshot_state_entries[ONESHOT_STATE_SIZE];
extern size_t oneshot_state_entries_size;

// Handle change of state
void oneshot_process_event(oneshot_state_entry_t *oneshot);

// Programmatically cancel all active oneshots.
void oneshot_cancel(void);

// Defines keys to ignore when determining
// whether a oneshot mod has been used. Setting this to modifiers and layer
// change keys allows stacking multiple oneshot modifiers, and carrying them
// between layers.
bool is_oneshot_ignored_key(uint16_t keycode);

