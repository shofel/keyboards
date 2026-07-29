/**
 * A keymap for 6x3_3 layout.
 * Originally made for Cantor; and hopefully useful for a Dactyl.
 */

#include <stdint.h>
#include QMK_KEYBOARD_H
#include "introspection.h"
#include "modules/shofel/unicode_ru/introspection.h"
#include "modules/getreuer/orbital_mouse/introspection.h"
#include "digitizer.h"
#include "bisect_geom.h"

/*
 * Runtime debug logging — all off by default.
 * Flip any flag to `true` (and build with CONSOLE_ENABLE) to stream the
 * corresponding events to `qmk console`. Kept as a ready-to-use toggle hook.
 */
void keyboard_post_init_user(void) {
  debug_enable   = false;
  debug_matrix   = false;
  debug_keyboard = false;
  debug_mouse    = false;
}

/* Fancy looking spare keys. */
#define __ KC_TRNS
#define XX KC_NO

enum my_keycodes {
  KK_RIGHT_ARROW = SAFE_RANGE,
  KK_FAT_RIGHT_ARROW,
  KK_LANGLE,  // « when shifted, < otherwise
  KK_RANGLE,  // » when shifted, > otherwise
  KK_NOOP,

  /* One-shot trigger keys */
  OS_CTL,
  OS_ALT,
  OS_GUI,
  OS_SFT,

  /* Mouse mode switch (live on the mouse layers) */
  KK_MM_POLAR,
  KK_MM_BISECT,

  /* Bisect mode actions */
  KK_BI_L,
  KK_BI_R,
  KK_BI_U,
  KK_BI_D,
  KK_BI_CLICK,
  KK_BI_RESET,
};

/* Layer names */
enum my_layer_names {
  L_BOO,
  L_RUSSIAN,
  L_SYMBOLS,
  L_NUM_NAV,
  L_FKEYS_SYS,
  L_MOUSE,
  L_MOUSE_BISECT,
};

/* Simple thumb keys. */
#define KK_SYMBO OSL(L_SYMBOLS)
#define KK_SHIFT OS_SFT
/* Right inner thumb: tap = Enter, hold = momentary SYMBOLS. */
#define KK_RET   LT(L_SYMBOLS, KC_ENTER)

/* Sticky toggle layers */

/*
 * Russian, F-keys, mouse and num/nav are all driven the same way: exactly one
 * toggle layer is active at a time, enabled by its own leader sequence and
 * disabled by leader,space. State is three concerns:
 *   active_toggle        — which toggle layer the user wants on (or TOGGLE_NONE)
 *   leader_active        — a leader sequence is in progress: mask ANY active
 *                          toggle layer so the sequence keys read from the base
 *                          layout (else e.g. leader,t would read num/nav's key
 *                          at that position instead of KC_T)
 *   mod one-shots        — while a Ctrl/Alt/Gui one-shot holds its mod, mask
 *                          Russian only, so the mod falls through to the Latin
 *                          base while num/nav etc. stay put. Derived from
 *                          one-shot state (mod_ru_suspended), never a counter.
 * applied_layer is the overlay we physically turned on. The reconcile only ever
 * touches our own layer, so it never disturbs a native OSL momentary layer or
 * the base layer (no layer_move).
 */
#define TOGGLE_NONE 0xFF

static uint8_t active_toggle        = TOGGLE_NONE;
static uint8_t applied_layer        = TOGGLE_NONE;
static bool    leader_active        = false;

/* Russian is masked while any Ctrl/Alt/Gui one-shot physically holds its mod, so
 * that chord falls through to the Latin base. Derived from one-shot state rather
 * than a +1/-1 counter, which used to leak (stuck > 0) when a mod one-shot was
 * re-tapped while queued — leaving Russian impossible to re-enable until a
 * toggle_disable (leader,e / Esc / leader,space) forced the counter back to 0. */
static bool mod_ru_suspended(void) {
    for (size_t i = 0; i < oneshot_state_entries_size; i++) {
        uint16_t trigger = oneshot_state_entries[i].trigger;
        if ((trigger == OS_CTL || trigger == OS_ALT || trigger == OS_GUI) &&
            oneshot_mod_held(oneshot_state_entries[i].state)) {
            return true;
        }
    }
    return false;
}

static void toggle_apply(void) {
    uint8_t want = active_toggle;
    if (leader_active) {
        want = TOGGLE_NONE;
    } else if (active_toggle == L_RUSSIAN && mod_ru_suspended()) {
        want = TOGGLE_NONE;
    }
    if (applied_layer == want) {
        return;
    }
    if (applied_layer != TOGGLE_NONE) {
        layer_off(applied_layer);
    }
    if (want != TOGGLE_NONE) {
        layer_on(want);
    }
    applied_layer = want;
}

