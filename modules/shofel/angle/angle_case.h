/* angle_case.h — which glyph the angle combos (KK_LANGLE / KK_RANGLE) emit.
 *
 * Pure logic, no QMK deps, so tools/test_angle_case.c can exercise it on the
 * host. keymap.c owns the actual emission; this owns only the choice.
 *
 * Two glyph pairs share one pair of combos: the ASCII angles `< >` and the
 * Russian quotation marks `« »`. Shift picks between them — but which one shift
 * costs extra depends on the language, because the rare glyph should be the
 * shifted one and rarity flips with the script:
 *
 *   Latin    `<` `>` are operators and JSX and comparisons — common;
 *            `«` `»` essentially never appear.
 *   Russian  `«` `»` are the standard quotation marks and open every quoted
 *            phrase; `<` `>` essentially never appear mid-Russian-prose.
 *
 * So the mapping is a XOR of the two booleans rather than a plain shift test.
 *
 * `russian_active` must reflect the LIVE layer, not merely the user's toggle:
 * holding Ctrl/Alt/GUI suspends L_RUSSIAN so Latin shortcuts keep working, and
 * angle brackets typed in that state are Latin ones. Shift deliberately does
 * NOT suspend the layer (see mod_ru_suspended in keymap.c), which is what makes
 * a shifted Russian angle combo reachable at all.
 */
#pragma once

#include <stdbool.h>

/* True -> emit the guillemet (« or »). False -> emit the ASCII angle (< or >). */
static inline bool angle_emits_guillemet(bool shifted, bool russian_active) {
    return shifted != russian_active;
}
