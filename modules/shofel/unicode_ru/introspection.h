#pragma once

#include QMK_KEYBOARD_H

/* Unicode */
// LC is for lowercase
// UC is for uppercase
enum unicode_names {
  RU_LC_A,
  RU_LC_B,
  RU_LC_V,
  RU_LC_G,
  RU_LC_D,
  RU_LC_E,
  RU_LC_YO,
  RU_LC_ZH,
  RU_LC_Z,
  RU_LC_I,
  RU_LC_Y,
  RU_LC_K,
  RU_LC_L,
  RU_LC_M,
  RU_LC_N,
  RU_LC_O,
  RU_LC_P,
  RU_LC_R,
  RU_LC_S,
  RU_LC_T,
  RU_LC_U,
  RU_LC_F,
  RU_LC_H,
  RU_LC_TS,
  RU_LC_CH,
  RU_LC_SH,
  RU_LC_SHCH,
  RU_LC_YERU,
  RU_LC_SOFT,
  RU_LC_HARD,
  RU_LC_EE,
  RU_LC_YU,
  RU_LC_YA,

  RU_UC_A,
  RU_UC_B,
  RU_UC_V,
  RU_UC_G,
  RU_UC_D,
  RU_UC_E,
  RU_UC_YO,
  RU_UC_ZH,
  RU_UC_Z,
  RU_UC_I,
  RU_UC_Y,
  RU_UC_K,
  RU_UC_L,
  RU_UC_M,
  RU_UC_N,
  RU_UC_O,
  RU_UC_P,
  RU_UC_R,
  RU_UC_S,
  RU_UC_T,
  RU_UC_U,
  RU_UC_F,
  RU_UC_H,
  RU_UC_TS,
  RU_UC_CH,
  RU_UC_SH,
  RU_UC_SHCH,
  RU_UC_YERU,
  RU_UC_SOFT,
  RU_UC_HARD,
  RU_UC_EE,
  RU_UC_YU,
  RU_UC_YA,

  U_DOT,
  U_COMMA,

  U_MDASH,
  U_NUMERO,
  U_SECTION,
};

// unicode_map array is defined in introspection.c
extern const uint32_t PROGMEM unicode_map[];

/* Compose-mode Russian emission backend (see unicode_ru.c). The keymap toggles
 * `ru_compose_mode` (leader,c) and calls `ru_compose_process` at the top of
 * process_record_user; when it returns true the key was emitted via Compose. */
extern bool ru_compose_mode;
bool ru_compose_process(uint16_t keycode, keyrecord_t *record);
/* Emit `Compose + <code>` for a standalone glyph (e.g. « ») from the keymap. */
void ru_compose_emit_code(const char *code);

// Combine lower and upper case letter as a single `unicode pair` key
#define RU_A UP(RU_LC_A, RU_UC_A)
#define RU_B UP(RU_LC_B, RU_UC_B)
#define RU_V UP(RU_LC_V, RU_UC_V)
#define RU_G UP(RU_LC_G, RU_UC_G)
#define RU_D UP(RU_LC_D, RU_UC_D)
#define RU_E UP(RU_LC_E, RU_UC_E)
#define RU_YO UP(RU_LC_YO, RU_UC_YO)
#define RU_ZH UP(RU_LC_ZH, RU_UC_ZH)
#define RU_Z UP(RU_LC_Z, RU_UC_Z)
#define RU_I UP(RU_LC_I, RU_UC_I)
#define RU_Y UP(RU_LC_Y, RU_UC_Y)
#define RU_K UP(RU_LC_K, RU_UC_K)
#define RU_L UP(RU_LC_L, RU_UC_L)
#define RU_M UP(RU_LC_M, RU_UC_M)
#define RU_N UP(RU_LC_N, RU_UC_N)
#define RU_O UP(RU_LC_O, RU_UC_O)
#define RU_P UP(RU_LC_P, RU_UC_P)
#define RU_R UP(RU_LC_R, RU_UC_R)
#define RU_S UP(RU_LC_S, RU_UC_S)
#define RU_T UP(RU_LC_T, RU_UC_T)
#define RU_U UP(RU_LC_U, RU_UC_U)
#define RU_F UP(RU_LC_F, RU_UC_F)
#define RU_H UP(RU_LC_H, RU_UC_H)
#define RU_TS UP(RU_LC_TS, RU_UC_TS)
#define RU_CH UP(RU_LC_CH, RU_UC_CH)
#define RU_SH UP(RU_LC_SH, RU_UC_SH)
#define RU_SHCH UP(RU_LC_SHCH, RU_UC_SHCH)
#define RU_SOFT UP(RU_LC_SOFT, RU_UC_SOFT)
#define RU_YERU UP(RU_LC_YERU, RU_UC_YERU)
#define RU_HARD UP(RU_LC_HARD, RU_UC_HARD)
#define RU_EE UP(RU_LC_EE, RU_UC_EE)
#define RU_YU UP(RU_LC_YU, RU_UC_YU)
#define RU_YA UP(RU_LC_YA, RU_UC_YA)
#define RU_DOT UP(U_DOT, U_COMMA)

/* Typographic symbols used on the SYM layer (same glyph for tap and shifted). */
#define RU_MDASH UP(U_MDASH, U_MDASH)
#define RU_NUM   UP(U_NUMERO, U_NUMERO)
#define RU_SECT  UP(U_SECTION, U_SECTION)

