/**
 * A keymap for 6x3_3 layout.
 * Originally made for Cantor; and hopefully useful for a Dactyl.
 */

#include <stdint.h>
#include QMK_KEYBOARD_H
#include "introspection.h"
#include "modules/shofel/unicode_ru/introspection.h"
#include "modules/getreuer/orbital_mouse/introspection.h"
#include "modules/shofel/leader/leader_fsm.h"

/*
 * Runtime debug logging — all off by default.
 * Flip any flag to `true` (and build with CONSOLE_ENABLE) to stream the
 * corresponding events to `qmk console`. Kept as a ready-to-use toggle hook.
 */
/* Timeoutless leader (defined with the leader machinery further down). */
static void leader_build(void);   // build the matcher table once at boot
static void lead_abort(void);     // drop an in-flight sequence (reset escape hatch)

void keyboard_post_init_user(void) {
  debug_enable   = false;
  debug_matrix   = false;
  debug_keyboard = false;
  debug_mouse    = false;
  leader_build();
}

/* Fancy looking spare keys. */
#define __ KC_TRNS
#define XX KC_NO

enum my_keycodes {
  KK_RIGHT_ARROW = SAFE_RANGE,  // emits "->"
  KK_FAT_RIGHT_ARROW,           // emits "=>"
  KK_FAT_LEFT_ARROW,            // emits "<="
  KK_LEFT_ARROW,                // emits "<-"
  KK_LANGLE,  // « when shifted, < otherwise
  KK_RANGLE,  // » when shifted, > otherwise

  /* The two outer-thumb leader keys. Distinct keycodes (not a single shared
   * QK_LEAD) so the reset chord — both of them at once — is an unambiguous
   * combo: stock QMK matches combos by keycode, so a {QK_LEAD, QK_LEAD} combo
   * could never fire. Both arm the leader in process_record_user. */
  KK_LEAD_L,
  KK_LEAD_R,

  /* One-shot trigger keys */
  OS_CTL,
  OS_ALT,
  OS_GUI,
  OS_SFT,

