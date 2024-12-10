/* A layout for the Dactyl Manuform 5x6_5 Keyboard */

// TODO make a shared layout for cantor and dactyl
//      - transform dactyl keymap to a cantor's
//        - make a text transform program
//        - which removes keys not presented on cantor
//          - from the comment
//          - from the code
//        - which is covered by tests
// TODO cleanup readme
//      nix flake run
//      guide for initial flash for left and right
// TODO generate clean schemes from layer definitions.
//      Now they are good, but manual and prone to be outdated.
// TODO autoformat layer definitions

#include QMK_KEYBOARD_H

/* Home-row mods */

// Home-row mods (Boo).
//
#define GUI_A LGUI_T(KC_A)
#define ALT_O LALT_T(KC_O)
#define LT3_E LT(L_NUM_NAV, KC_E)
#define CTL_S LCTL_T(KC_S)
//
#define CTL_N RCTL_T(KC_N)
#define LT3_T LT(L_NUM_NAV, KC_T)
#define ALT_R LALT_T(KC_R)
#define GUI_I RGUI_T(KC_I)

// Home-row mods (qwerty).
//
#define GUI_A LGUI_T(KC_A)
#define ALT_S LALT_T(KC_S)
#define LT3_D LT(L_NUM_NAV, KC_D)
#define CTL_F LCTL_T(KC_F)
//
#define CTL_J RCTL_T(KC_J)
#define LT3_K LT(L_NUM_NAV, KC_K)
#define ALT_L LALT_T(KC_L)
#define GUI_SCLN RGUI_T(KC_SCLN)


/* Key aliases */

#define __ KC_TRNS
#define XX KC_NO
#define MY_LSFT OSM(MOD_LSFT)

/* */
enum my_keycodes {
  SWITCH_LANG = SAFE_RANGE,
  XX_FAKE,
};

/* Layer names */
enum my_layer_names {
  L_BOO,
  L_QWERTY,
  L_SYMBOLS,
  L_NUM_NAV,
  L_FKEYS_SYSTEM,
  L_MOUSE,
  L_RGB,
};

#define TG_QWER TG(L_QWERTY)
#define OSL_SYM OSL(L_SYMBOLS)
#define MO_SYS  MO(L_FKEYS_SYSTEM)
#define MO_RGB  MO(L_RGB)

/* Tap dance */

enum {
  TD_SYM_MOUSE, /* Hold for SYM, tap-hold for MOUSE */
};

void td_sym_mouse_on_tap(tap_dance_state_t *state, void *user_data) {
};

void td_sym_mouse_on_finish(tap_dance_state_t *state, void *user_data) {
  if (!state->pressed) {
    switch (state->count) {
      case 1: caps_word_on(); break;
    };
  } else {
    switch (state->count) {
      case 1: layer_on(L_SYMBOLS); break;
      case 2: layer_on(L_MOUSE); break;
    };
  };
};

void td_sym_mouse_on_reset(tap_dance_state_t *state, void *user_data) {
  layer_off(L_SYMBOLS);
  layer_off(L_MOUSE);
};

tap_dance_action_t tap_dance_actions[] = {
  [TD_SYM_MOUSE] = ACTION_TAP_DANCE_FN_ADVANCED(td_sym_mouse_on_tap,
                                                td_sym_mouse_on_finish,
                                                td_sym_mouse_on_reset),
};

/* DK is for "dance key" */
#define DK_SYMO TD(TD_SYM_MOUSE)


/* Combos */

#define COMBO_ONLY_FROM_LAYER 0

/* Hit both middle thumb keys for esc. */
const uint16_t PROGMEM esc_combo[]  = {MY_LSFT, KC_SPACE, COMBO_END};
/* Two outer bottom keys on a single half to get into bootloader. */
const uint16_t PROGMEM boot_combo_left[]  = {XX_FAKE,  DK_SYMO, COMBO_END};
const uint16_t PROGMEM boot_combo_right[] = {KC_ENTER, XX_FAKE, COMBO_END};
/* On a single half: the outermost bottom pinky key + the middle thumb key to reboot the keyboard. */
const uint16_t PROGMEM reset_combo_left[]  = {XX_FAKE,  MY_LSFT, COMBO_END};
const uint16_t PROGMEM reset_combo_right[] = {KC_SPACE, XX_FAKE, COMBO_END};

combo_t key_combos[] = {
  COMBO(esc_combo, KC_ESC),
  COMBO(boot_combo_left,  QK_BOOT),
  COMBO(boot_combo_right, QK_BOOT),
  COMBO(reset_combo_left,  QK_REBOOT),
  COMBO(reset_combo_right, QK_REBOOT),
};

/* */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case SWITCH_LANG:
      if (record->event.pressed) {
        // win+space
        register_code(KC_LGUI);
        tap_code16(KC_SPACE);
        unregister_code(KC_LGUI);
        //
        layer_invert(L_QWERTY);
      }
      return false; // Skip all further processing of this key
    default:
      return true; // Process all other keycodes normally
  }
}

