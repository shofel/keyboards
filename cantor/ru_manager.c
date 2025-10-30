#include QMK_KEYBOARD_H
#include "ru_manager.h"

static uint8_t ru_suspend_depth = 0;

void ru_suspend(uint8_t layer) {
  if (ru_suspend_depth == 0 && layer_state_is(layer)) {
    layer_off(layer);
    ru_suspend_depth = 1;
    return;
  }
  if (ru_suspend_depth > 0) ru_suspend_depth++;
}

void ru_resume(uint8_t layer) {
  if (ru_suspend_depth == 0) return;
  ru_suspend_depth--;
  if (ru_suspend_depth == 0) {
    layer_on(layer);
  }
}

int ru_suspended(uint8_t layer) {
  (void)layer;
  return ru_suspend_depth > 0;
}


