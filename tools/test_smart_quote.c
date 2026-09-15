/* Off-target unit test for smart_quote.h — what the quote combo (g+v) produces.
 * No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/quote \
 *       -o /tmp/test_smart_quote tools/test_smart_quote.c && /tmp/test_smart_quote
 * or:  make test-quote
 *
 * On a Russian layer the combo is a smart guillemet — « » with the cursor placed
 * between them; on the Latin layers it stays the ASCII double quote. This pins
 * only that layer-scoped choice; the actual emission lives in keymap.c.
 */
#include <stdio.h>
#include "smart_quote.h"

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
    CHECK(smart_quote_kind(false) == QUOTE_ASCII,
          "latin layer -> ascii double quote");
    CHECK(smart_quote_kind(true) == QUOTE_GUILLEMET_PAIR,
          "russian layer -> guillemet pair, cursor between");

    if (failures) {
        printf("\n%d smart_quote test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll smart_quote tests passed.\n");
    return 0;
}
