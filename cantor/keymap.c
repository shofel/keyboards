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
  // qwerty home-row mods
  ALT_S,
  LT3_D,
  CTL_F,
  CTL_J,
  LT3_K,
  ALT_L,
  GUI_SCLN,
  // thumb keys
  KK_RU,
  KK_SHIFT,
  KK_ENTER,
  KK_SPACE,
  KK_EN,
  SMTD_KEYCODES_END,
};

#include "sm_td.h"

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

#define TG_RU   TG(L_RUSSIAN)
#define OSL_SYM OSL(L_SYMBOLS)
#define MO_RGB  MO(L_RGB)

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

    // Home-row mods (qwerty).
    // SMTD_MT(GUI_A, KC_A, KC_LEFT_GUI) // Duplicates boo
    SMTD_MT(ALT_S, KC_S, KC_LEFT_ALT)
    SMTD_LT(LT3_D, KC_D, L_NUM_NAV)
    SMTD_MT(CTL_F, KC_F, KC_LEFT_CTRL)
    //
    SMTD_MT(CTL_J, KC_N, KC_RIGHT_CTRL)
    SMTD_LT(LT3_K, KC_K, L_NUM_NAV)
    SMTD_MT(ALT_L, KC_L, KC_RIGHT_ALT)
    SMTD_MT(GUI_SCLN, KC_SCLN, KC_RIGHT_GUI)

    // Thumb keys
    SMTD_MT(KK_SHIFT, KC_DOT  , KC_LEFT_SHIFT)
    SMTD_LT(KK_ENTER, KC_ENTER, L_SYMBOLS)
    SMTD_LT(KK_SPACE, KC_SPACE, L_SYMBOLS)

    /* tap = english
       hold = mouse
       tap-hold = sys */
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
            case 1:
              layer_on(L_FKEYS);
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
            case 1:
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

/* Unicode */

enum unicode_names {
  CYR_LC_A,
  CYR_LC_B,
  CYR_LC_V,
  CYR_LC_G,
  CYR_LC_D,
  CYR_LC_E,
  CYR_LC_YO,
  CYR_LC_ZH,
  CYR_LC_Z,
  CYR_LC_I,
  CYR_LC_Y,
  CYR_LC_K,
  CYR_LC_L,
  CYR_LC_M,
  CYR_LC_N,
  CYR_LC_O,
  CYR_LC_P,
  CYR_LC_R,
  CYR_LC_S,
  CYR_LC_T,
  CYR_LC_U,
  CYR_LC_F,
  CYR_LC_H,
  CYR_LC_TS,
  CYR_LC_CH,
  CYR_LC_SH,
  CYR_LC_SHCH,
  CYR_LC_YERU,
  CYR_LC_SOFT,
  CYR_LC_HARD,
  CYR_LC_EE,
  CYR_LC_YU,
  CYR_LC_YA,

  CYR_UC_A,
  CYR_UC_B,
  CYR_UC_V,
  CYR_UC_G,
  CYR_UC_D,
  CYR_UC_E,
  CYR_UC_YO,
  CYR_UC_ZH,
  CYR_UC_Z,
  CYR_UC_I,
  CYR_UC_Y,
  CYR_UC_K,
  CYR_UC_L,
  CYR_UC_M,
  CYR_UC_N,
  CYR_UC_O,
  CYR_UC_P,
  CYR_UC_R,
  CYR_UC_S,
  CYR_UC_T,
  CYR_UC_U,
  CYR_UC_F,
  CYR_UC_H,
  CYR_UC_TS,
  CYR_UC_CH,
  CYR_UC_SH,
  CYR_UC_SHCH,
  CYR_UC_YERU,
  CYR_UC_SOFT,
  CYR_UC_HARD,
  CYR_UC_EE,
  CYR_UC_YU,
  CYR_UC_YA,
};

