/* A layout for the Cantor Keyboard */

#include QMK_KEYBOARD_H

/* Fancy looking spare keys. */
#define __ KC_TRNS
#define XX KC_NO

/* */
enum my_keycodes {
  SWITCH_LANG = SAFE_RANGE,
  KK_GO_DECLARATION,
  KK_RIGHT_ARROW,
  KK_FAT_RIGHT_ARROW,
  KK_NOT_EQUAL,
  KK_CSB1,
  KK_NOOP,

  SMTD_KEYCODES_BEGIN,
  // boo home-row mods
  BH_A,
  BH_O,
  BH_E,
  BH_S,
  BH_N,
  BH_T,
  BH_R,
  BH_I,
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
  KK_ENTER,
  KK_SPACE,
  KK_MOUSE,
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
  L_FKEYS_SYS,
  L_MOUSE,
};

#define KK_SHIFT OSM(MOD_LSFT)
#define KK_SYMBO OSL(L_SYMBOLS)

/* Switch language */
typedef enum {
  Ru, En,
} SlavaLang;
void slava_set_language(SlavaLang l) {
  switch (l) {
    case Ru:
      layer_on(L_RUSSIAN);
      break;
    case En:
      layer_off(L_RUSSIAN);
      break;
  }
}

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
const uint16_t PROGMEM ctl_esc_combo[] = {KK_SHIFT, BH_S, COMBO_END};
const uint16_t PROGMEM alt_esc_combo[] = {KK_SHIFT, BH_O, COMBO_END};
/* Two outer bottom keys on a single half to get into bootloader. */
const uint16_t PROGMEM boot_combo_left[]  = {KK_NOOP, KK_SYMBO, COMBO_END};
const uint16_t PROGMEM boot_combo_right[] = {KK_ENTER, KK_NOOP, COMBO_END};
/* On each half: the outermost bottom pinky key + the middle thumb key to reboot the keyboard. */
const uint16_t PROGMEM reset_combo_left[]  = {KK_NOOP, KK_SHIFT, COMBO_END};
const uint16_t PROGMEM reset_combo_right[] = {KK_SPACE, KK_NOOP, COMBO_END};
/* Digraphs */
const uint16_t PROGMEM go_declaration_combo[]  = {KC_H, BH_I, COMBO_END}; // :=
const uint16_t PROGMEM right_arrow_combo[]     = {KC_M, BH_I, COMBO_END}; // ->
const uint16_t PROGMEM fat_right_arrow_combo[] = {KC_H, KC_M, COMBO_END}; // =>
const uint16_t PROGMEM not_equal_combo[]       = {KC_H, KC_X, COMBO_END}; // !=
/* For unimpaired */
const uint16_t PROGMEM square_left_combo[]  = {BH_O, BH_S, COMBO_END};
const uint16_t PROGMEM square_right_combo[] = {BH_N, BH_R, COMBO_END};

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

  COMBO(square_left_combo , KC_LBRC),
  COMBO(square_right_combo, KC_RBRC),
};

/* */

/* */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (!process_smtd(keycode, record)) {
    return false;
  }

  switch (keycode) {
    case KC_ESC: /* Switch language in vim. */
      slava_set_language(En);
      return true;
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
    case KK_CSB1:
      if (record->event.pressed) {
        register_mods(MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT));
        register_code16(KC_BTN1);
      } else {
        unregister_code16(KC_BTN1);
        unregister_mods(MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT));
      }
      return false;
    default:
      return true; // Process all other keycodes normally
  }
}

/* Make right middle tap-hold extremely averted to a tap. */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
  switch (keycode) {
    case BH_T:
    case RU_L:
      if (timeout == SMTD_TIMEOUT_TAP) return SMTD_GLOBAL_TAP_TERM + 100;
      if (timeout == SMTD_TIMEOUT_RELEASE) return 5;
      break;
  }

  return get_smtd_timeout_default(timeout);
}