static void toggle_enable(uint8_t layer) {
    active_toggle = layer;
    toggle_apply();
}

static void toggle_disable(void) {
    active_toggle = TOGGLE_NONE;
    /* Vim mode is only meaningful while Russian is on, but `—` `№` `§` are
     * unicode_map keys living on SYM — reachable with Russian off. A sticky
     * RU_BACKEND_VIM would emit `Ctrl-V U 00002014` into ordinary apps, so the
     * backend dies with the layer rather than outliving it. */
    ru_backend = RU_BACKEND_COMPOSE;
    toggle_apply();
}

static void toggle_reset(void) {
    oneshot_cancel();
    toggle_disable();
}

void leader_suspend(void) {
    leader_active = true;
    toggle_apply();
}

void leader_resume(void) {
    leader_active = false;
    toggle_apply();
}

/* Key overrides */

// Swap , and . under shift:  shifted , -> .   and   shifted . -> ,
const key_override_t comma_override = ko_make_basic(MOD_MASK_SHIFT, KC_COMMA, KC_DOT);
const key_override_t dot_override   = ko_make_basic(MOD_MASK_SHIFT, KC_DOT,   KC_COMMA);

// Suppress ? from the base layer: Shift+/ emits a plain /. `?` lives only on SYM,
// giving it one canonical home that is identical across languages.
const key_override_t slash_override = ko_make_basic(MOD_MASK_SHIFT, KC_SLSH, KC_SLSH);
// Backtick from the base layer: Shift+' -> ` . This frees the base `"` slot.
const key_override_t quote_override = ko_make_basic(MOD_MASK_SHIFT, KC_QUOT, KC_GRV);

const key_override_t *key_overrides[] = {
  &comma_override,
  &dot_override,
  &slash_override,
  &quote_override,
};

/**
 * Combos
 * NB: to add a combo, add it in 3 places
 *
 * Vertical (same-column) combos: a home-row key chorded with the key directly
 * above it (mods, backspace, layer toggles) or below it (brackets).
 * On a Cantor with choc switches these are comfortable and misfire-free —
 * during normal typing you never press two keys of one column with a single
 * finger, so a vertical chord is always unambiguous intent.
 * The top-row pairs (home + above) feel best; the bottom-row pairs (home +
 * below) are more of a reach — which is why the highest-value combos (mods,
 * backspace, primary num-layer access) take the top slots and brackets the
 * bottom.
 */

/* Hit both middle thumb keys for esc. */
const uint16_t PROGMEM esc_combo[]      = {KK_SHIFT, KC_SPACE, COMBO_END};
/* Two outer bottom keys on a single half to get into bootloader. */
const uint16_t PROGMEM boot_combo_left[]  = {KK_NOOP, KK_SYMBO, COMBO_END};
const uint16_t PROGMEM boot_combo_right[] = {KK_RET, KK_NOOP, COMBO_END};
/* On each half: the outermost bottom pinky key + the middle thumb key to reboot the keyboard. */
const uint16_t PROGMEM reset_combo_left[]  = {KK_NOOP, KK_SHIFT, COMBO_END};
const uint16_t PROGMEM reset_combo_right[] = {KC_SPACE, KK_NOOP, COMBO_END};
/* Digraphs */
const uint16_t PROGMEM fat_right_arrow_combo[] = {KC_H, KC_M, COMBO_END}; // =>
const uint16_t PROGMEM right_arrow_combo[]     = {KC_H, KC_K, COMBO_END}; // ->
/* := and != are gone: the SYM `:`->`=` roll and `!` make them unnecessary. */
/* [(<>)] */
const uint16_t PROGMEM square_left_combo[]  = {KC_S, KC_W, COMBO_END};
const uint16_t PROGMEM square_right_combo[] = {KC_N, KC_H, COMBO_END};
const uint16_t PROGMEM brace_left_combo[]   = {KC_E, KC_DOT, COMBO_END};
const uint16_t PROGMEM brace_right_combo[]  = {KC_T, KC_M, COMBO_END};
const uint16_t PROGMEM angle_left_combo[]   = {KC_G, KC_Z, COMBO_END};
const uint16_t PROGMEM angle_right_combo[]  = {KC_B, KC_P, COMBO_END};
/* Vertical combos for mods */
const uint16_t PROGMEM lctl_combo[] = {KC_S, KC_C, COMBO_END};
const uint16_t PROGMEM llt2_combo[] = {KC_E, KC_U, COMBO_END};
const uint16_t PROGMEM lalt_combo[] = {KC_O, KC_COMM, COMBO_END};
const uint16_t PROGMEM lgui_combo[] = {KC_A, KC_QUOT, COMBO_END};
const uint16_t PROGMEM rctl_combo[] = {KC_N, KC_F, COMBO_END};
const uint16_t PROGMEM rlt2_combo[] = {KC_T, KC_D, COMBO_END};
const uint16_t PROGMEM ralt_combo[] = {KC_R, KC_L, COMBO_END};
const uint16_t PROGMEM rgui_combo[] = {KC_I, KC_Y, COMBO_END};
/* G+V -> "  (took over the old backspace slot; backspace now lives on SYM). */
const uint16_t PROGMEM dquo_combo[] = {KC_G, KC_V, COMBO_END};
/* / + - -> one-shot FKEYS/SYS layer. A vertical same-column combo on the right
 * pinky column ('/' top-right pinky, '-' home-row right pinky). SYM no longer needs
 * this combo: it is reached via the left thumb (KK_SYMBO) and the RET layer-tap
 * (right inner thumb). */
