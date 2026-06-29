/**
 * A keymap for 6x3_3 layout.
 * Originally made for Cantor; and hopefully useful for a Dactyl.
 */

#include <stdint.h>
#include QMK_KEYBOARD_H
#include "introspection.h"
#include "modules/shofel/unicode_ru/introspection.h"

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
 *   mod_ru_suspend_depth — nested mod holds: mask Russian only, so Ctrl/Alt/Gui
 *                          fall through to the Latin base while num/nav etc.
 *                          stay put
 * applied_layer is the overlay we physically turned on. The reconcile only ever
 * touches our own layer, so it never disturbs a native OSL momentary layer or
 * the base layer (no layer_move).
 */
#define TOGGLE_NONE 0xFF

static uint8_t active_toggle        = TOGGLE_NONE;
static uint8_t applied_layer        = TOGGLE_NONE;
static bool    leader_active        = false;
static uint8_t mod_ru_suspend_depth = 0;

static void toggle_apply(void) {
    uint8_t want = active_toggle;
    if (leader_active) {
        want = TOGGLE_NONE;
    } else if (active_toggle == L_RUSSIAN && mod_ru_suspend_depth > 0) {
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
    mod_ru_suspend_depth = 0;
    toggle_apply();
}

static void toggle_reset(void) {
    oneshot_cancel();
    toggle_disable();
}

void mod_ru_suspend(void) {
    mod_ru_suspend_depth += 1;
    toggle_apply();
}

void mod_ru_resume(void) {
    if (mod_ru_suspend_depth > 0) {
        mod_ru_suspend_depth -= 1;
    }
    toggle_apply();
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
/* Q+B -> one-shot FKEYS/SYS layer. SYM no longer needs this combo: it is reached
 * via the left thumb (KK_SYMBO) and the RET layer-tap (right inner thumb). */
const uint16_t PROGMEM fkeys_combo[] = {KC_B, KC_Q, COMBO_END};

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
  if ((oneshot->trigger == OS_CTL) ||
      (oneshot->trigger == OS_ALT) ||
      (oneshot->trigger == OS_GUI))
  {
    switch (oneshot->state) {
      case os_down_unused: mod_ru_suspend(); break;
      case os_down_used: break;
      case os_up_queued: break;
      case os_up_unqueued: mod_ru_resume(); break;
    }
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
   * (leader,v) is for vim. All unicode (Cyrillic, « », — № §) goes through
   * compose now, so the old ibus hex backend (UNICODE_MODE_LINUX) is retired. */
  if (leader_sequence_one_key(KC_R) || leader_sequence_one_key(KC_C)) {
    ru_compose_mode = true;
    toggle_enable(L_RUSSIAN);
  }
  if (leader_sequence_one_key(KC_V)) {
    ru_compose_mode = false;
    set_unicode_input_mode(UNICODE_MODE_VIM);
    toggle_enable(L_RUSSIAN);
  }
  /* Back to English: drop the active toggle layer. */
  if (leader_sequence_one_key(KC_E)) {
    ru_compose_mode = true;
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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  /* Compose-mode Russian: when active, the RU_ and U_ unicode_map keys are
   * emitted as Compose sequences here (and consumed) before the hex path. */
  if (ru_compose_process(keycode, record)) {
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
          /* Shifted: emit the guillemet. Compose mode (default) → rolling-safe
           * Compose sequence; vim mode → register_unicode. ru_compose_emit_code
           * strips shift itself; the vim branch strips it manually. */
          if (ru_compose_mode) {
            ru_compose_emit_code(keycode == KK_LANGLE ? "q[" : "q]"); // « »
          } else {
            uint8_t held = get_mods();
            clear_oneshot_mods();
            del_mods(MOD_MASK_SHIFT);
            register_unicode(keycode == KK_LANGLE ? 0x00AB : 0x00BB); // « »
            set_mods(held);
          }
        } else {
          tap_code16(keycode == KK_LANGLE ? KC_LABK : KC_RABK); // < >
        }
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
 *  Switch between `Linux` and `Vim` input modes
 *  VIM mode looks and feels awesome, but works only in Vim/Neovim
 *  Linux mode feels clunky in some apps, but kinda works everywhere.
 *  Also, as of time of writing, the vim mode is not in upstream QMK. I sent [a pull-request](https://github.com/qmk/qmk_firmware/pull/25188) which implements it
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
       ·      ~   @   #   ·                        ·   ?   :   +   %   -
       ·  ·   №   §   \   ⌦                        ⌫   =   ;           ·
                            __  __  SYM   __  __  __
       */
        XX,  KC_GRV, RU_MDASH,   KC_DLR, KC_CIRC,      XX,            XX,  KC_EXLM,  KC_ASTR,  KC_PIPE, KC_AMPR,  KC_SLASH,
        XX,      XX,  KC_TILD,    KC_AT, KC_HASH,      XX,            XX,  KC_QUES,  KC_COLN,  KC_PLUS, KC_PERC,  KC_MINUS,
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
   * Mouse layer.
   *
   * Activated by Leader,m (sticky). Exit with Esc (shift+space combo) or Leader,space.
   *
   * Left hand can apply modifiers, to perform shift+click, or ctrl+wheelup.
   */
  [L_MOUSE] = LAYOUT_split_3x6_3(/*
        __  a2  __  __  __  __                       __  w↑  ↑  w↓  __  __
        __  a1  __  __  __  __                       __  <-  c  ->  b2  __
        __  a0  __  __  __  __                       __  b3  ↓  __  __  __
                             __  __  __     __  __  __
       */
        XX,      XX,        XX,       XX,      XX,  XX,       XX, MS_WHLU,  MS_UP  ,  MS_WHLD,      XX,  XX,
        XX,      XX,   MS_ACL2,  MS_ACL0, MS_ACL1,  XX,       XX, MS_LEFT,  MS_BTN1,  MS_RGHT, MS_BTN2,  __,
        XX,      XX,        XX,       XX,      XX,  XX,       XX, MS_BTN3,  MS_DOWN,       XX,      XX,  XX,

                                   __ ,    __ ,   __ ,         __ ,  __ ,  __
  ),
};