void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
  switch (keycode) {
    /* Boo home-row mods. */
    SMTD_MT(BH_A, KC_A, KC_LEFT_GUI)
    SMTD_MT(BH_O, KC_O, KC_LEFT_ALT)
    SMTD_LT(BH_E, KC_E, L_NUM_NAV)
    SMTD_MT(BH_S, KC_S, KC_LEFT_CTRL)
    //
    SMTD_MT(BH_N, KC_N, KC_RIGHT_CTRL)
    SMTD_LT(BH_T, KC_T, L_NUM_NAV)
    SMTD_MT(BH_R, KC_R, KC_RIGHT_ALT)
    SMTD_MT(BH_I, KC_I, KC_RIGHT_GUI)

    /* RU home-row mods. */
    SM_MU(RH_F, RU_F, KC_LEFT_GUI)
    SM_MU(RH_Y, RU_YERU, KC_LEFT_ALT)
    SM_LU(RH_V, RU_V, L_NUM_NAV)
    SM_MU(RH_A, RU_A, KC_LEFT_CTRL)
    SM_MU(RH_O, RU_O, KC_LEFT_CTRL)
    SM_LU(RH_L, RU_L, L_NUM_NAV)
    SM_MU(RH_D, RU_D, KC_LEFT_ALT)
    SM_MU(RH_Z, RU_ZH, KC_LEFT_GUI)

    // Thumb keys
    SMTD_LT(KK_ENTER, KC_ENTER, L_SYMBOLS)
    SMTD_LT(KK_SPACE, KC_SPACE, L_NUM_NAV)
    /* hold = mouse */
    case KK_MOUSE:
    {
      switch (action) {
        case SMTD_ACTION_TOUCH:
          break;

        case SMTD_ACTION_TAP:
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
    /* tap = ru, taptap = en, taphold = (↓en ↑ru)
       hold = fkeys */
    case KK_RU:
    {
      switch (action) {
        case SMTD_ACTION_TOUCH:
          break;

        case SMTD_ACTION_TAP:
          switch (tap_count) {
            case 0:
              slava_set_language(Ru);
              break;
            case 1:
              slava_set_language(En);
              break;
            case 2:
              break;
            default:
              break;
          }
          break;

        case SMTD_ACTION_HOLD:
          switch (tap_count) {
            case 0:
              layer_on(L_FKEYS_SYS);
              break;
            case 1:
              slava_set_language(En);
              break;
            default:
              break;
          }
          break;

        case SMTD_ACTION_RELEASE:
          switch (tap_count) {
            case 0:
              layer_off(L_FKEYS_SYS);
              break;
            case 1:
              slava_set_language(Ru);
            default:
              break;
          }
          break;
      }
      break;
    }
  }
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
 *  1. Spatial closeness is prioritised over predictability and speed
 *  2. As a result, home row mods are all the way down. There are misfirings some times,  
 *     but when it works, it feels great.
 *  3. Exception is for SHIFT, which is a dedicated `OSM(L_SFT)` key on a thumb.
 *
 *  ** Unicode Input
 *  1. A unicode layer with Russian letters. Switch between `Linux` and `Vim` input modes  
 *     VIM mode looks and awesome, but works only on Vim/Neovim  
 *     Linux mode implies a blink of a window for each character, but kinda works in all apps. By the way you can hide that window, being dedicated enough
 *  2. When using non-qwerty layout, switching language in OS requires lots of effort.  
 *     And symbols are spoiled: with Ru active, your `}` sent from keyboard becomes `ъ`
 *     With unicode input, theese issues just disappear.
 *
 */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [L_BOO] = LAYOUT_split_3x6_3(/** BOO LAYOUT
       __  '   ,   u   c   v                        q   f   d   l   y   /
       __  a   o   e   s   g                        b   n   t   r   i   -
       __      x   .   w   z                        p   h   m   k   j   __
                     MOUSE sft SYMBOLS          ret spc RU
       */
           __ , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           __ ,    BH_A,    BH_O,    BH_E,   BH_S,  KC_G,     KC_B,  BH_N,  BH_T,  BH_R,  BH_I,   KC_MINUS,
       KK_NOOP,      XX,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   KK_NOOP,

                          KK_MOUSE , KK_SHIFT , KK_SYMBO,     KK_ENTER , KK_SPACE, KK_RU
  ),

  /**
   * Russian layer.
   *
   * Activated by right-most key of the right thumb. Activation is tricky:
   *
   * 1. Tap activates the layer
   * 1. Double tap switches back to English.
   * 1. Tap of Esc switches back to English. This is especially useful in Vim
   * 1. When Ru is active, tap-hold the RU key. While you hold it, type English letters. Release to continue typing Russian. The idea is to facilitate typing isolated English words in a flow of Russian text.
   */
  [L_RUSSIAN] = LAYOUT_split_3x6_3(/** Russian layer ```
       ё   й   ц   у   к   е                        н   г   ш   щ   з   х
       __  ф   ы   в   а   п                        р   о   л   д   ж   э
       __  я   ч   с   м   и                        т   ь   б   ю   .   ъ
                            __  __  __    __  __  __
       ```
       */
         RU_YO,   RU_Y,    RU_TS,    RU_U,   RU_K,  RU_E,     RU_N,  RU_G,   RU_SH, RU_SHCH,RU_Z,   RU_H,
           __ ,   RH_F,    RH_Y,     RH_V,   RH_A,  RU_P,     RU_R,  RH_O,   RH_L,  RH_D,   RH_Z,   RU_EE,
           __ ,   RU_YA,   RU_CH,    RU_S,   RU_M,  RU_I,     RU_T,  RU_SOFT,RU_B,  RU_YU,  RU_DOT, RU_HARD,
                                     __ ,    __ ,   __ ,       __ ,   __ ,   __ ,
  ),

  /**
   * Layer for symbols.
   *
   * Activated by right-most thumb of the left hand.
   *
   * On both hands: middle row is for braces
   *
   * On the left hand:
   * - symbols on the top and bottom rows are placed as on `shift`ed L_NUM_NAV.
   * - middle row is prioritised for braces. So to reach `$` `%` `^` we tap `OSM(SHIFT)` and then a key from `L_NUM_NAV` layer.
   * -- ? on the right is useful when L_RUSSIAN is active, since `/` is shadowed by `RU_H`
   *
   * - / and \ are symmetrical and placed just by coincidence
   * - `*` `!` `#` on the left are kinda paired with
   *   `+` `?` `=` on the right
   *
   * - `\`` `;` are echoed with their shifted pairs
   *   `~` `:`
   *
   * Everything can be typed without combos, but we have combos for some digraphs: => != := ->
   * Mnemonics for these combos:
   *   - `=>` is like `=` plus the key on the right
   *   - `->` is a lighter arrow, so shifted one key to the weak fingers
   *   - `!=` `:=` are literally sums of their symbols
   *
   * Combos work independently of layers.
   *
   * Facility:
   *  - right hand: ringer finger: top row
   */
  [L_SYMBOLS] = LAYOUT_split_3x6_3(/*
       __  `   &   *   /   __                       __  \   +   __  ~   __
       __  ;   {   (   [   <                        >   ]   )   }   :   __
       __  __  !   @   #   󰹿                        󰭜   |   ?   __  __  __
                            __  __  __    __  __  __
       */
        XX,  KC_GRV,   KC_AMPR,  KC_ASTR, KC_SLSH,      XX,       XX,      KC_BSLS,  KC_PLUS,  KC_PIPE, KC_TILD, __,
        XX, KC_SCLN,   KC_LCBR,  KC_LPRN, KC_LBRC, KC_LABK,       KC_RABK, KC_RBRC,  KC_RPRN,  KC_RCBR, KC_COLN, __,
        XX,      XX,   KC_EXLM,    KC_AT, KC_HASH,  KC_DEL,       KC_BSPC,  KC_EQL,  KC_RABK,  KC_QUES, XX,      XX,
                                       __ ,    __ ,   __ ,         __ ,   __ ,   __ ,
  ),

  /**
   * Layer for numbers and navigation.
   *
   * Activated by holding home-row keys of the middle fingers.
   *
   * Basic idea is clean: numbers on the left, and navigation on the right.
   *
   * ** Here are a bit less obvious decisions
   * - two zeroes: one is in its logical place, before `1`, and another is on the home row place,
   *   which is easier to reach and free.
   * - `/` `:` `.` is to type `05/06/1970` `05:50` `3.1415`
   */
  [L_NUM_NAV] = LAYOUT_split_3x6_3(/*
       __  __  7   8   9   /                        __  pg↑ ↑   pg↓ __  __
       __  0   4   5   6   :                        ⇤-  ←   ⏎   →   -⇥  __
       __  0   1   2   3   .                        __  ⮀   ↓   __  __  __
                            __  __  __    __  __  __
       */
        XX,    XX,  KC_7,  KC_8,  KC_9, KC_SLSH,       XX,      KC_PGUP,  KC_UP,    KC_PGDN, XX,      XX,
        XX,  KC_0,  KC_4,  KC_5,  KC_6, KC_COLN,       KC_HOME, KC_LEFT,  KC_ENTER, KC_RGHT, KC_END,  XX,
        XX,  KC_0,  KC_1,  KC_2,  KC_3,  KC_DOT,       XX,      KC_TAB,   KC_DOWN,  XX,      XX,      XX,
                            __ ,    __ ,   __ ,         __ ,   __ ,   __ ,
  ),
  /**
   * Layer for F keys and multimedia buttons.
   *
   * Activated by right-most key of the right thumb.
   *
   * There are also keys to switch the mode of unicode input.
   *
   * Idea: UC_VIM and UC_LINX as well as other one-shot swithes would be fancier represented as
   *       [leader-sequences](https://docs.qmk.fm/features/leader_key).
   */
  [L_FKEYS_SYS] = LAYOUT_split_3x6_3(/*
        __ F11  F7  F8  F9  UVIM                     __  br↑ vl↑ ULX __  __
        __ F11  F4  F5  F6  __                       __  __  mut __  __  __
        __ F10  F1  F2  F3  __                       __  br↓ vl↓ __  __  __
                             __  __  __     __  __  __
       */
        XX,  KC_F12,  KC_F7,  KC_F8,  KC_F9, UC_VIM,       XX, KC_BRIU,  KC_VOLU,  UC_LINX,  XX,      XX,
        XX,  KC_F11,  KC_F4,  KC_F5,  KC_F6,     XX,       XX,      XX,  KC_MUTE,       XX,  XX,      XX,
   QK_BOOT,  KC_F10,  KC_F1,  KC_F2,  KC_F3,     XX,       XX, KC_BRID,  KC_VOLD,       XX,  XX, QK_BOOT,
                                __ ,    __ ,   __ ,         __ ,   __ ,   __ ,
  ),

  /**
   * Mouse layer.
   *
   * Activated by the left-most key of the left thumb.
   *
   * Right hand does all the job.
   *
   * Left hand can apply modifiers, to perform shift+click, or ctrl+wheelup.
   * Left hand can also press and hold the left mouse button, so you can select some text.
   *
   * Idea: my dream is to implement mouse bisection (with every step you choose direction to finally get the destination point. Like in binary search.). It's even possible and easy with [digitizer](https://docs.qmk.fm/features/digitizer). But unfortunately digitizer doesn't work on my Linux.
   */
  [L_MOUSE] = LAYOUT_split_3x6_3(/*
        __  __  __  __  __  __                       __  w↑  ↑  w↓  b3  __
        __  sft alt b1  ctl __                       __  <-  c  ->  b2  __
        __  __  __  __  __  __                       __  __  ↓  __  __  __
                             __  __  __     __  __  __
       */
        XX,      XX,        XX,       XX,      XX,  XX,       XX, KC_WH_U,  KC_MS_U,  KC_WH_D, KC_BTN3,  XX,
        XX, KC_LSFT,   KC_LALT,  KC_BTN1, KC_LCTL,  XX,       XX, KC_MS_L,  KC_BTN1,  KC_MS_R, KC_BTN2,  XX,
        XX,      XX,        XX,       XX,      XX,  XX,       XX, KK_CSB1,  KC_MS_D,       XX,      XX,  XX,
                                   __ ,    __ ,   __ ,        __ ,   __ ,   __ ,
  ),
};

/*
 mayBE make a shared layout for cantor and dactyl
 make a macro to convert cantor's layer to a dactyl's
*/

/* facility:
 * , and . are used only as non-shifted. There are two half-keys to occuqy!
 */
