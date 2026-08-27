/* Off-target unit test for keylog_stats.h — the pure counters behind the
 * key-level typing instrument. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/keylog \
 *       -o /tmp/test_keylog_stats tools/test_keylog_stats.c && /tmp/test_keylog_stats
 * or:  make test-keylog
 *
 * Why this is counters and not a log: the instrument exists to answer what the
 * character corpus structurally cannot — backspace rate, layer and combo usage,
 * thumb load, which keys get typed then corrected. None of those need the ORDER
 * of keystrokes, and keeping order would mean the board could emit the user's
 * passwords. So the state holds exactly one slot of history (last_key, needed to
 * attribute a backspace to what it deleted) and nothing else. The final test
 * pins that property: no two-key sequence is recoverable from the state.
 */
#include <stdio.h>
#include <string.h>
#include "keylog_stats.h"

static int failures = 0;

#define CHECK(cond, msg)              \
    do {                              \
        if (cond) {                   \
            printf("ok   %s\n", msg); \
        } else {                      \
            printf("FAIL %s\n", msg); \
            failures++;               \
        }                             \
    } while (0)

static void test_record_key_counts(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_key(&s, 3);
    keylog_record_key(&s, 3);
    keylog_record_key(&s, 5);
    CHECK(s.key[3] == 2, "key pressed twice counts 2");
    CHECK(s.key[5] == 1, "other key counts 1");
    CHECK(s.total == 3, "total counts every press");
}

static void test_out_of_range_key_ignored(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_key(&s, KEYLOG_MAX_KEYS);      /* one past the end */
    keylog_record_key(&s, KEYLOG_MAX_KEYS + 40); /* far past the end */
    CHECK(s.total == 0, "out-of-range key changes nothing (no OOB write)");
}

static void test_backspace_credits_previous_key(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_key(&s, 7);
    keylog_record_backspace(&s);
    CHECK(s.corrected[7] == 1, "backspace credits the key it deleted");
    CHECK(s.backspace == 1, "backspace counted");
}

static void test_second_backspace_credits_nothing(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_key(&s, 7);
    keylog_record_backspace(&s);
    keylog_record_backspace(&s);
    /* A run of backspaces deletes a run of keys, but only the first has a known
     * victim — crediting key 7 twice would invent data. */
    CHECK(s.corrected[7] == 1, "a second backspace does not re-credit the same key");
    CHECK(s.backspace == 2, "both backspaces still counted");
}

static void test_backspace_with_no_history_is_safe(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_backspace(&s);
    CHECK(s.backspace == 1, "leading backspace counted");
    CHECK(s.total == 1, "leading backspace does not corrupt anything");
}

static void test_counter_saturates_instead_of_wrapping(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    s.key[2] = KEYLOG_COUNT_MAX;
    keylog_record_key(&s, 2);
    /* Wrapping would silently turn the most-pressed key into the least-pressed
     * one — the exact opposite of the measurement. */
    CHECK(s.key[2] == KEYLOG_COUNT_MAX, "key counter saturates rather than wrapping to 0");
}

static void test_layer_and_combo_counters(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_layer(&s, 2);
    keylog_record_layer(&s, 2);
    keylog_record_combo(&s);
    CHECK(s.layer[2] == 2, "layer usage counted per press");
    CHECK(s.combo_fired == 1, "combo firings counted");
    keylog_record_layer(&s, KEYLOG_MAX_LAYERS + 3);
    CHECK(s.layer[2] == 2, "out-of-range layer ignored");
}

static void test_combo_firing_is_not_a_physical_press(void) {
    /* A combo emits a keycode but no finger landed on a new key -- QMK even
     * reports the synthetic record at key position (0,0), which is a REAL key.
     * Counting it as a press would silently inflate whatever key sits at index
     * 0. The driver filters on IS_KEYEVENT; the counters must keep the two
     * tallies apart. */
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_combo(&s);
    CHECK(s.key[0] == 0, "a combo firing does not increment key index 0");
    CHECK(s.total == 0, "a combo firing is not counted as a keypress");
    CHECK(s.combo_fired == 1, "the combo is still tallied on its own counter");
}

static void test_correction_ppm(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    for (int i = 0; i < 4; i++) keylog_record_key(&s, 1);
    s.corrected[1] = 1;
    CHECK(keylog_correction_ppm(&s, 1) == 250000, "1 of 4 corrected = 250000 ppm");
    CHECK(keylog_correction_ppm(&s, 40) == 0, "unused key reports 0 ppm, not a divide by zero");
}

static void test_reset_clears(void) {
    keylog_stats_t s;
    keylog_reset(&s);
    keylog_record_key(&s, 4);
    keylog_record_layer(&s, 1);
    keylog_record_combo(&s);
    keylog_reset(&s);
    CHECK(s.total == 0 && s.key[4] == 0 && s.layer[1] == 0 && s.combo_fired == 0,
          "reset clears every counter");
    CHECK(s.last_key == KEYLOG_NO_KEY, "reset clears the one history slot");
}

static void test_state_carries_no_recoverable_sequence(void) {
    /* THE privacy property. Two different typing orders over the same keys must
     * leave identical state once the single attribution slot is cleared. If any
     * ordering information survived, a password would be reconstructable from a
     * dump. */
    keylog_stats_t a, b;
    keylog_reset(&a);
    keylog_reset(&b);

    const uint8_t forward[]  = {8, 21, 3, 14, 3};
    const uint8_t backward[] = {3, 14, 3, 21, 8};
    for (unsigned i = 0; i < sizeof(forward); i++) keylog_record_key(&a, forward[i]);
    for (unsigned i = 0; i < sizeof(backward); i++) keylog_record_key(&b, backward[i]);

    a.last_key = KEYLOG_NO_KEY;
    b.last_key = KEYLOG_NO_KEY;
    CHECK(memcmp(&a, &b, sizeof(keylog_stats_t)) == 0,
          "two different orders leave byte-identical state (no sequence recoverable)");
}

int main(void) {
    test_record_key_counts();
    test_out_of_range_key_ignored();
    test_backspace_credits_previous_key();
    test_second_backspace_credits_nothing();
    test_backspace_with_no_history_is_safe();
    test_counter_saturates_instead_of_wrapping();
    test_layer_and_combo_counters();
    test_combo_firing_is_not_a_physical_press();
    test_correction_ppm();
    test_reset_clears();
    test_state_carries_no_recoverable_sequence();

    if (failures) {
        printf("\n%d keylog_stats test(s) FAILED.\n", failures);
        return 1;
    }
    printf("\nAll keylog_stats tests passed.\n");
    return 0;
}
