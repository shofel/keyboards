#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(ONESHOT_TAP_TOGGLE) && ONESHOT_TAP_TOGGLE > 1
#    error "Layer retap parity requires non-locking QMK one-shots"
#endif

/* QMK's OSL release cancels whenever tap.count > 1. Instead, base the result
 * on whether this layer was waiting for a key at the start of this gesture.
 * Run before/after the stock action: QMK still owns holds, key consumption,
 * source-layer caching and tap classification. Never change record->tap. */
typedef struct {
    uint32_t cancel;
    uint32_t finish;
} layer_retap_t;

typedef enum {
    LAYER_RETAP_UNCHANGED,
    LAYER_RETAP_ARM,
    LAYER_RETAP_CANCEL,
} layer_retap_result_t;

static inline void layer_retap_before(layer_retap_t *state, uint8_t layer,
                                      bool pressed, bool tapped, bool same_pending) {
    const uint32_t bit = UINT32_C(1) << layer;
    state->finish &= ~bit;
    if (pressed) {
        if (same_pending) state->cancel |= bit;
        else state->cancel &= ~bit;
    } else if (tapped && same_pending) {
        /* A held-layer key already used by another key must not re-arm. */
        state->finish |= bit;
    }
}

static inline layer_retap_result_t layer_retap_after(layer_retap_t *state, uint8_t layer) {
    const uint32_t bit = UINT32_C(1) << layer;
    if (!(state->finish & bit)) return LAYER_RETAP_UNCHANGED;
    state->finish &= ~bit;
    return state->cancel & bit ? LAYER_RETAP_CANCEL : LAYER_RETAP_ARM;
}
