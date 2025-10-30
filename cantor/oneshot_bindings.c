#include QMK_KEYBOARD_H
#include "oneshot_engine.h"
#include "oneshot_bindings.h"

/* QMK-like OSM behavior: hold registers mod; tap queues one-shot and unregisters */
static void ops_mod_on_press(void *ctx)   { register_mods((uint8_t)(uintptr_t)ctx); }
static void ops_mod_on_release(void *ctx) { unregister_mods((uint8_t)(uintptr_t)ctx); }
static void ops_mod_on_queue_begin(void *ctx) {
  uint8_t bit = (uint8_t)(uintptr_t)ctx;
  set_oneshot_mods(get_oneshot_mods() | bit);
  unregister_mods(bit);
}
static void ops_mod_on_queue_end(void *ctx) { (void)ctx; }
const oneshot_ops OSM_OPS = {
  .on_press = ops_mod_on_press,
  .on_release = ops_mod_on_release,
  .on_queue_begin = ops_mod_on_queue_begin,
  .on_queue_end = ops_mod_on_queue_end,
};

/* OSL behavior: hold = layer_on; tap = oneshot layer */
static void ops_layer_on_press(void *ctx)   { layer_on((uint8_t)(uintptr_t)ctx); }
static void ops_layer_on_release(void *ctx) { layer_off((uint8_t)(uintptr_t)ctx); }
static void ops_layer_on_queue_begin(void *ctx) {
  uint8_t layer = (uint8_t)(uintptr_t)ctx;
  layer_off(layer);
  /* Use OSL tap to arm next-key oneshot reliably */
  tap_code16(OSL(layer));
}
static void ops_layer_on_queue_end(void *ctx) { (void)ctx; }
const oneshot_ops OSL_OPS = {
  .on_press = ops_layer_on_press,
  .on_release = ops_layer_on_release,
  .on_queue_begin = ops_layer_on_queue_begin,
  .on_queue_end = ops_layer_on_queue_end,
};