const uint32_t PROGMEM unicode_map[] = {
  // Lowercase
  [CYR_LC_A] = 0x430,
  [CYR_LC_B] = 0x431,
  [CYR_LC_V] = 0x432,
  [CYR_LC_G] = 0x433,
  [CYR_LC_D] = 0x434,
  [CYR_LC_E] = 0x435,
  [CYR_LC_YO] = 0x451,
  [CYR_LC_ZH] = 0x436,
  [CYR_LC_Z] = 0x437,
  [CYR_LC_I] = 0x438,
  [CYR_LC_Y] = 0x439,
  [CYR_LC_K] = 0x43A,
  [CYR_LC_L] = 0x43B,
  [CYR_LC_M] = 0x43C,
  [CYR_LC_N] = 0x43D,
  [CYR_LC_O] = 0x43E,
  [CYR_LC_P] = 0x43F,
  [CYR_LC_R] = 0x440,
  [CYR_LC_S] = 0x441,
  [CYR_LC_T] = 0x442,
  [CYR_LC_U] = 0x443,
  [CYR_LC_F] = 0x444,
  [CYR_LC_H] = 0x445,
  [CYR_LC_TS] = 0x446,
  [CYR_LC_CH] = 0x447,
  [CYR_LC_SH] = 0x448,
  [CYR_LC_SHCH] = 0x449,
  [CYR_LC_HARD] = 0x44A,
  [CYR_LC_YERU] = 0x44B,
  [CYR_LC_SOFT] = 0x44C,
  [CYR_LC_EE] = 0x44D,
  [CYR_LC_YU] = 0x44E,
  [CYR_LC_YA] = 0x44F,
  // Uppercase
  [CYR_UC_A] = 0x410,
  [CYR_UC_B] = 0x411,
  [CYR_UC_V] = 0x412,
  [CYR_UC_G] = 0x413,
  [CYR_UC_D] = 0x414,
  [CYR_UC_E] = 0x415,
  [CYR_UC_YO] = 0x401,
  [CYR_UC_ZH] = 0x416,
  [CYR_UC_Z] = 0x417,
  [CYR_UC_I] = 0x418,
  [CYR_UC_Y] = 0x419,
  [CYR_UC_K] = 0x41A,
  [CYR_UC_L] = 0x41B,
  [CYR_UC_M] = 0x41C,
  [CYR_UC_N] = 0x41D,
  [CYR_UC_O] = 0x41E,
  [CYR_UC_P] = 0x41F,
  [CYR_UC_R] = 0x420,
  [CYR_UC_S] = 0x421,
  [CYR_UC_T] = 0x422,
  [CYR_UC_U] = 0x423,
  [CYR_UC_F] = 0x424,
  [CYR_UC_H] = 0x425,
  [CYR_UC_TS] = 0x426,
  [CYR_UC_CH] = 0x427,
  [CYR_UC_SH] = 0x428,
  [CYR_UC_SHCH] = 0x429,
  [CYR_UC_HARD] = 0x42A,
  [CYR_UC_YERU] = 0x42B,
  [CYR_UC_SOFT] = 0x42C,
  [CYR_UC_EE] = 0x42D,
  [CYR_UC_YU] = 0x42E,
  [CYR_UC_YA] = 0x42F,
};

// Combine lower and upper case letters as a single key
#define RU_A UP(CYR_LC_A, CYR_UC_A)
#define RU_B UP(CYR_LC_B, CYR_UC_B)
#define RU_V UP(CYR_LC_V, CYR_UC_V)
#define RU_G UP(CYR_LC_G, CYR_UC_G)
#define RU_D UP(CYR_LC_D, CYR_UC_D)
#define RU_E UP(CYR_LC_E, CYR_UC_E)
#define RU_YO UP(CYR_LC_YO, CYR_UC_YO)
#define RU_ZH UP(CYR_LC_ZH, CYR_UC_ZH)
#define RU_Z UP(CYR_LC_Z, CYR_UC_Z)
#define RU_I UP(CYR_LC_I, CYR_UC_I)
#define RU_Y UP(CYR_LC_Y, CYR_UC_Y)
#define RU_K UP(CYR_LC_K, CYR_UC_K)
#define RU_L UP(CYR_LC_L, CYR_UC_L)
#define RU_M UP(CYR_LC_M, CYR_UC_M)
#define RU_N UP(CYR_LC_N, CYR_UC_N)
#define RU_O UP(CYR_LC_O, CYR_UC_O)
#define RU_P UP(CYR_LC_P, CYR_UC_P)
#define RU_R UP(CYR_LC_R, CYR_UC_R)
#define RU_S UP(CYR_LC_S, CYR_UC_S)
#define RU_T UP(CYR_LC_T, CYR_UC_T)
#define RU_U UP(CYR_LC_U, CYR_UC_U)
#define RU_F UP(CYR_LC_F, CYR_UC_F)
#define RU_H UP(CYR_LC_H, CYR_UC_H)
#define RU_TS UP(CYR_LC_TS, CYR_UC_TS)
#define RU_CH UP(CYR_LC_CH, CYR_UC_CH)
#define RU_SH UP(CYR_LC_SH, CYR_UC_SH)
#define RU_SHCH UP(CYR_LC_SHCH, CYR_UC_SHCH)
#define RU_SOFT UP(CYR_LC_SOFT, CYR_UC_SOFT)
#define RU_YERU UP(CYR_LC_YERU, CYR_UC_YERU)
#define RU_HARD UP(CYR_LC_HARD, CYR_UC_HARD)
#define RU_EE UP(CYR_LC_EE, CYR_UC_EE)
#define RU_YU UP(CYR_LC_YU, CYR_UC_YU)
#define RU_YA UP(CYR_LC_YA, CYR_UC_YA)


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

                              KK_EN , KK_SHIFT , OSL_SYM ,     KK_ENTER , KK_SPACE, TG_RU          ),

  [L_RUSSIAN] = LAYOUT_split_3x6_3(
      // ёйцуке нгшщзх
      //  фывап ролджэ
      //  ячсми тьбю.
         RU_YO,   RU_Y,    RU_TS,    RU_U,   RU_K,  RU_E,     RU_N,  RU_G,   RU_SH, RU_SHCH,RU_Z,    RU_H,
           __ ,   RU_F,    RU_YERU,  RU_V,   RU_A,  RU_P,     RU_R,  RU_O,   RU_L,  RU_D,   RU_ZH,  RU_EE,
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
