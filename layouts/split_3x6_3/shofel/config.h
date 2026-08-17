#define MAX_DEFERRED_EXECUTORS 10

#define UNICODE_TYPE_DELAY 0

/* No LEADER_* settings: QMK's stock leader is disabled (LEADER_ENABLE=no). The
 * custom timeoutless leader fires on a unique match with no per-key timeout. */

#define LAYER_STATE_8BIT
#define COMBO_ONLY_FROM_LAYER 0
/* Enables the combo_should_trigger() callback (keymap.c), which gates the RU
 * backend-select chords (r+v -> vim, r+w -> Windows) to the armed leader. Without
 * this define the callback is never compiled in, so those combos fire in ordinary
 * typing and their swallowed output keycodes eat the keys. */
#define COMBO_SHOULD_TRIGGER

#define DEBOUNCE 15

/* MOUSE
 * Pointer control is Orbital Mouse (polar, getreuer/orbital_mouse) on L_MOUSE.
 * Orbital replaces QMK mousekeys (its module sets mousekey:false) and manages
 * its own speed curve (defaults), so the old MK_* stock-mousekey tuning is
 * gone. */

/* Wheel speed: orbital's wheel is analog — it accumulates while the key is held
 * and only emits a scroll notch once it crosses a threshold. At the default 0.2
 * that is ~5 task ticks (~80 ms) of holding before the first notch, so a quick
 * tap scrolls nothing. Bump it so a firm tap (~2 ticks) already registers and a
 * hold scrolls faster. Tune to taste. */
#define ORBITAL_MOUSE_WHEEL_SPEED 0.6
