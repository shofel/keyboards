#pragma once

#include <stdint.h>
#include <stdbool.h>
#include QMK_KEYBOARD_H

typedef enum { os_up_unqueued, os_down_unused, os_down_used, os_up_queued } oneshot_state;

typedef struct {
  void (*on_press)(void *ctx);
  void (*on_release)(void *ctx);
  void (*on_queue_begin)(void *ctx);
  void (*on_queue_end)(void *ctx);
} oneshot_ops;

/* Policy helpers (override in your keymap if needed) */
bool is_oneshot_cancel_key(uint16_t keycode);
bool is_oneshot_ignored_key(uint16_t keycode);

void update_oneshot_generic(
    oneshot_state *state,
    uint16_t trigger,
    uint16_t keycode,
    keyrecord_t *record,
    const oneshot_ops *ops,
    void *ctx,
    int ru_layer /* -1 to disable RU-aware */
);


