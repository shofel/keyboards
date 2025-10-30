#include "quantum.h"
#include "ru_manager.h"

static uint8_t ru_suspend_depth = 0;

void ru_suspend(int layer) {
  if (layer < 0) return;
  if (ru_suspend_depth == 0 && layer_state_is(layer)) {
    layer_off(layer);
    ru_suspend_depth = 1;
    return;
  }
  if (ru_suspend_depth > 0) ru_suspend_depth++;
}

void ru_resume(int layer) {
  if (layer < 0) return;
  if (ru_suspend_depth == 0) return;
  ru_suspend_depth--;
  if (ru_suspend_depth == 0) {
    layer_on(layer);
  }
}

int ru_suspended(int layer) {
  if (layer < 0) return 0;
  return ru_suspend_depth > 0;
}


