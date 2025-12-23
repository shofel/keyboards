/**
 * UCIS Emoji Support - Introspection
 *
 * The ucis_symbol_table array is defined here so it's available to keymaps.
 */

#include "introspection.h"

const ucis_symbol_t ucis_symbol_table[] = UCIS_TABLE(
  UCIS_SYM("tulip", 0x1F337),      // 🌷
  UCIS_SYM("rose", 0x1F339),       // 🌹
  UCIS_SYM("cherry", 0x1F338),     // 🌸
  UCIS_SYM("hibiscus", 0x1F33A),   // 🌺
  UCIS_SYM("sunflower", 0x1F33B),  // 🌻
  UCIS_SYM("daisy", 0x1F33C),      // 🌼
  UCIS_SYM("thumbup", 0x1F44D),    // 👍
  UCIS_SYM("ok", 0x1F44C),         // 👌
  UCIS_SYM("think", 0x1F914),      // 🤔
  UCIS_SYM("monocle", 0x1F9D0),    // 🧐
  UCIS_SYM("handshake", 0x1F91D)   // 🤝
);

