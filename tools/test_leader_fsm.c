/* Off-target unit test for leader_fsm.h — the pure fire-on-unique-match matcher
 * behind the timeoutless leader. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/leader \
 *       -o /tmp/test_leader_fsm tools/test_leader_fsm.c && /tmp/test_leader_fsm
 * or:  make test-leader-fsm
 *
 * Covers:
 *   - fire the instant the growing buffer is a unique complete match;
 *   - keep waiting while it is a proper prefix of some sequence (PARTIAL);
 *   - abort the instant it matches nothing (NONE) — no clock needed;
 *   - flag the ambiguous case (an exact match that is ALSO a prefix of another
 *     sequence) that only a non-prefix-free set can produce — this is exactly
 *     what check_leader_prefix.py forbids, so the matcher never fires it.
 */
#include <stdio.h>
#include "leader_fsm.h"

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

/* A small prefix-free table: R, V, E singles; (M,L) (M,R) two-key. Bare M is
 * only ever a prefix here — no standalone M — so the set is a prefix code. */
static const leader_seq_keys_t PREFIX_FREE[] = {
    {1, {'R'}},
    {1, {'V'}},
    {1, {'E'}},
    {2, {'M', 'L'}},
    {2, {'M', 'R'}},
    {2, {'D', 'A'}},
    {2, {'D', 'U'}},
};
static const size_t PREFIX_FREE_N = sizeof(PREFIX_FREE) / sizeof(PREFIX_FREE[0]);

/* A deliberately non-prefix-free table: bare M AND (M,L). */
static const leader_seq_keys_t WITH_COLLISION[] = {
    {1, {'M'}},
    {2, {'M', 'L'}},
};
static const size_t WITH_COLLISION_N = sizeof(WITH_COLLISION) / sizeof(WITH_COLLISION[0]);

static leader_match_t classify(const leader_seq_keys_t *t, size_t n,
                               const uint16_t *buf, uint8_t len, size_t *idx) {
    return leader_match(t, n, buf, len, idx);
}

int main(void) {
    size_t idx = 999;

    /* Single-key unique match fires immediately. */
    {
        uint16_t buf[] = {'R'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, buf, 1, &idx) == LEADER_MATCH_UNIQUE,
              "single R -> UNIQUE");
        CHECK(idx == 0, "single R -> idx 0");
    }

    /* A key that is only a prefix waits (no exact match yet). */
    {
        uint16_t buf[] = {'M'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, buf, 1, &idx) == LEADER_MATCH_PARTIAL,
              "M (prefix of M,L / M,R) -> PARTIAL");
    }

    /* Completing that prefix fires. */
    {
        uint16_t buf[] = {'M', 'L'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, buf, 2, &idx) == LEADER_MATCH_UNIQUE,
              "M,L -> UNIQUE");
        CHECK(idx == 3, "M,L -> idx 3");
    }

    /* A non-matching first key aborts at once. */
    {
        uint16_t buf[] = {'Z'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, buf, 1, &idx) == LEADER_MATCH_NONE,
              "Z -> NONE");
    }

    /* A valid prefix followed by a bad key aborts. */
    {
        uint16_t buf[] = {'M', 'Z'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, buf, 2, &idx) == LEADER_MATCH_NONE,
              "M,Z -> NONE");
    }

    /* Partial then unique across the D-family. */
    {
        uint16_t d[]  = {'D'};
        uint16_t du[] = {'D', 'U'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, d, 1, &idx) == LEADER_MATCH_PARTIAL,
              "D -> PARTIAL");
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, du, 2, &idx) == LEADER_MATCH_UNIQUE,
              "D,U -> UNIQUE");
        CHECK(idx == 6, "D,U -> idx 6");
    }

    /* A buffer longer than any sequence matches nothing. */
    {
        uint16_t buf[] = {'R', 'X', 'Y'};
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, buf, 3, &idx) == LEADER_MATCH_NONE,
              "over-long buffer -> NONE");
    }

    /* The collision the lint forbids: [M] is an exact match AND a prefix of
     * [M,L]. The matcher must NOT fire it — it reports AMBIGUOUS. */
    {
        uint16_t buf[] = {'M'};
        CHECK(classify(WITH_COLLISION, WITH_COLLISION_N, buf, 1, &idx) == LEADER_MATCH_AMBIGUOUS,
              "M in a non-prefix-free set -> AMBIGUOUS (never fires)");
    }

    /* An empty buffer is a prefix of everything -> PARTIAL, never fires. */
    {
        CHECK(classify(PREFIX_FREE, PREFIX_FREE_N, NULL, 0, &idx) == LEADER_MATCH_PARTIAL,
              "empty buffer -> PARTIAL");
    }

    if (failures) {
        printf("\n%d leader_fsm test(s) FAILED.\n", failures);
        return 1;
    }
    printf("\nAll leader_fsm tests passed.\n");
    return 0;
}
