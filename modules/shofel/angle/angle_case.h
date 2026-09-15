/* angle_case.h — which glyph the angle combos (KK_LANGLE / KK_RANGLE) emit.
 *
 * Pure logic, no QMK deps, so tools/test_angle_case.c can exercise it on the
 * host. keymap.c owns the actual emission; this owns only the choice.
 *
 * One pair of combos carries two glyph pairs: the ASCII angles `< >` and the
 * Russian quotation marks `« »`. Shift picks between them the same way on every
 * layer — unshifted gives `< >`, Shift gives the guillemet. The choice does not
 * depend on whether a Russian layer is live: a key that means one thing
 * everywhere beats one that flips with the mode, even though it costs a Shift for
 * the guillemets Russian prose leans on.
 */
#pragma once

#include <stdbool.h>

/* True -> emit the guillemet (« or »). False -> emit the ASCII angle (< or >). */
static inline bool angle_emits_guillemet(bool shifted) {
    return shifted;
}
