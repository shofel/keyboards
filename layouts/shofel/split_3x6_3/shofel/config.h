#define MAX_DEFERRED_EXECUTORS 10

#define UNICODE_TYPE_DELAY 0

#define LEADER_PER_KEY_TIMING
#define LEADER_NO_TIMEOUT
#define LEADER_TIMEOUT 100

#define LAYER_STATE_8BIT
#define COMBO_ONLY_FROM_LAYER 0

#define DEBOUNCE 15

/* MOUSE
 * Pointer control is Orbital Mouse (polar, getreuer/orbital_mouse) on L_MOUSE,
 * and the digitizer bisect on L_MOUSE_BISECT. Orbital replaces QMK mousekeys
 * (its module sets mousekey:false) and manages its own speed curve (defaults),
 * so the old MK_* stock-mousekey tuning is gone. */
