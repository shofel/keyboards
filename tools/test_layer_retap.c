/* Off-target model of QMK's non-locking OP_ONESHOT branch.
 * Keep stock() in sync with quantum/action.c and action_util.c; this is not
 * a substitute for the firmware build or physical tap/hold QA. */
#include <assert.h>
#include <stdio.h>
#include "layer_retap.h"

enum { PRESSED = 1, OTHER = 2, SYM = 3, NAV = 4, FKEY = 5 };
static layer_retap_t retap;
static unsigned state, current;
static uint32_t layers;

static void arm(unsigned layer, unsigned flags) {
    current = layer;
    state = flags;
    layers |= UINT32_C(1) << layer;
}
static void clear(unsigned flags) {
    unsigned old = state;
    state &= ~flags;
    if (!state && old) {
        layers &= ~(UINT32_C(1) << current);
        current = 0;
    }
}
static void stock(unsigned layer, bool pressed, unsigned tap_count) {
    if (pressed) {
        clear(OTHER); /* process_action consumes the old OSL before setting it. */
        arm(layer, PRESSED | OTHER);
    } else {
        clear(PRESSED);
        if (tap_count > 1) clear(OTHER);
    }
}
static void event(unsigned layer, bool pressed, unsigned tap_count) {
    layer_retap_before(&retap, layer, pressed, tap_count != 0,
                       current == layer && (state & OTHER));
    stock(layer, pressed, tap_count);
    switch (layer_retap_after(&retap, layer)) {
        case LAYER_RETAP_ARM: arm(layer, OTHER); break;
        case LAYER_RETAP_CANCEL: clear(OTHER); break;
        default: break;
    }
}
static void reset(void) {
    retap = (layer_retap_t){0};
    state = current = layers = 0;
}
static void tap(unsigned layer, unsigned count) {
    event(layer, true, count);
    event(layer, false, count);
}
static char type_r(void) {
    char out = layers & (UINT32_C(1) << SYM) ? '+' : 'r';
    clear(OTHER);
    return out;
}
int main(void) {
    /* Pin the two reported stock failures before testing our adapter. */
    reset(); stock(SYM, true, 1); stock(SYM, false, 1);
    stock(SYM, true, 1); stock(SYM, false, 1);
    assert(type_r() == '+');
    reset();
    for (unsigned i = 1; i <= 3; ++i) { stock(SYM, true, i); stock(SYM, false, i); }
    assert(type_r() == 'r');

    /* QMK gives count=1 after a gap, increasing counts for rapid retaps.
     * All mixtures must alternate, including a fresh burst after cancellation. */
    for (unsigned pattern = 0; pattern < 256; ++pattern) {
        reset();
        unsigned count = 0;
        for (unsigned i = 0; i < 8; ++i) {
            count = (pattern & (1u << i)) ? 1 : count + 1;
            tap(SYM, count);
            assert(!!(layers & (UINT32_C(1) << SYM)) == !(i & 1));
            assert(!!state == !(i & 1));
        }
    }
    for (unsigned taps = 1; taps <= 4; ++taps) {
        for (unsigned rapid = 0; rapid <= 1; ++rapid) {
            reset();
            for (unsigned i = 1; i <= taps; ++i) tap(SYM, rapid ? i : 1);
            assert(type_r() == ((taps & 1) ? '+' : 'r'));
            assert(type_r() == 'r'); /* consumed exactly once */
        }
    }
    /* Consuming a one-shot starts the next tap from off, even if QMK counts up. */
    reset(); tap(SYM, 1); assert(type_r() == '+');
    tap(SYM, 2); assert(type_r() == '+');
    /* Hold + key retains stock momentary behavior, with or without prior arm. */
    for (unsigned pending = 0; pending <= 1; ++pending) {
        reset(); if (pending) tap(SYM, 1);
        event(SYM, true, 0); assert(type_r() == '+');
        assert(layers & (UINT32_C(1) << SYM));
        event(SYM, false, 0); assert(type_r() == 'r');
    }
    /* A quick overlap can consume the layer even with tap.count != 0. */
    reset(); event(SYM, true, 1); assert(type_r() == '+');
    event(SYM, false, 1); assert(type_r() == 'r');
    /* Switching layer is an arm, not a cancellation of the new layer. */
    reset(); tap(SYM, 1); tap(NAV, 1);
    assert(layers == (UINT32_C(1) << NAV));
    tap(NAV, 1); assert(layers == 0);
    tap(FKEY, 3); assert(layers == (UINT32_C(1) << FKEY));
    tap(FKEY, 4); assert(layers == 0);
    /* Highest encodable OSL layer and repeated post hook are safe. */
    reset(); tap(31, 1); assert(layers == (UINT32_C(1) << 31));
    assert(layer_retap_after(&retap, 31) == LAYER_RETAP_UNCHANGED);
    tap(31, 1); assert(layers == 0);
    puts("PASS: stock failures reproduced; retap parity, consumption, holds, overlap and layer switching.");
}
