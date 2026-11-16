/* Off-target unit test for toggle_select.h — the cancel-on-reselect decision for
 * the leader-activated toggle layers. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/toggle \
 *       -o /tmp/test_toggle_select tools/test_toggle_select.c && /tmp/test_toggle_select
 * or:  make test-toggle
 *
 * The rule: cancel-on-reselect applies only to the non-Russian overlays (they
 * are mods, reversible by a second tap). The Russian layers are stable language
 * switches — re-selecting one never cancels; you leave Russian with leader,e /
 * leader,space. So the decision reduces to: cancel iff the same non-Russian
 * layer is re-selected.
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
    CHECK(toggle_reselect_cancels(false, false) == false,
          "different non-ru layer -> switch");
    CHECK(toggle_reselect_cancels(false, true) == false,
          "different ru layer -> switch");

    /* Re-selecting the same non-Russian overlay cancels (mod, second-tap off). */
    CHECK(toggle_reselect_cancels(true, false) == true,
          "same non-ru overlay -> cancel");

    /* Re-selecting the active Russian layer never cancels — it stays (or, with a
     * different backend, the caller switches the backend). You leave Russian with
     * leader,e / leader,space, not by re-pressing the same switch. */
    CHECK(toggle_reselect_cancels(true, true) == false,
          "same ru layer -> stay (not cancel)");

    if (failures) {
        printf("\n%d toggle_select test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll toggle_select tests passed.\n");
    return 0;
}