/* The keymap */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [L_BOO] = LAYOUT_split_3x6_3(/* BOO LAYOUT
       --- '   ,   u   c   v                        q   f   d   l   y   /
       --- a   o   e   s   g                        b   n   t   r   i   -
       --- ;   x   .   w   z                        p   h   m   k   j   ---
                       SYS sft SYMO             ret spc ---
       */
           __ , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           __ ,   GUI_A,   ALT_O,   LT3_E,  CTL_S,  KC_G,     KC_B,  CTL_N, LT3_T, ALT_R, GUI_I,  KC_MINUS,
       XX_FAKE, KC_SCLN,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   XX_FAKE,

                             MO_SYS , MY_LSFT , DK_SYMO ,     KC_ENTER , KC_SPC,  SWITCH_LANG            ),

  [L_QWERTY] = LAYOUT_split_3x6_3(
        KC_GRV,    KC_Q,    KC_W,    KC_E,   KC_R,  KC_T,     KC_Y,  KC_U,  KC_I,    KC_O,   KC_P,     KC_LBRC,
           __ ,   GUI_A,   ALT_S,   LT3_D,  CTL_F,  KC_G,     KC_H,  CTL_J, LT3_K,   ALT_L,  GUI_SCLN, KC_QUOTE,
           __ ,    KC_Z,    KC_X,    KC_C,   KC_V,  KC_B,     KC_N,  KC_M,  KC_COMM, KC_DOT, KC_SLSH,  KC_RBRC,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_SYMBOLS] = LAYOUT_split_3x6_3(/*
        __  ~   __  __  {   __                       __  }   +   |   ;   __
        __  `   /   (   [   <                        >   ]   )   \   :   __
        __  __  __  <   __  __                       __  =   >   __  __  __
                           ___  ___ ___     ___ ___  ___
       */
        XX, KC_TILD,        XX,       XX, KC_LCBR,      XX,       XX,      KC_RCBR,  KC_PLUS,  KC_PIPE, KC_SCLN, XX,
        XX,  KC_GRV,  KC_SLASH,  KC_LPRN, KC_LBRC, KC_LABK,       KC_RABK, KC_RBRC,  KC_RPRN,  KC_BSLS, KC_COLN, KC_MINUS,
        XX,      XX,        XX,  KC_LABK,      XX,  KC_DEL,       KC_BSPC,  KC_EQL,  KC_RABK,  XX,      XX,      XX,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_NUM_NAV] = LAYOUT_split_3x6_3(/*
        __  __  7   8   9   scr                      hom pg↑ ↑   pg↓ end __
        __  0   4   5   6   del                      bs  ←  ret  →   __  __
        __  0   1   2   3   __                       __  tab ↓   __  __  __
                           ___  ___ ___     ___ ___  ___
       */
        XX,      XX,      KC_7,     KC_8,    KC_9, KC_PSCR,       KC_HOME, KC_PGUP,  KC_UP,    KC_PGDN, KC_END,  XX,
        XX,    KC_0,      KC_4,     KC_5,    KC_6,      XX,       XX,      KC_LEFT,  KC_ENTER, KC_RGHT, XX,      XX,
        XX,    KC_0,      KC_1,     KC_2,    KC_3,      XX,       XX,      KC_TAB,   KC_DOWN,  XX,      XX,      XX,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_FKEYS_SYSTEM] = LAYOUT_split_3x6_3(/*
        __ F11  F7  F8  F9  __                       __  br↑ vl↑ __  __  __
        __ F11  F4  F5  F6  __                       __  __  mut __  __  __
        __ F10  F1  F2  F3  __                       __  br↓ vl↓ __  __  __
                           ___  ___ ___     ___ ___  ___
       */
        XX,  KC_F12,     KC_F7,    KC_F8,   KC_F9,     XX,       XX, KC_BRIU,  KC_VOLU,       XX,      XX,      XX,
        XX,  KC_F11,     KC_F4,    KC_F5,   KC_F6,     XX,       XX,      XX,  KC_MUTE,       XX,      XX,      XX,
        XX,  KC_F10,     KC_F1,    KC_F2,   KC_F3,     XX,       XX, KC_BRID,  KC_VOLD,       XX,      XX,      XX,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_MOUSE] = LAYOUT_split_3x6_3(/*
        __ __  __  __  __  __                       __  w↑  ↑  w↓  b3  __
        __ __  alt b1  ctl __                       __  <-  b1 ->  b2  __
        __ __  __  __  __  __                       __  __  ↓  __  __  __
                          ___  ___ ___     ___ ___  ___
       */
        XX,      XX,        XX,       XX,      XX,     XX,       XX, KC_WH_U,  KC_MS_U,  KC_WH_D, KC_BTN3,      XX,
        XX,      XX,   KC_LALT,  KC_BTN1, KC_LCTL,     XX,       XX, KC_MS_L,  KC_BTN1,  KC_MS_R, KC_BTN2,      XX,
        XX,      XX,        XX,       XX,      XX,     XX,       XX,      XX,  KC_MS_D,       XX,      XX,      XX,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_RGB] = LAYOUT_split_3x6_3(/*
        __ __  hu↑ br↑ mod __                       __  __  __  __  __  __
        __ __  sa↓ tog sa↑ __                       __  __  __  __  __  __
        __ __  hu↓ br↓ m_p __                       __  __  __  __  __  __
                           ___  ___ ___     ___ ___  ___
       */
        XX,      XX,   RGB_HUI,  RGB_VAI, RGB_MOD,     XX,       XX,      XX,       XX,       XX,      XX,      XX,
        XX,      XX,   RGB_SAD,  RGB_TOG, RGB_SAI,     XX,       XX,      XX,       XX,       XX,      XX,      XX,
        XX,      XX,   RGB_HUD,  RGB_VAD, RGB_M_P,     XX,       XX,      XX,       XX,       XX,      XX,      XX,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),
 };
