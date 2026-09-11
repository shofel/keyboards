/* Off-target unit test for toggle_select.h — the cancel-on-reselect decision for
 * the leader-activated toggle layers. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/toggle \
 *       -o /tmp/test_toggle_select tools/test_toggle_select.c && /tmp/test_toggle_select
 * or:  make test-toggle
 *
 * The rule (mirroring a one-shot's second-tap cancel): re-selecting the switcher
 * that is already active turns it off. A switcher's identity is its layer — and,
 * for the Russian layers, also its backend, so that switching vim -> compose
 * while Russian is live still switches rather than cancelling.
 */
#include <stdio.h>
#include "toggle_select.h"

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
    /* Selecting a different layer always switches, never cancels. */
    CHECK(toggle_reselect_cancels(false, false, false) == false,
          "different non-ru layer -> switch");
    CHECK(toggle_reselect_cancels(false, true, true) == false,
          "different ru layer -> switch (even same backend)");

    /* Re-selecting the same non-Russian layer cancels; backend is irrelevant. */
    CHECK(toggle_reselect_cancels(true, false, false) == true,
          "same non-ru layer -> cancel (backend ignored)");
    CHECK(toggle_reselect_cancels(true, false, true) == true,
          "same non-ru layer -> cancel");

    /* Russian: cancel only when the backend also matches. */
    CHECK(toggle_reselect_cancels(true, true, true) == true,
          "same ru layer + same backend -> cancel");
    CHECK(toggle_reselect_cancels(true, true, false) == false,
          "same ru layer, different backend -> switch backend, not cancel");

    if (failures) {
        printf("\n%d toggle_select test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll toggle_select tests passed.\n");
    return 0;
}
