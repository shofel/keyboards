/**
 * A layout for the Cantor Keyboard.
 *
 * TODO Next:
 * - reach nav with a single right hand: leader,n
 *   - a way back
 * - oneshot win with a leader seq
 * - dedicated key to reset state. Employ it in ESC then ; leader seq or a combo
 * - mouse: turn one btn1 to a sticky - for selection
 * - mouse - bisect with digitizer. I see a digitizer in gnome settings. It shoold work now!
 * - employ ucis for emoji: tulip and other flowers, thumbup, ok
 * - implement UC_VIM in userspace ; and switch to qmk trunk
 * - - rawhid for seamless unicode modes in vim and linux
 * - dx: qmk userspace and useful lsp hints in vim
 *
 * Big dream: employ zig
 * - implement modules for keymap in zig
 * - make the whole keymap in zig
 * - from QMK use only low-level (keyboard support and matrix poll)
 *   - the rest features and a keymap implement in zig
 * - ? replace gcc with zig.build
 *
 * Idea: Draw the layers diagram by hand
 * Idea: Make animations to explain tricks
 *
 * References
 * https://github.com/possumvibes/keyboard-layout?tab=readme-ov-file#code-influences-alphabetically-and-non-comprehensively
 *   - callum
 *   - drashma
 */

#include <stdint.h>
#include QMK_KEYBOARD_H

#include "unicode.c"
#include "oneshot.h"

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
  KK = SAFE_RANGE,

  KK_GO_DECLARATION,
  KK_RIGHT_ARROW,
  KK_FAT_RIGHT_ARROW,
  KK_NOT_EQUAL,
  KK_NOOP,

  /* One-shot trigger keys */
  OS_CTL,
  OS_ALT,
  OS_GUI,
  OS_SFT,
};

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
#define KK_SYMBO OSL(L_SYMBOLS)
#define KK_SHIFT OS_SFT

/* Switch language */

static int8_t ru_active = 0;

void ru_suspend(void) {
    ru_active -= 1;
    layer_off(L_RUSSIAN);
}

void ru_resume(void) {
    ru_active += 1;
    if (ru_active > 0) {
        layer_on(L_RUSSIAN);
    }
}

void ru_enable(void) {
    ru_active = 1;
    layer_on(L_RUSSIAN);
}

void ru_disable(void) {
  ru_active = 0;
  layer_off(L_RUSSIAN);
}

/* Key overrides */

// Make , . immune to shift
const key_override_t labk_override = ko_make_basic(MOD_MASK_SHIFT, KC_COMMA, KC_COMMA);
const key_override_t rabk_override = ko_make_basic(MOD_MASK_SHIFT, KC_DOT,   KC_DOT);

const key_override_t *key_overrides[] = {
  &labk_override,
  &rabk_override,
};

/**
 * Combos
 * NB: to add a combo, add it in 3 places
 */

/* Hit both middle thumb keys for esc. */
const uint16_t PROGMEM esc_combo[]     = {KK_SHIFT, KC_SPACE, COMBO_END};
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
/* Vertical combos for mods */
const uint16_t PROGMEM lctl_combo[] = {KC_S, KC_C, COMBO_END};
const uint16_t PROGMEM llt2_combo[] = {KC_E, KC_U, COMBO_END};
const uint16_t PROGMEM lalt_combo[] = {KC_O, KC_SCLN, COMBO_END};
const uint16_t PROGMEM lgui_combo[] = {KC_A, KC_QUOT, COMBO_END};
const uint16_t PROGMEM rctl_combo[] = {KC_N, KC_F, COMBO_END};
const uint16_t PROGMEM rlt2_combo[] = {KC_T, KC_D, COMBO_END};
const uint16_t PROGMEM ralt_combo[] = {KC_R, KC_L, COMBO_END};
const uint16_t PROGMEM rgui_combo[] = {KC_I, KC_Y, COMBO_END};
/* Combos to access layers */
const uint16_t PROGMEM fkeys_combo[] = {KC_B, KC_Q, COMBO_END};

/* Indices for all combos (designated initializers) */
enum combos {
  CMB_ESC,
  CMB_CTL_ESC,
  CMB_ALT_ESC,

  CMB_BOOT_L,
  CMB_BOOT_R,

  CMB_RESET_L,
  CMB_RESET_R,

  CMB_GO_DECL,
  CMB_FAT_ARROW,
  CMB_RIGHT_ARROW,
  CMB_NOT_EQUAL,