const uint16_t PROGMEM fkeys_combo[] = {KC_SLASH, KC_MINUS, COMBO_END};

/* Indices for all combos (designated initializers) */
enum combos {
  CMB_ESC,

  CMB_BOOT_L,
  CMB_BOOT_R,

  CMB_RESET_L,
  CMB_RESET_R,

  CMB_FAT_ARROW,
  CMB_RIGHT_ARROW,

  CMB_SQ_L,
  CMB_SQ_R,
  CMB_BR_L,
  CMB_BR_R,
  CMB_ANG_L,
  CMB_ANG_R,

  CMB_LCTL,
  CMB_LLT2,
  CMB_LALT,
  CMB_LGUI,
  CMB_RCTL,
  CMB_RLT2,
  CMB_RALT,
  CMB_RGUI,

  CMB_DQUO,
  CMB_FSYS,
};

combo_t key_combos[] = {
  [CMB_ESC]        = COMBO(esc_combo, KC_ESC),

  [CMB_BOOT_L]     = COMBO(boot_combo_left,  QK_BOOT),
  [CMB_BOOT_R]     = COMBO(boot_combo_right, QK_BOOT),

  [CMB_RESET_L]    = COMBO(reset_combo_left,  QK_REBOOT),
  [CMB_RESET_R]    = COMBO(reset_combo_right, QK_REBOOT),

  [CMB_FAT_ARROW]  = COMBO(fat_right_arrow_combo, KK_FAT_RIGHT_ARROW),
  [CMB_RIGHT_ARROW]= COMBO(right_arrow_combo, KK_RIGHT_ARROW),

  /* ([<>])  NB: {} = shift+[] ; <>/« » resolved by KK_LANGLE/KK_RANGLE */
  [CMB_SQ_L]       = COMBO(square_left_combo , KC_LBRC),
  [CMB_SQ_R]       = COMBO(square_right_combo, KC_RBRC),
  [CMB_BR_L]       = COMBO(brace_left_combo, KC_LPRN),
  [CMB_BR_R]       = COMBO(brace_right_combo, KC_RPRN),
  [CMB_ANG_L]      = COMBO(angle_left_combo, KK_LANGLE),
  [CMB_ANG_R]      = COMBO(angle_right_combo, KK_RANGLE),

  [CMB_LCTL]       = COMBO(lctl_combo, OS_CTL),
  [CMB_LLT2]       = COMBO(llt2_combo, OSL(L_NUM_NAV)),
  [CMB_LALT]       = COMBO(lalt_combo, OS_ALT),
  [CMB_LGUI]       = COMBO(lgui_combo, OS_GUI),
  [CMB_RCTL]       = COMBO(rctl_combo, OS_CTL),
  [CMB_RLT2]       = COMBO(rlt2_combo, OSL(L_NUM_NAV)),
  [CMB_RALT]       = COMBO(ralt_combo, OS_ALT),
  [CMB_RGUI]       = COMBO(rgui_combo, OS_GUI),

  [CMB_DQUO]       = COMBO(dquo_combo, KC_DQUO),
  [CMB_FSYS]       = COMBO(fkeys_combo, OSL(L_FKEYS_SYS)),
};

/* Oneshot */

oneshot_state_entry_t oneshot_state_entries[] = {
  {OS_CTL, KC_LCTL, os_up_unqueued},
  {OS_ALT, KC_LALT, os_up_unqueued},
  {OS_GUI, KC_LGUI, os_up_unqueued},
  {OS_SFT, KC_LSFT, os_up_unqueued},
};

size_t oneshot_state_entries_size = sizeof(oneshot_state_entries) / sizeof(oneshot_state_entry_t);

