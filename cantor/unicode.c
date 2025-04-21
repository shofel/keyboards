/* Unicode */

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
};

const uint32_t PROGMEM unicode_map[] = {
  // Lowercase
  [RU_LC_A] = 0x430,
  [RU_LC_B] = 0x431,
  [RU_LC_V] = 0x432,
  [RU_LC_G] = 0x433,
  [RU_LC_D] = 0x434,
  [RU_LC_E] = 0x435,
  [RU_LC_YO] = 0x451,
  [RU_LC_ZH] = 0x436,
  [RU_LC_Z] = 0x437,
  [RU_LC_I] = 0x438,
  [RU_LC_Y] = 0x439,
  [RU_LC_K] = 0x43A,
  [RU_LC_L] = 0x43B,
  [RU_LC_M] = 0x43C,
  [RU_LC_N] = 0x43D,
  [RU_LC_O] = 0x43E,
  [RU_LC_P] = 0x43F,
  [RU_LC_R] = 0x440,
  [RU_LC_S] = 0x441,
  [RU_LC_T] = 0x442,
  [RU_LC_U] = 0x443,
  [RU_LC_F] = 0x444,
  [RU_LC_H] = 0x445,
  [RU_LC_TS] = 0x446,
  [RU_LC_CH] = 0x447,
  [RU_LC_SH] = 0x448,
  [RU_LC_SHCH] = 0x449,
  [RU_LC_HARD] = 0x44A,
  [RU_LC_YERU] = 0x44B,
  [RU_LC_SOFT] = 0x44C,
  [RU_LC_EE] = 0x44D,
  [RU_LC_YU] = 0x44E,
  [RU_LC_YA] = 0x44F,
  // Uppercase
  [RU_UC_A] = 0x410,
  [RU_UC_B] = 0x411,
  [RU_UC_V] = 0x412,
  [RU_UC_G] = 0x413,
  [RU_UC_D] = 0x414,
  [RU_UC_E] = 0x415,
  [RU_UC_YO] = 0x401,
  [RU_UC_ZH] = 0x416,
  [RU_UC_Z] = 0x417,
  [RU_UC_I] = 0x418,
  [RU_UC_Y] = 0x419,
  [RU_UC_K] = 0x41A,
  [RU_UC_L] = 0x41B,
  [RU_UC_M] = 0x41C,
  [RU_UC_N] = 0x41D,
  [RU_UC_O] = 0x41E,
  [RU_UC_P] = 0x41F,
  [RU_UC_R] = 0x420,
  [RU_UC_S] = 0x421,
  [RU_UC_T] = 0x422,
  [RU_UC_U] = 0x423,
  [RU_UC_F] = 0x424,
  [RU_UC_H] = 0x425,
  [RU_UC_TS] = 0x426,
  [RU_UC_CH] = 0x427,
  [RU_UC_SH] = 0x428,
  [RU_UC_SHCH] = 0x429,
  [RU_UC_HARD] = 0x42A,
  [RU_UC_YERU] = 0x42B,
  [RU_UC_SOFT] = 0x42C,
  [RU_UC_EE] = 0x42D,
  [RU_UC_YU] = 0x42E,
  [RU_UC_YA] = 0x42F,
};

// Combine lower and upper case letters as a single key
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

/* SM hold=layer tap=unicodemap */
#define SM_LU(macro_key, unicode_i, layer)  \
    case macro_key:                         \
    {                                       \
      switch (action) {                     \
        case SMTD_ACTION_TOUCH:             \
          break;                            \
                                            \
        case SMTD_ACTION_TAP:               \
          register_unicodemap(unicodemap_index(unicode_i));   \
          break;                            \
                                            \
        case SMTD_ACTION_HOLD:              \
          layer_on(L_NUM_NAV);              \
          break;                            \
                                            \
        case SMTD_ACTION_RELEASE:           \
          layer_off(L_NUM_NAV);              \
          break;                            \
      }                                     \
      break;                                \
    }                                       \
/* SM hold=modifier tap=unicodemap */
#define SM_MU(macro_key, unicode_i, mod)    \
    case macro_key:                         \
    {                                       \
      switch (action) {                     \
        case SMTD_ACTION_TOUCH:             \
          break;                            \
                                            \
        case SMTD_ACTION_TAP:               \
          register_unicodemap(unicodemap_index(unicode_i));   \
          break;                            \
                                            \
        case SMTD_ACTION_HOLD:              \
          layer_off(L_RUSSIAN);             \
          register_mods(MOD_BIT(mod));      \
          break;                            \
                                            \
        case SMTD_ACTION_RELEASE:           \
          layer_on(L_RUSSIAN);              \
          unregister_mods(MOD_BIT(mod));    \
          break;                            \
      }                                     \
      break;                                \
    }                                       \

