/* Pure counters for the key-level typing instrument.
 *
 * QMK-free and header-only so the counting logic can be unit-tested off-target
 * (see tools/test_keylog_stats.c). keylog.c drives this from
 * process_record_user and prints it on demand.
 *
 * WHY COUNTERS AND NOT A LOG
 * --------------------------
 * The character corpus (tools/typing_corpus.py) already measures what survived
 * into a prompt. It is structurally blind to what this instrument is for:
 * backspace rate, modifiers, layer switches, combo usage, thumb load, and which
 * keys get typed then immediately corrected — several of which are plausibly
 * among the most-pressed keys on the board and none of which reach the text.
 *
 * None of those questions need the ORDER of keystrokes. Order is precisely what
 * would make this a keylogger in the harmful sense: a board that can emit the
 * sequence can emit a password. So the state is counters plus exactly ONE slot
 * of history — last_key, the minimum needed to attribute a backspace to the key
 * it deleted. Nothing here can reconstruct a two-key sequence, and a unit test
 * pins that by asserting two different typing orders leave byte-identical
 * state.
 *
 * Counters saturate rather than wrap: wrapping would silently turn the
 * most-pressed key into the least-pressed one, inverting the measurement this
 * exists to make.
 */
#pragma once

#include <stdint.h>
#include <string.h>

/* Cantor is 42 keys; the headroom covers a bigger board without a format change. */
#define KEYLOG_MAX_KEYS   64
#define KEYLOG_MAX_LAYERS 8

#define KEYLOG_COUNT_MAX 0xFFFFu
#define KEYLOG_NO_KEY    0xFFu

typedef struct {
    uint16_t key[KEYLOG_MAX_KEYS];       /* presses per physical key            */
    uint16_t corrected[KEYLOG_MAX_KEYS]; /* presses undone by an immediate bksp */
    uint16_t layer[KEYLOG_MAX_LAYERS];   /* presses made while this layer was on */
    uint32_t total;                      /* every press, including backspace    */
    uint16_t backspace;                  /* backspace presses                   */
    uint16_t combo_fired;                /* combos that actually fired          */
    uint8_t  last_key;                   /* THE only order-carrying field       */
} keylog_stats_t;

static inline void keylog_bump(uint16_t *slot) {
    if (*slot < KEYLOG_COUNT_MAX) (*slot)++;
}

static inline void keylog_reset(keylog_stats_t *s) {
    /* memset the whole struct, padding included, so two runs that saw the same
     * keys really are byte-identical (the privacy test compares raw bytes). */
    memset(s, 0, sizeof(*s));
    s->last_key = KEYLOG_NO_KEY;
}

static inline void keylog_record_key(keylog_stats_t *s, uint8_t index) {
    if (index >= KEYLOG_MAX_KEYS) return; /* never write out of bounds */
    keylog_bump(&s->key[index]);
    s->total++;
    s->last_key = index;
}

static inline void keylog_record_backspace(keylog_stats_t *s) {
    keylog_bump(&s->backspace);
    s->total++;
    if (s->last_key != KEYLOG_NO_KEY) {
        keylog_bump(&s->corrected[s->last_key]);
        /* Clear the slot: a run of backspaces deletes a run of keys, but only
         * the first has a known victim. Re-crediting would invent data. */
        s->last_key = KEYLOG_NO_KEY;
    }
}

static inline void keylog_record_layer(keylog_stats_t *s, uint8_t layer) {
    if (layer >= KEYLOG_MAX_LAYERS) return;
    keylog_bump(&s->layer[layer]);
}

/* A combo firing is deliberately NOT a keypress: no finger landed on a new key,
 * and QMK reports the synthetic record at position (0,0) -- a real key -- so
 * counting it would inflate whatever sits at index 0.
 *
 * This is a total rather than a per-combo breakdown because QMK offers no
 * fire-time hook carrying the combo index: `combo_should_trigger` is called once
 * per candidate key of every candidate combo on every press, so counting there
 * would overcount wildly. `IS_COMBOEVENT` is exact but anonymous. An exact total
 * beats a plausible-looking wrong breakdown; per-combo attribution needs an
 * upstream hook and is left undone rather than faked. */
static inline void keylog_record_combo(keylog_stats_t *s) {
    keylog_bump(&s->combo_fired);
}

/* Parts-per-million of presses of `index` that were immediately corrected.
 * ppm rather than a float: no FPU cost on the MCU, and no rounding argument. */
static inline uint32_t keylog_correction_ppm(const keylog_stats_t *s, uint8_t index) {
    if (index >= KEYLOG_MAX_KEYS || s->key[index] == 0) return 0;
    return (uint32_t)s->corrected[index] * 1000000u / s->key[index];
}
