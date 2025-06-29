/**
 * A layout for the Cantor Keyboard.
 *
 * When I return later, then I'll
 * - rewrite sm_td to replicate behaviour, but make it cleaner
 * - make a slim firmware ground-up with zig
 *
 * problems with sm_td:
 * - leader key is shadowed : https://github.com/stasmarkin/sm_td/issues/29
 * - one-shot mods on an sm_td hold-layer
 * - caps-word is non-trivial
 *
 * Idea: one-word without home-row mods
 * - disable HRM on ctrl-w or ctrl+backspace : as if mod misfire detected
 * - enable HRM back on space : as if a problem word was successfully typed
 *
 * Idea: One-word layer
 * It can also be a mod-layer
 * Contra: I don't mind holding a layer key, since all of them are on thumb keys
 *
 * Idea: Draw the layers diagram by hand
 *
 * Idea: Make animations to explain tricks
 *
 * References
 * https://github.com/possumvibes/keyboard-layout?tab=readme-ov-file#code-influences-alphabetically-and-non-comprehensively
 *   - callum
 *   - drashma
 */

#include QMK_KEYBOARD_H

// #define SMTD_DEBUG_ENABLED
#include "sm_td.h"

void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable = false;
  debug_matrix = false;
  debug_keyboard = false;
  debug_mouse = false;
}

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
  KK_NOOP,

  // thumb keys
  KK_RU,
  KK_MOUSE,

  // Unicode modes
  KK_VIM,
  KK_LINX,
};

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

/* Simple thumb keys. */
#define KK_SHIFT OSM(MOD_LSFT)
#define KK_SYMBO OSL(L_SYMBOLS)

/* One-shot modifiers, to place on L_MOUSE and L_FKEYS_SYS */
#define OSM_SFT OSM(MOD_LSFT)
#define OSM_ALT OSM(MOD_LALT)
#define OSM_CTL OSM(MOD_LCTL)
#define OSM_GUI OSM(MOD_LGUI)

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
const key_override_t labk_override = ko_make_basic(MOD_MASK_SHIFT, KC_COMMA, KC_SEMICOLON);
const key_override_t rabk_override = ko_make_basic(MOD_MASK_SHIFT, KC_DOT,   KC_COLON);

const key_override_t *key_overrides[] = {
  &labk_override,
  &rabk_override,
};

/* Combos */

/* Hit both middle thumb keys for esc. */
const uint16_t PROGMEM esc_combo[]     = {KK_SHIFT, KC_SPACE, COMBO_END};
const uint16_t PROGMEM ctl_esc_combo[] = {KK_SHIFT, KC_S, COMBO_END};
const uint16_t PROGMEM alt_esc_combo[] = {KK_SHIFT, KC_O, COMBO_END};
/* Two outer bottom keys on a single half to get into bootloader. */
const uint16_t PROGMEM boot_combo_left[]  = {KK_NOOP, KK_SYMBO, COMBO_END};
const uint16_t PROGMEM boot_combo_right[] = {KC_ENTER, KK_NOOP, COMBO_END};
/* On each half: the outermost bottom pinky key + the middle thumb key to reboot the keyboard. */
const uint16_t PROGMEM reset_combo_left[]  = {KK_NOOP, KK_SHIFT, COMBO_END};
const uint16_t PROGMEM reset_combo_right[] = {KC_SPACE, KK_NOOP, COMBO_END};
/* Digraphs */
const uint16_t PROGMEM go_declaration_combo[]  = {KC_H, KC_I, COMBO_END}; // :=
const uint16_t PROGMEM fat_right_arrow_combo[] = {KC_H, KC_M, COMBO_END}; // =>
const uint16_t PROGMEM right_arrow_combo[]     = {KC_H, KC_K, COMBO_END}; // ->
const uint16_t PROGMEM not_equal_combo[]       = {KC_H, KC_X, COMBO_END}; // !=
/* [{(<>)}] */
const uint16_t PROGMEM square_left_combo[]  = {KC_S, KC_O, COMBO_END};
const uint16_t PROGMEM square_right_combo[] = {KC_N, KC_R, COMBO_END};
const uint16_t PROGMEM brace_left_combo[]   = {KC_S, KC_A, COMBO_END};
const uint16_t PROGMEM brace_right_combo[]  = {KC_N, KC_I, COMBO_END};
const uint16_t PROGMEM curly_left_combo[]   = {KC_E, KC_A, COMBO_END};
const uint16_t PROGMEM curly_right_combo[]  = {KC_T, KC_I, COMBO_END};
const uint16_t PROGMEM angle_left_combo[]   = {KC_G, KC_E, COMBO_END};
const uint16_t PROGMEM angle_right_combo[]  = {KC_B, KC_T, COMBO_END};