  CMB_SQ_L,
  CMB_SQ_R,
  CMB_BR_L,
  CMB_BR_R,
  CMB_CU_L,
  CMB_CU_R,
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

  CMB_FSYS,
};

combo_t key_combos[] = {
  [CMB_ESC]        = COMBO(esc_combo, KC_ESC),

  [CMB_BOOT_L]     = COMBO(boot_combo_left,  QK_BOOT),
  [CMB_BOOT_R]     = COMBO(boot_combo_right, QK_BOOT),

  [CMB_RESET_L]    = COMBO(reset_combo_left,  QK_REBOOT),
  [CMB_RESET_R]    = COMBO(reset_combo_right, QK_REBOOT),

  [CMB_GO_DECL]    = COMBO(go_declaration_combo, KK_GO_DECLARATION),
  [CMB_FAT_ARROW]  = COMBO(fat_right_arrow_combo, KK_FAT_RIGHT_ARROW),
  [CMB_RIGHT_ARROW]= COMBO(right_arrow_combo, KK_RIGHT_ARROW),
  [CMB_NOT_EQUAL]  = COMBO(not_equal_combo, KK_NOT_EQUAL),

  /* ({[<>]}) */
  [CMB_SQ_L]       = COMBO(square_left_combo , KC_LBRC),
  [CMB_SQ_R]       = COMBO(square_right_combo, KC_RBRC),
  [CMB_BR_L]       = COMBO(brace_left_combo, KC_LPRN),
  [CMB_BR_R]       = COMBO(brace_right_combo, KC_RPRN),
  [CMB_CU_L]       = COMBO(curly_left_combo, KC_LCBR),
  [CMB_CU_R]       = COMBO(curly_right_combo, KC_RCBR),
  [CMB_ANG_L]      = COMBO(angle_left_combo, KC_LABK),
  [CMB_ANG_R]      = COMBO(angle_right_combo, KC_RABK),

  [CMB_LCTL]       = COMBO(lctl_combo, OS_CTL),
  [CMB_LLT2]       = COMBO(llt2_combo, OSL(L_NUM_NAV)),
  [CMB_LALT]       = COMBO(lalt_combo, OS_ALT),
  [CMB_LGUI]       = COMBO(lgui_combo, OS_GUI),
  [CMB_RCTL]       = COMBO(rctl_combo, OS_CTL),
  [CMB_RLT2]       = COMBO(rlt2_combo, OSL(L_NUM_NAV)),
  [CMB_RALT]       = COMBO(ralt_combo, OS_ALT),
  [CMB_RGUI]       = COMBO(rgui_combo, OS_GUI),

  [CMB_FSYS]       = COMBO(fkeys_combo, OSL(L_FKEYS_SYS)),
};

void process_combo_event(uint16_t combo_index, bool pressed) {
  switch (combo_index) {
  }
}

/* Oneshot */

oneshot_state_entry_t oneshot_state_entries[] = {
  {OS_CTL, KC_LCTL, os_up_unqueued},
  {OS_ALT, KC_LALT, os_up_unqueued},
  {OS_GUI, KC_LGUI, os_up_unqueued},
  {OS_SFT, KC_LSFT, os_up_unqueued},
};

size_t oneshot_state_entries_size = sizeof(oneshot_state_entries) / sizeof(oneshot_state_entry_t);

bool is_oneshot_cancel_key(uint16_t keycode) {
  return false;
}

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
      return true;
    default:
      return false;
  }
}

void oneshot_process_event(oneshot_state_entry_t *oneshot) {
  if ((oneshot->trigger == OS_CTL) ||
      (oneshot->trigger == OS_ALT) ||
      (oneshot->trigger == OS_GUI))
  {
    switch (oneshot->state) {
      case os_down_unused: ru_suspend(); break;
      case os_down_used: break;
      case os_up_queued: break;
      case os_up_unqueued: ru_resume(); break;
    }
  }
}

/* Leader */

void leader_start_user(void) {
  ru_suspend();
}

void leader_end_user(void) {
  ru_resume();

  /* Ru */
  if (leader_sequence_one_key(KC_R)) {
    ru_enable();
  }
  if (leader_sequence_one_key(KC_E)) {
    ru_disable();
  }
  if (leader_sequence_one_key(KC_L)) {
    set_unicode_input_mode(UNICODE_MODE_LINUX);
    ru_enable();
  }
  if (leader_sequence_one_key(KC_V)) {
    set_unicode_input_mode(UNICODE_MODE_VIM);
    ru_enable();
  }

  /* Layers */
  if (leader_sequence_one_key(KC_M)) {
    layer_on(L_MOUSE);
  }

  /* */
  if (leader_sequence_one_key(KC_A)) {
    tap_code16(LCTL(KC_A));
    tap_code16(KC_DEL);
  }
  if (leader_sequence_one_key(KC_K)) {
    tap_code16(LGUI(KC_T));
  }
}

