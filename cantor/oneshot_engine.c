#include QMK_KEYBOARD_H
#include "ru_manager.h"
#include "oneshot_engine.h"

__attribute__((weak)) bool is_oneshot_cancel_key(uint16_t keycode) {
  return keycode == KC_ESC;
}

__attribute__((weak)) bool is_oneshot_ignored_key(uint16_t keycode) {
  switch (keycode) {
    case KC_LCTL: case KC_LSFT: case KC_LALT: case KC_LGUI:
    case KC_RCTL: case KC_RSFT: case KC_RALT: case KC_RGUI:
      return true;
  }
  return false;
}

void update_oneshot_generic(
    oneshot_state *state,
    uint16_t trigger,
    uint16_t keycode,
    keyrecord_t *record,
    const oneshot_ops *ops,
    void *ctx,
    int ru_layer
) {
  if (keycode == trigger) {
    if (record->event.pressed) {
      if (*state == os_up_unqueued) {
        if (ru_layer >= 0) ru_suspend((uint8_t)ru_layer);
        if (ops && ops->on_press) ops->on_press(ctx);
      }
      *state = os_down_unused;
    } else {
      switch (*state) {
        case os_down_unused:
          if (ops && ops->on_queue_begin) ops->on_queue_begin(ctx);
          *state = os_up_queued;
          break;
        case os_down_used:
          if (ops && ops->on_release) ops->on_release(ctx);
          if (ru_layer >= 0) ru_resume((uint8_t)ru_layer);
          *state = os_up_unqueued;
          break;
        default:
          break;
      }
    }
  } else {
    if (record->event.pressed) {
      if (is_oneshot_cancel_key(keycode) && *state != os_up_unqueued) {
        if (*state == os_up_queued) {
          if (ops && ops->on_queue_end) ops->on_queue_end(ctx);
        } else {
          if (ops && ops->on_release) ops->on_release(ctx);
        }
        if (ru_layer >= 0) ru_resume((uint8_t)ru_layer);
        *state = os_up_unqueued;
      }
    } else {
      if (!is_oneshot_ignored_key(keycode)) {
        switch (*state) {
          case os_down_unused:
            *state = os_down_used;
            break;
          case os_up_queued:
            if (ops && ops->on_queue_end) ops->on_queue_end(ctx);
            if (ru_layer >= 0) ru_resume((uint8_t)ru_layer);
            *state = os_up_unqueued;
            break;
          default:
            break;
        }
      }
    }
  }
}


