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
/* NOTE nice spare combos:
 *      right: space + a home row key
 */

#include QMK_KEYBOARD_H

/* Key aliases */
#define __ KC_TRNS
#define XX KC_NO

/* */
enum my_keycodes {
  SWITCH_LANG = SAFE_RANGE,
  XX_FAKE,
  KK_GO_DECLARATION,
  KK_RIGHT_ARROW,
  KK_FAT_RIGHT_ARROW,
  KK_NOT_EQUAL,
  SMTD_KEYCODES_BEGIN,
  // boo home-row mods
  GUI_A,
  ALT_O,
  LT3_E,
  CTL_S,
  CTL_N,
  LT3_T,
  ALT_R,
  GUI_I,
  // ru home-row mods // RH = russian homerow
  RH_F,
  RH_Y,
  RH_V,
  RH_A,
  RH_O,
  RH_L,
  RH_D,
  RH_Z,
  // thumb keys
  KK_RU,
  KK_SHIFT,
  KK_ENTER,
  KK_SPACE,
  KK_EN,
  SMTD_KEYCODES_END,
};

#include "sm_td.h"
#include "unicode.c"

/* Layer names */
enum my_layer_names {
  L_BOO,
  L_RUSSIAN,
  L_SYMBOLS,
  L_NUM_NAV,
  L_FKEYS,
  L_MOUSE,
  L_RGB,
};

#define OSL_SYM OSL(L_SYMBOLS)

/* Key overrides */

// Suppress < and > on the main layer
const key_override_t labk_override = ko_make_with_layers(MOD_MASK_SHIFT, KC_COMMA, KC_NO, 1<<L_BOO);
const key_override_t rabk_override = ko_make_with_layers(MOD_MASK_SHIFT, KC_DOT,   KC_NO, 1<<L_BOO);

const key_override_t *key_overrides[] = {
  &labk_override,
  &rabk_override,
};

/* Combos */

#define COMBO_ONLY_FROM_LAYER 0

/* Hit both middle thumb keys for esc. */
const uint16_t PROGMEM esc_combo[]     = {KK_SHIFT, KK_SPACE, COMBO_END};
const uint16_t PROGMEM ctl_esc_combo[] = {KK_SHIFT, CTL_S,    COMBO_END};
const uint16_t PROGMEM alt_esc_combo[] = {KK_SHIFT, ALT_O,    COMBO_END};
/* Two outer bottom keys on a single half to get into bootloader. */
const uint16_t PROGMEM boot_combo_left[]  = {XX_FAKE,  OSL_SYM, COMBO_END};
const uint16_t PROGMEM boot_combo_right[] = {KK_ENTER, XX_FAKE, COMBO_END};
/* On each half: the outermost bottom pinky key + the middle thumb key to reboot the keyboard. */
const uint16_t PROGMEM reset_combo_left[]  = {XX_FAKE,  KK_SHIFT, COMBO_END};
const uint16_t PROGMEM reset_combo_right[] = {KK_SPACE, XX_FAKE, COMBO_END};
/* := -> => != */
const uint16_t PROGMEM go_declaration_combo[]  = {KC_H, GUI_I, COMBO_END};
const uint16_t PROGMEM right_arrow_combo[]     = {KC_M, GUI_I, COMBO_END};
const uint16_t PROGMEM fat_right_arrow_combo[] = {KC_H, KC_M, COMBO_END};
const uint16_t PROGMEM not_equal_combo[]       = {KC_H, KC_K, COMBO_END};

combo_t key_combos[] = {
  COMBO(esc_combo, KC_ESC),
  COMBO(ctl_esc_combo, LCTL(KC_ESC)),
  COMBO(alt_esc_combo, LALT(KC_ESC)),

  COMBO(boot_combo_left,  QK_BOOT),
  COMBO(boot_combo_right, QK_BOOT),

  COMBO(reset_combo_left,  QK_REBOOT),
  COMBO(reset_combo_right, QK_REBOOT),

  COMBO(go_declaration_combo, KK_GO_DECLARATION),
  COMBO(right_arrow_combo, KK_RIGHT_ARROW),
  COMBO(fat_right_arrow_combo, KK_FAT_RIGHT_ARROW),
  COMBO(not_equal_combo, KK_NOT_EQUAL),
};

/* */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (!process_smtd(keycode, record)) {
    return false;
  }

  switch (keycode) {
    case KK_GO_DECLARATION:
      if (record->event.pressed) {
        SEND_STRING(":=");
      }
      return false;
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
    case KK_NOT_EQUAL:
      if (record->event.pressed) {
        SEND_STRING("!=");
      }
      return false;
    default:
      return true; // Process all other keycodes normally
  }
}