combo_t key_combos[] = {
  COMBO(esc_combo, KC_ESC),
  COMBO(ctl_esc_combo, LCTL(KC_ESC)),
  COMBO(alt_esc_combo, LALT(KC_ESC)),

  COMBO(boot_combo_left,  QK_BOOT),
  COMBO(boot_combo_right, QK_BOOT),

  COMBO(reset_combo_left,  QK_REBOOT),
  COMBO(reset_combo_right, QK_REBOOT),

  COMBO(go_declaration_combo, KK_GO_DECLARATION),
  COMBO(fat_right_arrow_combo, KK_FAT_RIGHT_ARROW),
  COMBO(right_arrow_combo, KK_RIGHT_ARROW),
  COMBO(not_equal_combo, KK_NOT_EQUAL),

  COMBO(square_left_combo , KC_LBRC),
  COMBO(square_right_combo, KC_RBRC),
  COMBO(brace_left_combo, KC_LPRN),
  COMBO(brace_right_combo, KC_RPRN),
  COMBO(curly_left_combo, KC_LCBR),
  COMBO(curly_right_combo, KC_RCBR),
  COMBO(angle_left_combo, KC_LABK),
  COMBO(angle_right_combo, KC_RABK),
};

/* */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
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

    case LCTL(KC_ESC): return true;
    case LALT(KC_ESC): return true;

    case QK_BOOT: return true;
    case QK_REBOOT: return true;

    case KC_LBRC: return true;
    case KC_RBRC: return true;
    case KC_LPRN: return true;
    case KC_RPRN: return true;
    case KC_LCBR: return true;
    case KC_RCBR: return true;
    case KC_LABK: return true;
    case KC_RABK: return true;

    default:
      break;
  }

  if (!process_smtd(keycode, record)) {
    return false;
  }

  return true;
}

/* Make right middle tap-hold extremely averted to a tap. */
uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
  switch (keycode) {
    case KC_T:
    case RU_L:
      if (timeout == SMTD_TIMEOUT_TAP) return SMTD_GLOBAL_TAP_TERM + 100;
      if (timeout == SMTD_TIMEOUT_RELEASE) return 5;
      break;
  }

  return get_smtd_timeout_default(timeout);
}