  /* Mouse mode switch (live on the mouse layers) */
  KK_MM_POLAR,
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
    lead_abort();   // timeoutless leader has no clock — this is its escape hatch
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
 * above it (mods, backspace, layer toggles) or below it (brackets). On choc the
 * two keys sit close enough that pressing both at once is almost like pressing a
 * single key between them — an easy, unambiguous placement (see the Modifiers
 * note in the DESIGN block).
 * The top-row pairs (home + above) feel best; the bottom-row pairs (home +
 * below) are more of a reach — which is why the highest-value combos (mods,
 * backspace, primary num-layer access) take the top slots and brackets the
 * bottom.
 */

/* Hit both middle thumb keys for esc. */
const uint16_t PROGMEM esc_combo[]      = {KK_SHIFT, KC_SPACE, COMBO_END};
/* Arrows — mirror-symmetric, bottom row: index+middle = fat (=> / <=), index+ring = thin (-> / <-);
   left hand points left, right points right. It's strange, but consistent :D */
const uint16_t PROGMEM fat_right_arrow_combo[] = {KC_H, KC_M, COMBO_END};
const uint16_t PROGMEM fat_left_arrow_combo[]  = {KC_DOT, KC_W, COMBO_END};
const uint16_t PROGMEM thin_left_arrow_combo[] = {KC_X, KC_W, COMBO_END};
/* One-handed Esc: right inner-index, top+home (vertical same-column, misfire-free).
 * Was two top-row combos (c+u / f+d), but c+u bridged the Ctrl (s+c) and Nav (e+u)
 * combos, so a fast c-s-u-e roll dumped literal "cs"; a vertical combo shares no key
 * with them. */
const uint16_t PROGMEM esc_qb_combo[] = {KC_Q, KC_B, COMBO_END};
/* All three thumb keys of one half at once -> bootloader (for flashing). */
const uint16_t PROGMEM boot_combo_left[]  = {KK_LEAD_L, KK_SHIFT, KK_SYMBO, COMBO_END};
const uint16_t PROGMEM boot_combo_right[] = {KK_RET, KC_SPACE, KK_LEAD_R, COMBO_END};
/* On each half, its outer + inner thumb at once -> reboot the keyboard (so
 * either half can reboot on its own). Each is a 2-key subset of that half's
 * 3-key boot combo; stock QMK's overlaps() drops the shorter combo when the
 * longer one also completes within COMBO_TERM, so a firm 3-thumb press still
 * bootloads and a 2-thumb press reboots. */
const uint16_t PROGMEM reset_combo_left[]  = {KK_LEAD_L, KK_SYMBO, COMBO_END};
const uint16_t PROGMEM reset_combo_right[] = {KK_RET, KK_LEAD_R, COMBO_END};
/* Thin right arrow: bottom row, right hand, index+ring. */
const uint16_t PROGMEM right_arrow_combo[] = {KC_H, KC_K, COMBO_END}; // ->
/* [(<>)] */
const uint16_t PROGMEM square_left_combo[]  = {KC_S, KC_W, COMBO_END};
const uint16_t PROGMEM square_right_combo[] = {KC_N, KC_H, COMBO_END};
const uint16_t PROGMEM brace_left_combo[]   = {KC_E, KC_DOT, COMBO_END};
const uint16_t PROGMEM brace_right_combo[]  = {KC_T, KC_M, COMBO_END};
const uint16_t PROGMEM angle_left_combo[]   = {KC_G, KC_Z, COMBO_END};
const uint16_t PROGMEM angle_right_combo[]  = {KC_B, KC_P, COMBO_END};
/* Curly braces: ring finger, home+below — left = { , right = } . */
const uint16_t PROGMEM curly_left_combo[]   = {KC_O, KC_X, COMBO_END};
const uint16_t PROGMEM curly_right_combo[]  = {KC_R, KC_K, COMBO_END};
/* Vertical combos for mods */
const uint16_t PROGMEM lctl_combo[] = {KC_S, KC_C, COMBO_END};
const uint16_t PROGMEM llt2_combo[] = {KC_E, KC_U, COMBO_END};
const uint16_t PROGMEM lalt_combo[] = {KC_O, KC_COMM, COMBO_END};
const uint16_t PROGMEM lgui_combo[] = {KC_A, KC_QUOT, COMBO_END};
const uint16_t PROGMEM rctl_combo[] = {KC_N, KC_F, COMBO_END};
const uint16_t PROGMEM rlt2_combo[] = {KC_T, KC_D, COMBO_END};
const uint16_t PROGMEM ralt_combo[] = {KC_R, KC_L, COMBO_END};
const uint16_t PROGMEM rgui_combo[] = {KC_I, KC_Y, COMBO_END};
/* Double-quote ": left inner-index, top+home (took over the old backspace slot; backspace now lives on SYM). */
const uint16_t PROGMEM dquo_combo[] = {KC_G, KC_V, COMBO_END};
/* One-shot FKEYS/SYS layer: right pinky, outer column, top+home.
 * SYM no longer needs this combo: it is reached via the left thumb (KK_SYMBO)
 * and the RET layer-tap (right inner thumb). */
const uint16_t PROGMEM fkeys_combo[] = {KC_SLASH, KC_MINUS, COMBO_END};

/* Indices for all combos (designated initializers) */
enum combos {
  CMB_ESC,
  CMB_FAT_RIGHT_ARROW,
  CMB_FAT_LEFT_ARROW,
  CMB_THIN_LEFT_ARROW,
  CMB_ESC_QB,

  CMB_BOOT_L,
  CMB_BOOT_R,

  CMB_RESET_L,
  CMB_RESET_R,

  CMB_RIGHT_ARROW,

  CMB_SQ_L,
  CMB_SQ_R,
  CMB_BR_L,
  CMB_BR_R,
  CMB_ANG_L,
  CMB_ANG_R,
  CMB_CURLY_L,
  CMB_CURLY_R,

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
  [CMB_ESC]             = COMBO(esc_combo, KC_ESC),
  [CMB_FAT_RIGHT_ARROW] = COMBO(fat_right_arrow_combo, KK_FAT_RIGHT_ARROW),
  [CMB_FAT_LEFT_ARROW]  = COMBO(fat_left_arrow_combo,  KK_FAT_LEFT_ARROW),
  [CMB_THIN_LEFT_ARROW] = COMBO(thin_left_arrow_combo, KK_LEFT_ARROW),
  [CMB_ESC_QB]          = COMBO(esc_qb_combo, KC_ESC),

  [CMB_BOOT_L]     = COMBO(boot_combo_left,  QK_BOOT),
  [CMB_BOOT_R]     = COMBO(boot_combo_right, QK_BOOT),