void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
  switch (keycode) {
    // Home-row mods (Boo).
    SMTD_MT(GUI_A, KC_A, KC_LEFT_GUI)
    SMTD_MT(ALT_O, KC_O, KC_LEFT_ALT)
    SMTD_LT(LT3_E, KC_E, L_NUM_NAV)
    SMTD_MT(CTL_S, KC_S, KC_LEFT_CTRL)
    //
    SMTD_MT(CTL_N, KC_N, KC_RIGHT_CTRL)
    SMTD_LT(LT3_T, KC_T, L_NUM_NAV)
    SMTD_MT(ALT_R, KC_R, KC_RIGHT_ALT)
    SMTD_MT(GUI_I, KC_I, KC_RIGHT_GUI)

    /* RU home row */
    SM_MU(RH_F, CYR_LC_F, KC_LEFT_GUI)
    SM_MU(RH_Y, CYR_LC_YERU, KC_LEFT_ALT)
    // SM_LU(RH_V, CYR_LC_V, L_NUM_NAV)
    SM_MU(RH_A, CYR_LC_A, KC_LEFT_CTRL)
    SM_MU(RH_O, CYR_LC_O, KC_LEFT_CTRL)
    // SM_LU(RH_L, CYR_LC_L, L_NUM_NAV)
    SM_MU(RH_D, CYR_LC_D, KC_LEFT_ALT)
    SM_MU(RH_Z, CYR_LC_ZH, KC_LEFT_GUI)

    // Thumb keys
    SMTD_MT(KK_SHIFT, KC_DOT  , KC_LEFT_SHIFT)
    SMTD_LT(KK_ENTER, KC_ENTER, L_SYMBOLS)
    SMTD_LT(KK_SPACE, KC_SPACE, L_SYMBOLS)
    /* tap = english, hold = mouse */
    case KK_EN:
    {
      switch (action) {
        case SMTD_ACTION_TOUCH:
          break;

        case SMTD_ACTION_TAP:
          layer_off(L_RUSSIAN);
          break;

        case SMTD_ACTION_HOLD:
          switch (tap_count) {
            case 0:
              layer_on(L_MOUSE);
              break;
            default:
              break;
          }
          break;

        case SMTD_ACTION_RELEASE:
          switch (tap_count) {
            case 0:
              layer_off(L_MOUSE);
              break;
            default:
              break;
          }
          break;
      }
      break;
    }
    /* tap = ru, hold = fkeys */
    case KK_RU:
    {
      switch (action) {
        case SMTD_ACTION_TOUCH:
          break;

        case SMTD_ACTION_TAP:
          layer_on(L_RUSSIAN);
          break;

        case SMTD_ACTION_HOLD:
          switch (tap_count) {
            case 0:
              layer_on(L_FKEYS);
              break;
            default:
              break;
          }
          break;

        case SMTD_ACTION_RELEASE:
          switch (tap_count) {
            case 0:
              layer_off(L_FKEYS);
              break;
            default:
              break;
          }
          break;
      }
      break;
    }
  }
}

/* The keymap */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [L_BOO] = LAYOUT_split_3x6_3(/* BOO LAYOUT
       --- '   ,   u   c   v                        q   f   d   l   y   /
       --- a   o   e   s   g                        b   n   t   r   i   -
       ---     x   .   w   z                        p   h   m   k   j   ---
                        EN sft SYM              ret spc RU
       */
           __ , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           __ ,   GUI_A,   ALT_O,   LT3_E,  CTL_S,  KC_G,     KC_B,  CTL_N, LT3_T, ALT_R, GUI_I,  KC_MINUS,
       XX_FAKE,      XX,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   XX_FAKE,

                              KK_EN , KK_SHIFT , OSL_SYM ,     KK_ENTER , KK_SPACE, KK_RU          ),

  [L_RUSSIAN] = LAYOUT_split_3x6_3(
      // ёйцуке нгшщзх
      //  фывап ролджэ
      //  ячсми тьбю.
         RU_YO,   RU_Y,    RU_TS,    RU_U,   RU_K,  RU_E,     RU_N,  RU_G,   RU_SH, RU_SHCH,RU_Z,   RU_H,
           __ ,   RH_F,    RH_Y,     RH_V,   RH_A,  RU_P,     RU_R,  RH_O,   RH_L,  RH_D,   RH_Z,   RU_EE,
           __ ,   RU_YA,   RU_CH,    RU_S,   RU_M,  RU_I,     RU_T,  RU_SOFT,RU_B,  RU_YU,  KC_DOT, RU_HARD,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_SYMBOLS] = LAYOUT_split_3x6_3(/*
        __  `   __  __  {   __                       __  }   +   |   ~   __
        __  ;   /   (   [   __                       __  ]   )   \   :   __
        __  __  __  <   __  __                       __  =   >   !   __  __
                           ___  ___ ___     ___ ___  ___
       */
        XX,  KC_GRV,        XX,       XX, KC_LCBR,      XX,       XX,      KC_RCBR,  KC_PLUS,  KC_PIPE, KC_TILD, XX,
        XX, KC_SCLN,  KC_SLASH,  KC_LPRN, KC_LBRC,      XX,       XX,      KC_RBRC,  KC_RPRN,  KC_BSLS, KC_COLN, KC_MINUS,
   QK_BOOT,      XX,        XX,  KC_LABK,      XX,  KC_DEL,       KC_BSPC,  KC_EQL,  KC_RABK,  KC_EXLM, XX,      QK_BOOT,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_NUM_NAV] = LAYOUT_split_3x6_3(/*
        __  __  7   8   9   __                       hom pg↑ ↑   pg↓ end __
        __  0   4   5   6   :                        bs  ←  ret  →   __  __
        __  0   1   2   3   .                        __  tab ↓   __  __  __
                           ___  ___ ___     ___ ___  ___
       */
        XX,      XX,      KC_7,     KC_8,    KC_9,      XX,       KC_HOME, KC_PGUP,  KC_UP,    KC_PGDN, KC_END,  XX,
        XX,    KC_0,      KC_4,     KC_5,    KC_6, KC_COLN,       XX,      KC_LEFT,  KC_ENTER, KC_RGHT, XX,      XX,
        XX,    KC_0,      KC_1,     KC_2,    KC_3,  KC_DOT,       XX,      KC_TAB,   KC_DOWN,  XX,      XX,      XX,

                         __     ,         __  ,    __  ,       __  ,    __  ,    __                      ),

  [L_FKEYS] = LAYOUT_split_3x6_3(/*
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