/* Allow oneshots to stack up and to penetrate layers. */
bool is_oneshot_ignored_key(uint16_t keycode) {
  /* Ignore oneshot triggers */
  for (size_t i = 0; i < oneshot_state_entries_size; i++) {
    if (oneshot_state_entries[i].trigger == keycode) {
      return true;
    }
  }

  switch (keycode) {
    case OSL(L_NUM_NAV):
    case OSL(L_SYMBOLS):
    case OSL(L_FKEYS_SYS):
      return true;
    default:
      return false;
  }
}

void oneshot_process_event(oneshot_state_entry_t *oneshot) {
  /* A Ctrl/Alt/Gui one-shot changing state may flip whether Russian should be
   * masked, so re-derive it from the current one-shot states. (Shift does not
   * suspend Russian.) mod_ru_suspended() reads the states directly, so there is
   * no counter to keep balanced. */
  if ((oneshot->trigger == OS_CTL) ||
      (oneshot->trigger == OS_ALT) ||
      (oneshot->trigger == OS_GUI))
  {
    toggle_apply();
  }
}

/* Leader */

void leader_start_user(void) {
  leader_suspend();
}

void leader_end_user(void) {
  leader_resume();

  /* Ru. Compose mode (rolling-safe; host xkb compose:sclk + ~/.XCompose) is the
   * default Russian backend — leader,r and leader,c both select it. Vim mode
   * (leader,v) emits vim-native unicode for typing Cyrillic inside vim/neovim.
   * Both backends are emitted in userspace by the unicode_ru module; the old
   * ibus hex backend (UNICODE_MODE_LINUX) and the out-of-tree UNICODE_MODE_VIM
   * are no longer used. */
  if (leader_sequence_one_key(KC_R) || leader_sequence_one_key(KC_C)) {
    ru_backend = RU_BACKEND_COMPOSE;
    toggle_enable(L_RUSSIAN);
  }
  if (leader_sequence_one_key(KC_V)) {
    ru_backend = RU_BACKEND_VIM;
    toggle_enable(L_RUSSIAN);
  }
  /* Back to English: drop the active toggle layer. */
  if (leader_sequence_one_key(KC_E)) {
    ru_backend = RU_BACKEND_COMPOSE;
    toggle_disable();
  }

  /* Disable any active toggle layer (one seq for all) */
  if (leader_sequence_one_key(KC_SPACE)) {
    toggle_reset();
  }
  /* Esc / Ctrl+Esc / Sesc — symmetric.
   * Esc mirrors the thumb esc combo: it exits whatever toggle layer is active
   * (toggle_disable) *and* sends Esc. tap_code(KC_ESC) alone would not — it
   * goes through register_code, which writes the report directly and never
   * re-enters process_record, so the KC_ESC->toggle_disable path never fires. */
  if (leader_sequence_one_key(KC_N)) {
    toggle_disable();
    tap_code(KC_ESC);
  }
  if (leader_sequence_one_key(KC_S)) {
    toggle_disable();
    tap_code(KC_ESC);
  }
  if (leader_sequence_one_key(KC_H)) {
    tap_code16(LCTL(KC_ESC));
  }
  if (leader_sequence_one_key(KC_W)) {
    tap_code16(LCTL(KC_ESC));
  }
  if (leader_sequence_one_key(KC_F)) {
    toggle_enable(L_FKEYS_SYS);
  }
  if (leader_sequence_one_key(KC_M)) {
    toggle_enable(L_MOUSE);
  }
  /* Currency signs (money): leader,m,<l|r|e> -> ₺ ₽ € via the active backend.
   * Two-key sequences, so they coexist with leader,m (mouse-layer toggle, one
   * key). */
  if (leader_sequence_two_keys(KC_M, KC_L)) { ru_emit_glyph("$l", 0x20BA); } // ₺ lira  (TRY)
  if (leader_sequence_two_keys(KC_M, KC_R)) { ru_emit_glyph("$r", 0x20BD); } // ₽ ruble (RUB)
  if (leader_sequence_two_keys(KC_M, KC_E)) { ru_emit_glyph("$e", 0x20AC); } // € euro  (EUR)
  if (leader_sequence_one_key(KC_T)) {
    toggle_enable(L_NUM_NAV);
  }

  /* Text editing */
  if (leader_sequence_two_keys(KC_D, KC_A)) { // Delete All
    tap_code16(LCTL(KC_A));
    tap_code16(KC_DEL);
  }
  if (leader_sequence_two_keys(KC_D, KC_U)) { // Like ctrl-u
    tap_code16(LSFT(KC_HOME));
    tap_code16(KC_DEL);
  }
  if (leader_sequence_two_keys(KC_D, KC_W)) { // Delete Word
    tap_code16(LCTL(KC_BSPC));
  }

  /* Kitty */
  if (leader_sequence_one_key(KC_K)) {
    tap_code16(LGUI(KC_T));
  }

  /* Print Screen */
  if (leader_sequence_one_key(KC_P)) {
    tap_code(KC_PSCR);
  }

  /* UCIS emoji — disabled (module conflict with unicodemap) */
}

