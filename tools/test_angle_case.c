/* Off-target unit test for angle_case.h — which glyph the angle combos emit.
 * No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/angle \
 *       -o /tmp/test_angle_case tools/test_angle_case.c && /tmp/test_angle_case
 * or:  make test-angle
 *
 * The whole behaviour is a four-row truth table, and the only way to get it
 * wrong is to invert the polarity — which is precisely what this change does on
 * purpose for one of the two states. So pin all four rows.
 *
 * Latin: shift picks the guillemet (the rare glyph costs the extra press).
 * Russian: inverted, because « » is what Russian prose quotes with and < > is
 * the rarity there.
 */
#include <stdio.h>
#include "angle_case.h"

static int failures = 0;

#define CHECK(cond, msg)                        \
    do {                                        \
        if (cond) {                             \
            printf("ok   %s\n", msg);           \
        } else {                                \
            printf("FAIL %s\n", msg);           \
            failures++;                         \
        }                                       \
    } while (0)

int main(void) {
    /* Latin (RU layer off) — unchanged from how the board has always behaved. */
    CHECK(angle_emits_guillemet(false, false) == false,
          "latin, unshifted -> < >");
    CHECK(angle_emits_guillemet(true, false) == true,
          "latin, shifted   -> guillemet");

    /* Russian (RU layer live) — inverted. */
    CHECK(angle_emits_guillemet(false, true) == true,
          "russian, unshifted -> guillemet");
    CHECK(angle_emits_guillemet(true, true) == false,
          "russian, shifted   -> < >");

    /* The two states must genuinely differ, or the inversion is a no-op that
     * would still satisfy any single row above. */
    CHECK(angle_emits_guillemet(false, false) != angle_emits_guillemet(false, true),
          "unshifted output differs between latin and russian");
    CHECK(angle_emits_guillemet(true, false) != angle_emits_guillemet(true, true),
          "shifted output differs between latin and russian");

    if (failures) {
        printf("\n%d angle_case test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll angle_case tests passed.\n");
    return 0;
}