  [CMB_RESET_L]    = COMBO(reset_combo_left,  QK_REBOOT),
  [CMB_RESET_R]    = COMBO(reset_combo_right, QK_REBOOT),

  [CMB_RIGHT_ARROW]= COMBO(right_arrow_combo, KK_RIGHT_ARROW),

  /* ([{<>}])  {} now have their own combos; <>/« » resolved by KK_LANGLE/KK_RANGLE */
  [CMB_SQ_L]       = COMBO(square_left_combo , KC_LBRC),
  [CMB_SQ_R]       = COMBO(square_right_combo, KC_RBRC),
  [CMB_BR_L]       = COMBO(brace_left_combo, KC_LPRN),
  [CMB_BR_R]       = COMBO(brace_right_combo, KC_RPRN),
  [CMB_ANG_L]      = COMBO(angle_left_combo, KK_LANGLE),
  [CMB_ANG_R]      = COMBO(angle_right_combo, KK_RANGLE),
  [CMB_CURLY_L]    = COMBO(curly_left_combo, KC_LCBR),
  [CMB_CURLY_R]    = COMBO(curly_right_combo, KC_RCBR),

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

/* Leader — timeoutless, fire-on-unique-match. QMK's stock leader is disabled
 * (LEADER_ENABLE=no); the machinery below captures keys and fires the moment a
 * sequence is uniquely matched, with no per-key timeout. */

/* Leader sequences — data so tools/gen_layer_schemes.py can extract them into
 * docs/reference.md. k2 == KC_NO marks a one-key sequence; doc strings appear
 * verbatim in the generated reference. Mirror pairs (s·n, w·h, m·.) are two
 * rows sharing an action, so either hand can trigger them. */
typedef struct {
  uint16_t k1, k2;
  void (*act)(void);
  const char *doc;
} leader_seq_t;

/* Ru compose is the default backend; see the unicode_ru module. */
static void lead_ru(void)       { ru_backend = RU_BACKEND_COMPOSE; toggle_enable(L_RUSSIAN); }
static void lead_vim(void)      { ru_backend = RU_BACKEND_VIM; toggle_enable(L_RUSSIAN); }
static void lead_win(void)      { ru_backend = RU_BACKEND_WINDOWS; toggle_enable(L_RUSSIAN); }
static void lead_en(void)       { ru_backend = RU_BACKEND_COMPOSE; toggle_disable(); }
static void lead_reset(void)    { toggle_reset(); }
static void lead_fkeys(void)    { toggle_enable(L_FKEYS_SYS); }
static void lead_mouse(void)    { toggle_enable(L_MOUSE); }
static void lead_num(void)      { toggle_enable(L_NUM_NAV); }
static void lead_lira(void)     { ru_emit_glyph("$l", 0x20BA); }
static void lead_rub(void)      { ru_emit_glyph("$r", 0x20BD); }
static void lead_eur(void)      { ru_emit_glyph("$e", 0x20AC); }
static void lead_del_all(void)  { tap_code16(LCTL(KC_A)); tap_code16(KC_DEL); }
static void lead_del_line(void) { tap_code16(LSFT(KC_HOME)); tap_code16(KC_DEL); }
static void lead_del_word(void) { tap_code16(LCTL(KC_BSPC)); }
static void lead_kitty(void)    { tap_code16(LGUI(KC_T)); }
static void lead_pscr(void)     { tap_code(KC_PSCR); }

static const leader_seq_t leader_seqs[] = {
  /* Russian backend selection, one bare key each (prefix-free single-key seqs):
   * r = compose (the default; rolling-safe, host-wide), v = vim, w = Windows. */
  {KC_R,     KC_NO, lead_ru,       "Russian — compose backend (default; rolling-safe, host-wide)"},
  {KC_V,     KC_NO, lead_vim,      "Russian — vim backend (i_CTRL-V U, no host setup)"},
  {KC_W,     KC_NO, lead_win,      "Russian — Windows backend (Alt+numpad hex; EnableHexNumpad)"},
  {KC_E,     KC_NO, lead_en,       "back to English (drop the toggle layer)"},
  {KC_SPACE, KC_NO, lead_reset,    "disable any toggle layer, cancel one-shots"},
  {KC_F,     KC_NO, lead_fkeys,    "F-keys / system layer (sticky)"},
  {KC_M,     KC_NO, lead_mouse,    "mouse layer, polar mode (mirror pair m·.)"},
  {KC_DOT,   KC_NO, lead_mouse,    "mouse layer, polar mode (mirror pair m·.)"},
  {KC_T,     KC_NO, lead_num,      "num/nav layer (sticky)"},
  /* Currencies moved off the M/. mouse prefix (bare M was a prefix of M,L/M,R/M,E
   * — illegal under a timeoutless leader). New home: leader,u,{l|r|e} (u =
   * currency unit). Rare symbols, so the m·. mirror is dropped; single-hand. */
  {KC_U,     KC_L,  lead_lira,     "₺ lira"},
  {KC_U,     KC_R,  lead_rub,      "₽ ruble"},
  {KC_U,     KC_E,  lead_eur,      "€ euro"},
  {KC_D,     KC_A,  lead_del_all,  "delete all (Ctrl+A, Del)"},
  {KC_D,     KC_U,  lead_del_line, "delete to line start (like Ctrl-U)"},
  {KC_D,     KC_W,  lead_del_word, "delete word (Ctrl+Backspace)"},
  {KC_K,     KC_NO, lead_kitty,    "kitty terminal (Gui+T)"},
  {KC_P,     KC_NO, lead_pscr,     "Print Screen"},
};

/* Emoji: leader,{a|i},<sel> -> a flower or reaction, host-wide, via the compose
 * backend (Compose + a private '@' code; the code<->glyph map lives in
 * tools/gen_unicode_compose.py and the generated ~/.XCompose). `a` (left home)
 * and `i` (right home) are a mirror pair, so either hand triggers it. Always
 * compose — emoji go to chat/host apps, so the vim backend is irrelevant. File
 * scope so both the table builder below and tools/gen_layer_schemes.py read it. */
static const struct { uint16_t sel; const char *code; } emoji_seqs[] = {
  {KC_T, "@t"},  // 🌷 tulip
  {KC_R, "@r"},  // 🌹 rose
  {KC_C, "@c"},  // 🌸 cherry
  {KC_H, "@h"},  // 🌺 hibiscus
  {KC_S, "@s"},  // 🌻 sunflower
  {KC_D, "@d"},  // 🌼 daisy
  {KC_U, "@u"},  // 👍 thumbup
  {KC_O, "@o"},  // 👌 ok
  {KC_K, "@k"},  // 🤔 think
  {KC_M, "@m"},  // 🧐 monocle
  {KC_N, "@n"},  // 🤝 handshake
};

/* The matcher's view of the leader set: keys (lead_key_table) + the action each
 * fires (lead_acts), built once at boot from leader_seqs[] and emoji_seqs[]. An
 * action is either a plain fn() or, for emoji, a compose code. */
#define LEADER_SEQS_N  (sizeof(leader_seqs) / sizeof(leader_seqs[0]))
#define EMOJI_SEQS_N   (sizeof(emoji_seqs) / sizeof(emoji_seqs[0]))
#define LEAD_ENTRIES_N (LEADER_SEQS_N + 2 * EMOJI_SEQS_N)

typedef struct {
  void (*fn)(void);   // plain leader action, or NULL for an emoji entry
  const char *emoji;  // compose code for an emoji entry, or NULL
} lead_action_t;

static leader_seq_keys_t lead_key_table[LEAD_ENTRIES_N];
static lead_action_t     lead_acts[LEAD_ENTRIES_N];
static size_t            lead_entries_n = 0;

static void leader_build(void) {
  lead_entries_n = 0;
  for (size_t i = 0; i < LEADER_SEQS_N; i++) {
    leader_seq_keys_t *k = &lead_key_table[lead_entries_n];
    k->len     = (leader_seqs[i].k2 == KC_NO) ? 1 : 2;
    k->keys[0] = leader_seqs[i].k1;
    if (k->len == 2) { k->keys[1] = leader_seqs[i].k2; }
    lead_acts[lead_entries_n].fn    = leader_seqs[i].act;
    lead_acts[lead_entries_n].emoji = NULL;
    lead_entries_n++;
  }
  const uint16_t emoji_prefix[2] = {KC_A, KC_I};  // either home index triggers
  for (size_t i = 0; i < EMOJI_SEQS_N; i++) {
    for (int p = 0; p < 2; p++) {
      leader_seq_keys_t *k = &lead_key_table[lead_entries_n];
      k->len     = 2;
      k->keys[0] = emoji_prefix[p];
      k->keys[1] = emoji_seqs[i].sel;
      lead_acts[lead_entries_n].fn    = NULL;
      lead_acts[lead_entries_n].emoji = emoji_seqs[i].code;
      lead_entries_n++;
    }
  }
}

static leader_capture_t lead_cap;

static void lead_begin(void) {
  leader_capture_begin(&lead_cap);
  leader_suspend();   // mask toggle layers so tokens read the base layout
}

/* End capture and un-mask layers. Safe to call when not capturing. */
static void lead_abort(void) {
  if (lead_cap.active) {
    lead_cap.active = false;
    leader_resume();
  }
}

static void lead_fire(size_t idx) {
  if (lead_acts[idx].fn) {
    lead_acts[idx].fn();
  } else if (lead_acts[idx].emoji) {
    ru_compose_emit_code(lead_acts[idx].emoji);
  }
}

/* Feed one captured key. Fires on a unique match, aborts on no match, else keeps
 * waiting. leader_resume() runs before the action (as leader_end_user did) so
 * the action's layer/toggle changes actually apply. */
static void lead_feed(uint16_t keycode) {
  size_t idx = 0;
  switch (leader_capture_feed(&lead_cap, keycode, lead_key_table, lead_entries_n, &idx)) {
    case LEADER_MATCH_UNIQUE:
      leader_resume();
      lead_fire(idx);
      break;
    case LEADER_MATCH_NONE:
    case LEADER_MATCH_AMBIGUOUS:
      leader_resume();   // abort: un-mask, act on nothing
      break;
    case LEADER_MATCH_PARTIAL:
      break;             // keep capturing
  }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  /* Timeoutless leader takes priority: while capturing, each PRESS is a sequence
   * token fed to the matcher and swallowed (never typed nor seen by a layer).
   * Combos resolve before us, so a combo's OUTPUT keycode is what gets captured
   * — the basis for the RU-via-combos work.
   *
   * Only presses are swallowed. RELEASES fall through to normal processing: a key
   * registered BEFORE the leader was armed (a letter still held from a fast roll,
   * or a held OS_SFT/LT thumb) must get its release or it sticks. A swallowed
   * token key's own release is harmless — its press never registered, so QMK's
   * source-layer cache resolves the release to a no-op unregister. */
  if (lead_cap.active && record->event.pressed) {
    lead_feed(keycode);
    return false;
  }

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
    case KK_FAT_LEFT_ARROW:
      if (record->event.pressed) {
        SEND_STRING("<=");
      }
      return false;
    case KK_LEFT_ARROW:
      if (record->event.pressed) {
        SEND_STRING("<-");
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

    /* The two outer thumbs are custom leader keys (distinct so the reset combo
     * can tell them apart); both arm the timeoutless leader. lead_begin() sets
     * the capture flag; the intercept at the top of this function then captures
     * every following key. Returning false swallows the arming key itself. */
    case KK_LEAD_L:
    case KK_LEAD_R:
      if (record->event.pressed) { lead_begin(); }
      return false;

    /* Mouse mode switch */
    case KK_MM_POLAR:
      if (record->event.pressed) { toggle_enable(L_MOUSE); }
      return false;

    default:
      break;
  }

  return true;
}


/*
 * DESIGN — the layout rationale. Everything between the begin/end markers
 * below is extracted verbatim (as markdown) into the "Design" section of
 * docs/reference.md by tools/gen_layer_schemes.py. Edit it here, then
 * `make gen-docs`.
 *
 * @design-begin
 * ### Base layer
 *
 * It's the [BOO layout](https://ballerboo.github.io/boolayout/) — Dvorak
 * modified for more rollover.
 *
 * ### Modifiers
 *
 * Vertical combo mods: a home-row key plus the key just above it (or below it,
 * for brackets). With choc's tight spacing, pressing both keys of a column at
 * once is almost as easy as pressing one — no finger-flattening, just a
 * different finger placement, so a vertical combo feels like an extra key
 * between the two. Ordinary typing aims at the key centers, so it never fires
 * one by accident. The top-row pairs (home + above) feel best; the bottom-row
 * pairs (home + below) are more of a reach — which is why the highest-value
 * combos (mods, backspace, primary num-layer access) take the top slots and
 * brackets the bottom.
 *
 * ### Thumbs
 *
 * On the Cantor the middle thumb is the most comfortable key, so the two most
 * used thumb actions live there: Space (right middle) and Shift (left middle).
 * Esc is both middle thumbs at once — the strongest pair. The right inner thumb
 * is a SYMBOLS layer-tap (tap = Enter, hold = SYM); Enter lives there since it
 * is far less frequent than Space.
 *
 * Punctuation on the SYM layer lives mostly on the right hand — the same hand
 * as the right-thumb Space — so a symbol→Space sequence is a same-hand roll:
 * tap the left-thumb one-shot SYM, type the symbol with the right hand, then
 * roll into Space. (`,` `.` stay on the left of the base layer.)
 *
 * ### Unicode input
 *
 * With a non-qwerty layout, switching language in the OS is not enough: an OS
 * language map assumes qwerty (q→й, w→ц), so on a non-qwerty base the map is
 * wrong. And with two keyboards attached — a qwerty laptop and the BOO board —
 * the OS can't tell which map to apply. So the laptop switches language with
 * `win+space`, and the QMK board carries its own Russian layer instead.
 *
 * Three backends, selected by leader (see the unicode_ru module): compose mode
 * (`leader,r`, default, rolling-safe, host-wide), vim mode (`leader,v`), which
 * emits vim's native `i_CTRL-V U <hex>` so Cyrillic types inside vim/neovim with
 * no host compose setup, and Windows mode (`leader,w`, Alt+numpad hex; needs
 * EnableHexNumpad). All are emitted in userspace, so the firmware no longer
 * needs the out-of-tree UNICODE_MODE_VIM patch (QMK PR #25188).
 *
 * ### Symmetry
 *
 * The layout aims to be mirror-symmetric: the left and right hands hold matching
 * structures, and the finger-pair picks the variant. Brackets illustrate it most
 * cleanly — one finger, home+below, left hand opens and right closes. Arrows on
 * the bottom row follow the same logic: index+middle gives the fat arrow (=> / <=),
 * index+ring the thin one (-> / <-), and the direction follows the hand (left
 * points left, right points right). Reset/cancel (drop toggle layers, cancel
 * one-shots) is `leader,space`. Esc sits apart on a vertical same-column combo (right
 * inner-index, top+home) plus the two-thumb chord — it was moved off c+u/f+d because
 * those bridged the Ctrl and Nav combos into misfires.
 *
 * ### Key comfort scores
 *
 * Layout-wide ergonomic weights, higher = easier (scale 0-9). Combos resolve
 * from layer 0, so one map serves every layer. This drives frequency-first
 * symbol placement: rank symbols by how often you type them, rank free keys by
 * score, then match highest-to-highest.
 *
 * ```
 *    pinky2 pinky  ring   mid  index  inner | inner  index   mid   ring  pinky pinky2
 *       1     4     6      8     6      2   |   2      6      8      6     4     3
 *       0     5     7      9     9      3   |   3      9      9      7     5     3
 *       0     1     4      5     6      3   |   3      6      5      4     1     1
 *                    · reserved for layer/mod keys, not symbol slots ·
 * ```
 * @design-end
 */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  /**
   * Base layer — the [BOO layout](https://ballerboo.github.io/boolayout/):
   * Dvorak modified for more rollover.
   */
  [L_BOO] = LAYOUT_split_3x6_3(/* GENERATED scheme — edit the array, then `make gen-docs`.
       ·  '  ,  u  c  v        q  f  d  l  y  /
       ·  a  o  e  s  g        b  n  t  r  i  -
       ·  ·  x  .  w  z        p  h  m  k  j  ·
         LEAD  sft  SYM        ret  spc  LEAD
  */
           XX , KC_QUOT, KC_COMM,    KC_U,   KC_C,  KC_V,     KC_Q,  KC_F,  KC_D,  KC_L,  KC_Y,   KC_SLASH,
           XX ,    KC_A,    KC_O,    KC_E,   KC_S,  KC_G,     KC_B,  KC_N,  KC_T,  KC_R,  KC_I,   KC_MINUS,
           XX ,     XX,    KC_X,  KC_DOT,   KC_W,  KC_Z,     KC_P,  KC_H,  KC_M,  KC_K,  KC_J,   XX,

                         KK_LEAD_L , KK_SHIFT , KK_SYMBO,       KK_RET , KC_SPACE, KK_LEAD_R
  ),