/* Mouse: bisect mode — binary-search absolute positioning via the digitizer.
 * The box lives here; layer_state_set_user arms/releases the digitizer when
 * entering/leaving L_MOUSE_BISECT. */
static bisect_box_t bi_box;

static void bisect_move(void) {
  digitizer_set_position(bisect_cx(&bi_box), bisect_cy(&bi_box));
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  /* Russian: the RU_ and U_ unicode_map keys are emitted in userspace by the
   * active backend (compose or vim) here, and consumed before QMK's hex path. */
  if (ru_unicode_process(keycode, record)) {
    return false;
  }
  switch (keycode) {
    case KC_ESC:
      if (record->event.pressed) {
        toggle_disable(); // exits any active toggle; restores En in vim normal mode
      }
      return true;
    case KK_RIGHT_ARROW:
      if (record->event.pressed) {
        SEND_STRING("->");
      }
      return false;
    case KK_FAT_RIGHT_ARROW:
      if (record->event.pressed) {
        SEND_STRING("=>");
      }
      return false;
    case KK_LANGLE:
    case KK_RANGLE:
      if (record->event.pressed) {
        /* One-shot shift is in use, so plain get_mods() would miss it — OR in
         * the pending one-shot mods too. */
        uint8_t mods = get_mods() | get_oneshot_mods();
        if (mods & MOD_MASK_SHIFT) {
          /* Shifted: emit the guillemet via the active backend (it strips shift
           * itself). Compose uses the private code; vim emits the codepoint. */
          ru_emit_glyph(keycode == KK_LANGLE ? "q[" : "q]",
                        keycode == KK_LANGLE ? 0x00AB : 0x00BB); // « »
        } else {
          tap_code16(keycode == KK_LANGLE ? KC_LABK : KC_RABK); // < >
        }
      }
      return false;

    /* Mouse mode switch */
    case KK_MM_POLAR:
      if (record->event.pressed) { toggle_enable(L_MOUSE); }
      return false;
    case KK_MM_BISECT:
      if (record->event.pressed) { toggle_enable(L_MOUSE_BISECT); }
      return false;

    /* Bisect: halve the box and re-center the pointer; click holds the tip. */
    case KK_BI_L:
      if (record->event.pressed) { bisect_left(&bi_box);  bisect_move(); }
      return false;
    case KK_BI_R:
      if (record->event.pressed) { bisect_right(&bi_box); bisect_move(); }
      return false;
    case KK_BI_U:
      if (record->event.pressed) { bisect_up(&bi_box);    bisect_move(); }
      return false;
    case KK_BI_D:
      if (record->event.pressed) { bisect_down(&bi_box);  bisect_move(); }
      return false;
    case KK_BI_RESET:
      if (record->event.pressed) { bisect_reset(&bi_box); bisect_move(); }
      return false;
    case KK_BI_CLICK:
      if (record->event.pressed) { digitizer_tip_switch_on(); }
      else                       { digitizer_tip_switch_off(); }
      return false;

    default:
      break;
  }

  return true;
}

/* Arm the digitizer while in bisect mode; release it on the way out (however
 * the layer is left — mode switch, Esc, or leader,space). */
layer_state_t layer_state_set_user(layer_state_t state) {
  static bool bisect_was_on = false;
  bool bisect_on = layer_state_cmp(state, L_MOUSE_BISECT);
  if (bisect_on && !bisect_was_on) {
    digitizer_in_range_on();
    bisect_reset(&bi_box);
    bisect_move();
  } else if (!bisect_on && bisect_was_on) {
    digitizer_tip_switch_off();  // release a held click before leaving
    digitizer_in_range_off();
  }
  bisect_was_on = bisect_on;
  return state;
}

