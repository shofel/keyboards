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

/* SM layer - unicode */
// #define SM_LU
/* SM modifier unicode */
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