/* */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  /* Route oneshot triggers first. */
  // TODO stop processing on false?
  // when a oneshot is a custom key, then no difference
  // but if we redefine an existing key, then it's a must do
  oneshot_process_record(
      oneshot_state_entries,
      oneshot_state_entries_size,
      keycode,
      record
  );

  switch (keycode) {
    case KC_ESC:
      ru_disable(); /* Especially useful in vim. */
      layer_off(L_MOUSE);
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
    default:
      break;
  }

  return true;
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
 *  Vertical combo mods: a home-row key plus a key just above it.
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
                        __ sft SYMBOLS          ret spc LEAD
       */
           __ , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           __ ,    KC_A,    KC_O,    KC_E,   KC_S,  KC_G,     KC_B,  KC_N,  KC_T,  KC_R,  KC_I,   KC_MINUS,
       KK_NOOP,     __ ,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   KK_NOOP,

                                __ , KK_SHIFT , KK_SYMBO,     KC_ENTER , KC_SPACE, QK_LEAD
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
        XX,  KC_GRV,   KC_AMPR,  KC_ASTR, EM_THNK, EM_FLWR,       XX,      KC_BSLS,  KC_PLUS,  KC_PIPE, KC_TILD, KC_SLASH,
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
       XX,KC_QUOT,  KC_7,  KC_8,  KC_9, KC_SLSH,       KC_GRV,  KC_PGUP,  KC_UP,    KC_PGDN, XX,      XX,
       XX,   KC_0,  KC_4,  KC_5,  KC_6, KC_COLN,       KC_HOME, KC_LEFT,  KC_ENTER, KC_RGHT, KC_END,  XX,
       XX,KC_COMM,  KC_1,  KC_2,  KC_3,  KC_DOT,       XX,      KC_TAB,   KC_DOWN,  XX,      XX,      XX,
                            __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),
  /**
   * Layer for F keys and multimedia buttons.
   */
  [L_FKEYS_SYS] = LAYOUT_split_3x6_3(/*
        __ F11  F7  F8  F9  __                       __  br↑ vl↑ __  DBG __
        __ F11  F4  F5  F6  __                       __  __  __  __  __  __
       bot F10  F1  F2  F3  __                       __  br↓ vl↓ vl0 __  bot
                             __  __  __     __  __  __
       */
        XX,  KC_F12,  KC_F7,  KC_F8,  KC_F9,     XX,       XX, KC_BRIU,  KC_VOLU,       XX,  DB_TOGG, XX,
        XX,  KC_F11,  KC_F4,  KC_F5,  KC_F6,     XX,       XX,      XX,       XX,       XX,       XX, XX,
   QK_BOOT,  KC_F10,  KC_F1,  KC_F2,  KC_F3,     XX,       XX, KC_BRID,  KC_VOLD,  KC_MUTE,  XX, QK_BOOT,
                                __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),

  /**
   * Mouse layer.
   *
   * Activated by Leader,m
   *
   * Left hand has its own button one to facilitate selection
   * Left hand can apply modifiers, to perform shift+click, or ctrl+wheelup.
   */
  [L_MOUSE] = LAYOUT_split_3x6_3(/*
        __  a2  __  __  __  __                       __  w↑  ↑  w↓  b3  __
        __  a1  __  __  __  __                       __  <-  c  ->  b2  __
        __  a0  __  __  __  __                       __  b1  ↓  __  __  __
                             __  __  __     b1  b2  b3
       */
        XX, MS_ACL2,        XX,       XX,      XX,  XX,       XX, KC_WH_U,  KC_MS_U,  KC_WH_D, KC_BTN3,  XX,
        XX, MS_ACL1,        XX,       XX,      XX,  XX,       XX, KC_MS_L,  KC_BTN1,  KC_MS_R, KC_BTN2,  XX,
        XX, MS_ACL0,        XX,       XX,      XX,  XX,       XX, KC_BTN1,  KC_MS_D,       XX,      XX,  XX,

                                   __ ,    __ ,   __ ,         __ ,  __ ,  __
  ),
};