  /**
   * Russian layer.
   *
   * Activate and deactivate with leader seqs.
   */
  [L_RUSSIAN] = LAYOUT_split_3x6_3(/* GENERATED scheme — edit the array, then `make gen-docs`.
       ё   й  ц  у  к  е        н  г  ш  щ  з  х
       __  ф  ы  в  а  п        р  о  л  д  ж  э
       __  я  ч  с  м  и        т  ь  б  ю  .  ъ
              __  __  __        __  __  __
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
   * - `"`  — the left inner-index top+home combo (KC_DQUO).
   * - `« »` — the angle combos via KK_LANGLE / KK_RANGLE (unshifted = `<` `>`).
   * - `?` is suppressed from base Shift+/, so SYM is its one canonical home.
   *
   * Other combos still work here: `=>` `->`, brackets, mods.
   */
  [L_SYMBOLS] = LAYOUT_split_3x6_3(/* GENERATED scheme — edit the array, then `make gen-docs`.
       ·  `  —  $  ^  ·        ·  !  *  |  &  /
       ·  ·  ~  @  #  ·        ·  :  ?  +  %  -
       ·  ·  №  §  \  ⌦        ⌫  =  ;  ·  ·  ·
            __  __  SYM        __  __  __
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
  [L_NUM_NAV] = LAYOUT_split_3x6_3(/* GENERATED scheme — edit the array, then `make gen-docs`.
       ·  '  7  8  9  /        `  pg↑  ↑  pg↓  ·  __
       ·  0  4  5  6  :        ⇤  ←    ⏎  →    ⇥  __
       ·  ,  1  2  3  .        ·  ⮀    ↓  ·    ·  ·
             __  __  __        __  __  __
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
   * one-shot via the right-pinky outer-column combo (top+home).
   */
  [L_FKEYS_SYS] = LAYOUT_split_3x6_3(/* GENERATED scheme — edit the array, then `make gen-docs`.
       ·     F12  F7  F8  F9  ·        ·  br↑  vl↑  ·     DBG  ·
       ·     F11  F4  F5  F6  ·        ·  ·    ·    ·     ·    __
       boot  F10  F1  F2  F3  ·        ·  br↓  vl↓  mute  ·    boot
                     __  __  __        __  __  __
  */
        XX,  KC_F12,  KC_F7,  KC_F8,  KC_F9,     XX,       XX, KC_BRIU,  KC_VOLU,       XX,  DB_TOGG, XX,
        XX,  KC_F11,  KC_F4,  KC_F5,  KC_F6,     XX,       XX,      XX,       XX,       XX,       XX, __,
   QK_BOOT,  KC_F10,  KC_F1,  KC_F2,  KC_F3,     XX,       XX, KC_BRID,  KC_VOLD,  KC_MUTE,  XX, QK_BOOT,
                                __ ,    __ ,   __ ,         __ ,   __ ,   __
  ),

  /**
   * Mouse layer — POLAR mode (Orbital Mouse, getreuer/orbital_mouse). DEFAULT.
   *
   * Reached via Leader,m. Exit with Esc (shift+space combo) or Leader,space.
   *
   * Polar/heading control: fwd/bwd move the pointer forward/backward along a
   * heading; ←/→ steer that heading; slo/fst change speed while held. Left hand
   * can apply modifiers (shift+click, ctrl+wheelup).
   */
  [L_MOUSE] = LAYOUT_split_3x6_3(/* GENERATED scheme — edit the array, then `make gen-docs`.
       ·  ·  pol  ·  ·    ·        ·  w↑  fwd  w↓  ·   ·
       ·  ·  slo  ·  fst  ·        ·  ←   b1   →   b2  __
       ·  ·  ·    ·  ·    ·        ·  b3  bwd  ·   ·   ·
                 __  __  __        __  __  __
  */
        XX, XX, KK_MM_POLAR,      XX,           XX, XX,   XX, OM_W_U, OM_U   , OM_W_D,      XX, XX,
        XX, XX,     OM_SLOW,      XX,      OM_FAST, XX,   XX, OM_L  , OM_BTN1, OM_R   , OM_BTN2, __,
        XX, XX,          XX,      XX,           XX, XX,   XX, OM_BTN3, OM_D   ,      XX,      XX, XX,

                                   __ ,    __ ,   __ ,         __ ,  __ ,  __
  ),

};
