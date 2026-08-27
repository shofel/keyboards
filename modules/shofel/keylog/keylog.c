/* Key-level typing instrument.
 *
 * The character corpus (tools/typing_corpus.py) measures what survived into a
 * prompt. This measures what the fingers did: which physical keys, on which
 * layer, how often a key was immediately taken back with backspace, and how
 * often a combo fired. Those are the quantities the number-entry study lacked.
 *
 * Counts only, never sequences — see keylog_stats.h for why, and for the unit
 * test that pins the property.
 */

#include QMK_KEYBOARD_H
#include "print.h"
#include "keylog.h"

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

static keylog_stats_t stats = {.last_key = KEYLOG_NO_KEY};

const keylog_stats_t *keylog_get(void) {
    return &stats;
}

void keylog_clear(void) {
    keylog_reset(&stats);
    uprintf("keylog cleared\n");
}

/* Community Module hook: process_record. Never consumes the record. */
bool process_record_keylog(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    /* A combo emits a keycode from key position (0,0) with type COMBO_EVENT.
     * That position is a REAL key on this board, so counting the synthetic
     * record as a press would silently inflate key index 0. Physical presses
     * only. */
    if (IS_COMBOEVENT(record->event)) {
        keylog_record_combo(&stats);
        return true;
    }
    if (!IS_KEYEVENT(record->event)) return true; /* encoders, ticks, dip switches */

    /* Layer usage is counted per press rather than per activation: what a layout
     * decision needs is how many keystrokes happen on a layer, not how many
     * times it was entered. */
    keylog_record_layer(&stats, (uint8_t)get_highest_layer(layer_state));

    if (keycode == KC_BSPC) {
        keylog_record_backspace(&stats);
    } else {
        const uint8_t index = (uint8_t)(record->event.key.row * MATRIX_COLS + record->event.key.col);
        keylog_record_key(&stats, index);
    }
    return true;
}

void keylog_dump(void) {
    uprintf("keylog total=%lu backspace=%u combo=%u\n",
            (unsigned long)stats.total, stats.backspace, stats.combo_fired);

    for (uint8_t l = 0; l < KEYLOG_MAX_LAYERS; l++) {
        if (stats.layer[l]) uprintf("keylog layer %u %u\n", l, stats.layer[l]);
    }
    /* row/col as well as the flat index: the flat index is what the counters
     * use, but a layout decision is made in row/col terms. */
    for (uint8_t i = 0; i < KEYLOG_MAX_KEYS; i++) {
        if (!stats.key[i]) continue;
        uprintf("keylog key %u r%u c%u n=%u corrected=%u ppm=%lu\n",
                i, i / MATRIX_COLS, i % MATRIX_COLS, stats.key[i], stats.corrected[i],
                (unsigned long)keylog_correction_ppm(&stats, i));
    }
    uprintf("keylog end\n");
}