smtd_resolution on_smtd_action(
    uint16_t keycode, smtd_action action, uint8_t tap_count
) {
  switch (keycode) {
    /* Boo home-row mods. */
    SMTD_MT(KC_A, KC_LEFT_GUI)
    SMTD_MT(KC_O, KC_LEFT_ALT)
    SMTD_LT(KC_E, L_NUM_NAV)
    SMTD_MT(KC_S, KC_LEFT_CTRL)
    //
    SMTD_MT(KC_N, KC_LEFT_CTRL)
    SMTD_LT(KC_T, L_NUM_NAV)
    SMTD_MT(KC_R, KC_LEFT_ALT)
    SMTD_MT(KC_I, KC_LEFT_GUI)

    /* RU home-row mods. */
    SM_MU(RU_F, KC_LEFT_GUI)
    SM_MU(RU_YERU, KC_LEFT_ALT)
    SM_LU(RU_V, L_NUM_NAV)
    SM_MU(RU_A, KC_LEFT_CTRL)
    SM_MU(RU_O, KC_LEFT_CTRL)
    SM_LU(RU_L, L_NUM_NAV)
    SM_MU(RU_D, KC_LEFT_ALT)
    SM_MU(RU_ZH, KC_LEFT_GUI)

    // Thumb keys
    SMTD_LT(KC_ENTER, L_SYMBOLS)
    SMTD_LT(KC_SPACE, L_NUM_NAV)
    /* hold = mouse */
    SMTD_DANCE(KK_MOUSE,
      NOTHING,
      NOTHING,
      layer_on(L_MOUSE),
      layer_off(L_MOUSE))

    /* tap = ru, taptap = en, taphold = (↓en ↑ru)
       hold = fkeys */
    SMTD_DANCE(KK_RU,
      NOTHING,
      switch (tap_count) {
        case 0: slava_set_language(Ru); break;
        case 1: slava_set_language(En); break;
        default: break;
      },
      switch (tap_count) {
        case 0: layer_on(L_FKEYS_SYS); break;
        case 1: slava_set_language(En); break;
        default: break;
      },
      switch (tap_count) {
        case 0:
          layer_off(L_FKEYS_SYS);
          break;
        case 1:
          slava_set_language(Ru);
        default:
          break;
      }
    );

    SMTD_DANCE(KK_VIM,
        EXEC(
          set_unicode_input_mode(UNICODE_MODE_VIM);
          slava_set_language(Ru);
        ),
        NOTHING,
        NOTHING,
        NOTHING
    );

    SMTD_DANCE(KK_LINX,
        EXEC(
          set_unicode_input_mode(UNICODE_MODE_LINUX);
          slava_set_language(Ru);
        ),
        NOTHING,
        NOTHING,
        NOTHING
    );
  }

  return SMTD_RESOLUTION_UNHANDLED;
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
 *  3. Exception is for SHIFT, which is a dedicated `OSM(L_SFT)` key on a middle thumb of left hand.
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
 *  Switch between `Linux` and `Vim` input modes  
 *  VIM mode looks and feels awesome, but works only in Vim/Neovim  
 *  Linux mode feels clunky in some apps, but kinda works everywhere.  
 *  Also, as of time of writing, the vim mode is not in upstream QMK. I sent [a pull-request](https://github.com/qmk/qmk_firmware/pull/25188) which implements it
 */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [L_BOO] = LAYOUT_split_3x6_3(/** BOO LAYOUT
       __  '   ,   u   c   v                        q   f   d   l   y   /
       __  a   o   e   s   g                        b   n   t   r   i   -
       __      x   .   w   z                        p   h   m   k   j   __
                     MOUSE sft SYMBOLS          ret spc RU
       */
           __ , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           __ ,    KC_A,    KC_O,    KC_E,   KC_S,  KC_G,     KC_B,  KC_N,  KC_T,  KC_R,  KC_I,   KC_MINUS,
       KK_NOOP,      XX,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   KK_NOOP,

                          KK_MOUSE , KK_SHIFT , KK_SYMBO,     KC_ENTER , KC_SPACE, KK_RU
  ),

  /**
   * Russian layer.
   *
   * Activated by right-most key of the right thumb. Activation is tricky:
   *
   * 1. Tap activates the layer
   * 1. Double tap switches back to English.
   * 1. Tap of Esc switches back to English. So that in vim you return to normal mode in English.
   * 1. Tap-then-hold RU key to type an English word in a flow of Russian text. When Ru is active, tap-then-hold the RU key. While you hold it, you type English letters. Release it to continue typing Russian. The idea is to facilitate typing isolated English words in a flow of Russian text.
   */
  [L_RUSSIAN] = LAYOUT_split_3x6_3(/** Russian layer ```
       ё   й   ц   у   к   е                        н   г   ш   щ   з   х
       __  ф   ы   в   а   п                        р   о   л   д   ж   э
       __  я   ч   с   м   и                        т   ь   б   ю   .   ъ
                            __  __  __    __  __  RU
       ```
       */
         RU_YO,   RU_Y,    RU_TS,    RU_U,   RU_K,  RU_E,     RU_N,  RU_G,   RU_SH, RU_SHCH,RU_Z,   RU_H,
           __ ,   RU_F,    RU_YERU,  RU_V,   RU_A,  RU_P,     RU_R,  RU_O,   RU_L,  RU_D,   RU_ZH,  RU_EE,
           __ ,   RU_YA,   RU_CH,    RU_S,   RU_M,  RU_I,     RU_T,  RU_SOFT,RU_B,  RU_YU,  RU_DOT, RU_HARD,
                                     __ ,    __ ,   __ ,       __ ,   __ ,   KK_RU
  ),

  /**
   * Layer for symbols.
   *
   * Activated by right-most thumb of the left hand.
   *
   * Braces are combos, they are not in this layer.
   *
   * On the left hand:
   * - symbols are placed as on `shift`ed L_NUM_NAV.
   * -- ? on the right is useful when L_RUSSIAN is active, since `/` is shadowed by `RU_H`
   *
   * - `*` `!` `#` on the left are kinda paired with
   *   `+` `?` `=` on the right
   *
   * - `\`` `;` are echoed with their shifted pairs
   *   `~` `:`
   *
   * Everything can be typed without combos, but we have combos for some digraphs: => != := ->
   * Mnemonics for these combos:
   *   - `=>` is like `=` plus the key on the right
   *   - `->` is a lighter arrow, so shifted head key to the weaker finger
   *   - `!=` `:=` are literally sums of their symbols
   *
   * Combos work independently of layers.
   */
  [L_SYMBOLS] = LAYOUT_split_3x6_3(/*
       __  `   &   *   __  __                       __  \   +   |   ~   /
       __  ;   $   %   ^   __                       __  __  __  __  :   -
       __  __  !   @   #   󰹿                        󰭜   =   __  ?   __  __
                            __  __  SYM   __  __  __
       */
        XX,  KC_GRV,   KC_AMPR,  KC_ASTR, EM_THNK,      XX,       XX,      KC_BSLS,  KC_PLUS,  KC_PIPE, KC_TILD, KC_SLASH,
        XX, KC_SCLN,    KC_DLR,  KC_PERC, KC_CIRC,      XX,       XX,           XX,       XX,       XX, KC_COLN, KC_MINUS,
        XX,      XX,   KC_EXLM,    KC_AT, KC_HASH,  KC_DEL,       KC_BSPC,  KC_EQL,       XX,  KC_QUES, XX,      XX,
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
   * - two zeroes: one is in its logical place, before `1`, and another is on the home row place,
   *   which is easier to reach and free.
   * - `/` `:` `.` is to type `05/06/1970` `05:50` `3.1415`
   * - KC_GRV (```) is to switch windows in gnome:
   *   - with the left hand hold alt+L_NUM_NAV
   *   - with the right hand tap KC_GRV
   *   - while still holding alt and L_NUM_NAV, you can tap left and right arrows with right
   */
  [L_NUM_NAV] = LAYOUT_split_3x6_3(/*
       __  __  7   8   9   /                        `   pg↑ ↑   pg↓ __  __
       __  0   4   5   6   :                        ⇤-  ←   ⏎   →   -⇥  __
       __  0   1   2   3   .                        __  ⮀   ↓   __  __  __
                            __  __  __    __  __  __
       */
        XX,    XX,  KC_7,  KC_8,  KC_9, KC_SLSH,       KC_GRV,  KC_PGUP,  KC_UP,    KC_PGDN, XX,      XX,
        XX,  KC_0,  KC_4,  KC_5,  KC_6, KC_COLN,       KC_HOME, KC_LEFT,  KC_ENTER, KC_RGHT, KC_END,  XX,
        XX,  KC_0,  KC_1,  KC_2,  KC_3,  KC_DOT,       XX,      KC_TAB,   KC_DOWN,  XX,      XX,      XX,
                            __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),
  /**
   * Layer for F keys and multimedia buttons.
   * Also modifiers on the home-row.
   *
   * Activated by right-most key of the right thumb.
   *
   * There are also keys to switch the mode of unicode input.
   *
   * Idea: UC_VIM and UC_LINX as well as other one-shot swithes would be fancier represented as
   *       [leader-sequences](https://docs.qmk.fm/features/leader_key).
   */
  [L_FKEYS_SYS] = LAYOUT_split_3x6_3(/*
        __ F11  F7  F8  F9  UVIM                     hr↑ br↑ vl↑ ULX DBG __
        __ F11  F4  F5  F6  __                       hr↓ ctl sft alt gui __
       bot F10  F1  F2  F3  __                       __  br↓ vl↓ vl0 __  bot
                             __  __  __     __  __  __
       */
        XX,  KC_F12,  KC_F7,  KC_F8,  KC_F9, KK_VIM,       XX, KC_BRIU,  KC_VOLU,  KK_LINX,  DB_TOGG, XX,
        XX,  KC_F11,  KC_F4,  KC_F5,  KC_F6,     XX,       XX, KC_LCTL,  KC_LSFT,  KC_LALT,  KC_RGUI, XX,
   QK_BOOT,  KC_F10,  KC_F1,  KC_F2,  KC_F3,     XX,       XX, KC_BRID,  KC_VOLD,  KC_MUTE,  XX, QK_BOOT,
                                __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),

  /**
   * Mouse layer.
   * Also contains modifiers on the home row.
   *
   * Activated by the left-most key of the left thumb.
   *
   * Right hand does all the job.
   * Mouse buttons are duplicated on thumb keys.
   *
   * Left hand has its own button one to facilitate selection
   * Left hand can apply modifiers, to perform shift+click, or ctrl+wheelup.
   *
   * Idea: my dream is to implement mouse bisection (with every step you choose direction to finally get the destination point. Like in binary search.). It's even possible and easy with [digitizer](https://docs.qmk.fm/features/digitizer). But unfortunately digitizer doesn't work on my Linux.
   */
  [L_MOUSE] = LAYOUT_split_3x6_3(/*
        __  __  __  __  __  __                       __  w↑  ↑  w↓  b3  __
        __  gui alt sft ctl __                       __  <-  c  ->  b2  __
        __  __  __  __  b1  __                       __  __  ↓  __  __  __
                             __  __  __     b1  b2  b3
       */
        XX, MS_ACL2,        XX,       XX,      XX,  XX,       XX, KC_WH_U,  KC_MS_U,  KC_WH_D, KC_BTN3,  XX,
   OSM_GUI, MS_ACL1,   OSM_ALT,  OSM_SFT, OSM_CTL,  XX,       XX, KC_MS_L,  KC_BTN1,  KC_MS_R, KC_BTN2,  XX,
        XX, MS_ACL0,        XX,       XX, KC_BTN1,  XX,       XX,      XX,  KC_MS_D,       XX,      XX,  XX,

                                   __ ,    __ ,   __ ,     KC_BTN1, KC_BTN3, KC_BTN2
  ),
};
