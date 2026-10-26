/* angle_case.h — which glyph the angle combos (KK_LANGLE / KK_RANGLE) emit.
 *
 * Pure logic, no QMK deps, so tools/test_angle_case.c can exercise it on the
 * host. keymap.c owns the actual emission; this owns only the choice.
 *
 * In every language: plain combos emit `< >`; held or one-shot Shift emits
 * the quotation marks `« »`. The active language never changes this mapping.
 */
#pragma once

#include <stdbool.h>

/* True -> emit the guillemet (« or »). False -> emit the ASCII angle (< or >). */
static inline bool angle_emits_guillemet(bool shifted) {
    return shifted;
}
