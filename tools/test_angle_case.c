/* Off-target unit test for angle_case.h — which glyph the angle combos emit.
 * No QMK deps, so it runs on the host with make test-angle.
 *
 * The helper deliberately has no language input: English and Russian must use
 * the same mapping. Both held and one-shot Shift are resolved by the caller.
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
    CHECK(angle_emits_guillemet(false) == false,
          "unshifted -> < > in every language");
    CHECK(angle_emits_guillemet(true) == true,
          "shifted -> guillemet in every language");

    if (failures) {
        printf("\n%d angle_case test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll angle_case tests passed.\n");
    return 0;
}
