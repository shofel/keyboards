/* Off-target unit test for angle_case.h — which glyph the angle combos emit.
 * No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/angle \
 *       -o /tmp/test_angle_case tools/test_angle_case.c && /tmp/test_angle_case
 * or:  make test-angle
 *
 * The glyph choice is stable on every layer: Shift alone picks between the two,
 * with no dependence on whether a Russian layer is live. Unshifted gives the
 * ASCII angle `< >`; Shift gives the guillemet `« »`. Two rows say all of it.
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
    CHECK(angle_emits_guillemet(false) == false, "unshifted -> < >");
    CHECK(angle_emits_guillemet(true)  == true,  "shifted   -> guillemet");

    if (failures) {
        printf("\n%d angle_case test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll angle_case tests passed.\n");
    return 0;
}