/**
 * The keymap
 *
 * * Design:
 *
 *  ** Base layer
 *  It's the [BOO layout](https://ballerboo.github.io/boolayout/), which is Dvorak modified for more rollover.
 *
 *  ** Modifiers
 *  Vertical combo mods: a home-row key plus a key just above it. See the Combos
 *  block above for why same-column combos are comfortable and misfire-free on choc.
 *
 *  ** Thumbs
 *  On the Cantor the middle thumb is the most comfortable key, so the two most
 *  used thumb actions live there: Space (right middle) and Shift (left middle).
 *  Esc is both middle thumbs at once — the strongest pair. The right inner thumb
 *  is a SYMBOLS layer-tap (tap = Enter, hold = SYM); Enter lives there since it is
 *  far less frequent than Space.
 *
 *  Punctuation on the SYM layer lives mostly on the RIGHT hand — the same hand as
 *  the right-thumb Space — so a symbol->Space sequence is a same-hand roll: tap the
 *  left-thumb one-shot SYM, type the symbol with the right hand, then roll into
 *  Space. (`,` `.` stay on the LEFT of the base layer.)
 *
 *  ** Unicode Input
 *  *** Why?
 *  When using non-qwerty layout, then switching language in OS is non-trivial. Normally there is just
 *  a keymap from qwerty to a language. That is, each latin letter is mapped to a letter of another
 *  alphabet: q->й, w->ц. But with non-qwerty layout the map is different.
 *  To mitigate this, one would implement a keymap for their case. But what if we have two keyboards
 *  attached? Let's say, I prefer qwerty on the notebook keyboard, and Boo layout on QMK keyboard.
 *  Since an OS can't make difference between keyboards, it can't know which keymap to apply when
 *  receiving a keypress from any of the keyboards.
 *  To solve this:
 *  - with notebook keeb I just switch language with `win+space`,
 *  - and the qmk keyboard uses its own Russian layer.
 *
 *  *** How
 *  A separate layer with Russian letters.
 *  https://docs.qmk.fm/features/unicode#input-subsystems
 *
 *  Two backends, selected by leader (see the unicode_ru module): compose mode
 *  (default, rolling-safe, host-wide) and vim mode (leader,v). Vim mode emits
 *  vim's native `i_CTRL-V U <hex>`, so it types Cyrillic inside vim/neovim with
 *  no host compose setup. Both are emitted in userspace; the firmware no longer
 *  needs the out-of-tree UNICODE_MODE_VIM patch (QMK PR #25188).
 *
 *
 * * KEY COMFORT SCORES  —  layout-wide ergonomic weights, higher = easier.
 *
 * Scale 0-9 (adjust to taste). Combos resolve from layer 0, so one map serves
 * every layer. Drives frequency-first symbol placement: rank symbols by how
 * often you type them, rank free keys by score, then match highest-to-highest.
 *
 *    pinky2 pinky  ring   mid  index  inner | inner  index   mid   ring  pinky pinky2
 *       1     4     6      8     6      2   |   2      6      8      6     4     3
 *       0     5     7      9     9      3   |   3      9      9      7     5     3
 *       0     1     4      5     6      3   |   3      6      5      4     1     1
 *                    · reserved for layer/mod keys, not symbol slots ·
 */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [L_BOO] = LAYOUT_split_3x6_3(/** BOO LAYOUT
       XX  '   ,   u   c   v                        q   f   d   l   y   /
       XX  a   o   e   s   g                        b   n   t   r   i   -
     noop  ·   x   .   w   z                        p   h   m   k   j   noop
                        __ sft SYMBOLS          ret spc LEAD
       */
           XX , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           XX ,    KC_A,    KC_O,    KC_E,   KC_S,  KC_G,     KC_B,  KC_N,  KC_T,  KC_R,  KC_I,   KC_MINUS,
       KK_NOOP,     XX,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   KK_NOOP,

                           QK_LEAD , KK_SHIFT , KK_SYMBO,       KK_RET , KC_SPACE, QK_LEAD
  ),

  /**
   * Russian layer.
   *
   * Activate and deactivate with leader seqs.
   */
  [L_RUSSIAN] = LAYOUT_split_3x6_3(/** Russian layer ```
       ё   й   ц   у   к   е                        н   г   ш   щ   з   х
       __  ф   ы   в   а   п                        р   о   л   д   ж   э
       __  я   ч   с   м   и                        т   ь   б   ю   .   ъ
                            __  __  __    __  __  __
       ```
       */
         RU_YO,   RU_Y,    RU_TS,    RU_U,   RU_K,  RU_E,     RU_N,  RU_G,   RU_SH, RU_SHCH,RU_Z,   RU_H,
           __ ,   RU_F,    RU_YERU,  RU_V,   RU_A,  RU_P,     RU_R,  RU_O,   RU_L,  RU_D,   RU_ZH,  RU_EE,
           __ ,   RU_YA,   RU_CH,    RU_S,   RU_M,  RU_I,     RU_T,  RU_SOFT,RU_B,  RU_YU,  RU_DOT, RU_HARD,
                                     __ ,    __ ,   __ ,       __ ,   __ ,   __
  ),

  /**
   * Symbol layer — frequency-first, punctuation on the RIGHT hand.
   *
   * Reach SYM two ways: KK_SYMBO (left thumb) — tap for a one-shot (next key from
   * SYM, then it reverts) or hold to keep it active — and the RET layer-tap (right
   * inner thumb) — hold for SYM, tap for Enter. Shiftless: every symbol has its own
   * key, no Shift needed.
   *
   * Punctuation lives on the RIGHT hand so a symbol rolls into the right-thumb Space
   * on the *same* hand. Best paired with the left-thumb one-shot: tap KK_SYMBO, type
   * the symbol with the right hand, then roll into Space.
   *
   *         pinky2 pinky ring  mid  index inner | inner index  mid  ring  pinky pinky2
   *  top      ·     `    —     $     ^     ·   |   ·     !     *     |     &     /
   *  home     ·          ~     @     #     ·   |   ·     ?     :     +     %     -
   *  bot      ·     ·    №     §     \     ⌦   |   ⌫     =     ;     ·     ·     ·
   *
   * Notes / mnemonics (do not "fix" these):
   * - `? !` take the two strongest right keys (index home `?` / top `!`) — the
   *   sentence-enders are the symbols most reliably followed by Space.
   * - `=` sits below them (index bot). `:` (mid home) -> `=` (index bot) is still a
   *   real cross-row roll, so there is no `:=` combo.
   * - `: ;` stack on the right-mid column (home `:` / bot `;`).
   * - `| &` pair on the right top row (ring `|` / pinky `&`).
   * - `/` and `-` are pinned to their base right-outer positions, so the key is
   *   identical on every layer (and `_` stays Shift+-). On SYM they mainly serve
   *   Russian, where the base latin keys are shadowed by Cyrillic.
   * - the LEFT hand holds the token-hugging specials `# @ $ ~ ^ \` and backtick,
   *   plus the unicode `—` `№` `§` (RU_MDASH / RU_NUM / RU_SECT).
   * - backtick (left-pinky top) sits where base `'` (and Shift+' = `) already live —
   *   one home for the quote/backtick key across base and SYM.
   * - `⌫` backspace (right inner) and `⌦` delete (left inner).
   *
   * Not on SYM (all global, so they work while Russian is active):
   * - `"`  — the G+V combo (KC_DQUO).
   * - `« »` — the angle combos via KK_LANGLE / KK_RANGLE (unshifted = `<` `>`).
   * - `?` is suppressed from base Shift+/, so SYM is its one canonical home.
   *
   * Other combos still work here: `=>` `->`, brackets, mods.
   */
  [L_SYMBOLS] = LAYOUT_split_3x6_3(/*
       ·  `   —   $   ^   ·                        ·   !   *   |   &   /
       ·      ~   @   #   ·                        ·   :   ?   +   %   -
       ·  ·   №   §   \   ⌦                        ⌫   =   ;           ·
                            __  __  SYM   __  __  __
       */
        XX,  KC_GRV, RU_MDASH,   KC_DLR, KC_CIRC,      XX,            XX,  KC_EXLM,  KC_ASTR,  KC_PIPE, KC_AMPR,  KC_SLASH,
        XX,      XX,  KC_TILD,    KC_AT, KC_HASH,      XX,            XX,  KC_COLN,  KC_QUES,  KC_PLUS, KC_PERC,  KC_MINUS,
        XX,      XX,   RU_NUM,  RU_SECT, KC_BSLS,  KC_DEL,       KC_BSPC,   KC_EQL,  KC_SCLN,       XX,      XX,  XX,
                                      __ ,  __ , KK_SYMBO,         __ ,  __ ,  __
  ),

  /**
   * Layer for numbers and navigation.
   *
   * Activated by holding home-row of the middle fingers.
   * Extra activation: middle thumb on the right hand. This way you can use ↑,↓ with a single hand.
   *
   * Basic idea is clean: numbers on the left, and navigation on the right.
   *
   * ** Here are a bit less obvious decisions
   * - `0` sits on the home-row place (left of `4`), which is easier to reach than its
   *   logical spot before `1` (that slot holds `,`).
   * - `/` `:` `.` is to type `05/06/1970` `05:50` `3.1415`
   * - KC_GRV (```) is to switch windows in gnome:
   *   - with the left hand hold alt+L_NUM_NAV (howe-row of the ring and middle fingers)
   *   - with the right hand tap KC_GRV
   *   - while still holding alt and L_NUM_NAV, you can tap left and right arrows with the right hand
   */
  [L_NUM_NAV] = LAYOUT_split_3x6_3(/*
       __  __  7   8   9   /                        `   pg↑ ↑   pg↓ __  __
       __  0   4   5   6   :                        ⇤-  ←   ⏎   →   -⇥  __
       __  ,   1   2   3   .                        __  ⮀   ↓   __  __  __
                            __  __  __    __  __  __
       */
       XX,KC_QUOT,  KC_7,  KC_8,  KC_9, KC_SLASH,      KC_GRV,  KC_PGUP,  KC_UP,    KC_PGDN, XX,      __,
       XX,   KC_0,  KC_4,  KC_5,  KC_6, KC_COLN,       KC_HOME, KC_LEFT,  KC_ENTER, KC_RGHT, KC_END,  __,
       XX,KC_COMM,  KC_1,  KC_2,  KC_3,  KC_DOT,       XX,      KC_TAB,   KC_DOWN,  XX,      XX,      XX,
                            __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),
  /**
   * Layer for F keys and multimedia buttons.
   *
   * Sticky via Leader,f (exit with Esc or Leader,space). Also reachable as a
   * one-shot via the Q+B combo.
   */
  [L_FKEYS_SYS] = LAYOUT_split_3x6_3(/*
        __ F12  F7  F8  F9  __                       __  br↑ vl↑ __  DBG __
        __ F11  F4  F5  F6  __                       __  __  __  __  __  __
       bot F10  F1  F2  F3  __                       __  br↓ vl↓ vl0 __  bot
                             __  __  __     __  __  __
       */
        XX,  KC_F12,  KC_F7,  KC_F8,  KC_F9,     XX,       XX, KC_BRIU,  KC_VOLU,       XX,  DB_TOGG, XX,
        XX,  KC_F11,  KC_F4,  KC_F5,  KC_F6,     XX,       XX,      XX,       XX,       XX,       XX, __,
   QK_BOOT,  KC_F10,  KC_F1,  KC_F2,  KC_F3,     XX,       XX, KC_BRID,  KC_VOLD,  KC_MUTE,  XX, QK_BOOT,
                                __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),

  /**
   * Mouse layer — POLAR mode (Orbital Mouse, getreuer/orbital_mouse). DEFAULT.
   *
   * Reached via Leader,m, which starts here. Top-row left keys switch mode:
   *   `,` = polar (this layer)   `c` = bisect.
   * Exit any mouse mode with Esc (shift+space combo) or Leader,space.
   *
   * Polar/heading control: fwd/bwd move the pointer forward/backward along a
   * heading; ←/→ steer that heading; slo/fst change speed while held. Left hand
   * can apply modifiers (shift+click, ctrl+wheelup).
   */
  [L_MOUSE] = LAYOUT_split_3x6_3(/*
        __  __  pol __  bis __                       __  w↑  fwd w↓  __  __
        __  __  slo __  fst __                       __  ←   b1  →   b2  __
        __  __  __  __  __  __                       __  b3  bwd __  __  __
                             __  __  __     __  __  __
       */
        XX, XX, KK_MM_POLAR,      XX, KK_MM_BISECT, XX,   XX, OM_W_U, OM_U   , OM_W_D,      XX, XX,
        XX, XX,     OM_SLOW,      XX,      OM_FAST, XX,   XX, OM_L  , OM_BTN1, OM_R   , OM_BTN2, __,
        XX, XX,          XX,      XX,           XX, XX,   XX, OM_BTN3, OM_D   ,      XX,      XX, XX,

                                   __ ,    __ ,   __ ,         __ ,  __ ,  __
  ),

  /**
   * Mouse layer — BISECT mode (digitizer binary search). Opt-in via the `c` key.
   *
   * Non-default: Linux does not bind the digitizer (libwacom has no entry for
   * usb:feed:0000), so the pointer does not move on this host. Kept because the
   * firmware side is proven correct — it needs only a host-side quirk.
   *
   * Pointer starts at screen center. The right-hand arrow cross halves the
   * screen and re-centers each press:  ↑=up  <-=left  ->=right  ↓=down.
   * clk = click (holds the digitizer tip; hold to drag).  rst = reset to full
   * screen.  `,` switches to polar. Leaving bisect releases the digitizer
   * (layer_state_set_user).
   */
  [L_MOUSE_BISECT] = LAYOUT_split_3x6_3(/*
        __  __  pol __  bis __                       __  __  ↑    __   __  __
        __  __  __  __  __  __                       __  <-  clk  ->   rst __
        __  __  __  __  __  __                       __  __  ↓    __   __  __
                             __  __  __     __  __  __
       */
        XX, XX, KK_MM_POLAR, XX, KK_MM_BISECT, XX,   XX,      XX,     KK_BI_U,      XX,          XX, XX,
        XX, XX,          XX, XX,           XX, XX,   XX, KK_BI_L, KK_BI_CLICK, KK_BI_R, KK_BI_RESET, __,
        XX, XX,          XX, XX,           XX, XX,   XX,      XX,     KK_BI_D,      XX,          XX, XX,

                                   __ ,    __ ,   __ ,         __ ,  __ ,  __
  ),
};
