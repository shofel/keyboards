/* Pure finite-state machine for the eager one-shot modifiers.
 *
 * QMK-free and header-only so the transition logic can be unit-tested
 * off-target (see tools/test_oneshot_fsm.c), mirroring bisect_geom.h. oneshot.c
 * drives this table and performs the register/unregister effects; the keymap
 * derives Russian-layer suspension from the state via oneshot_mod_held().
 *
 * Eager = the modifier is pressed (register_code) on the trigger keydown, not
 * deferred to the next key. That lets a one-shot be chorded/held. The subtlety
 * is releasing it after exactly ONE key when tapped, even under fast rolls.
 *
 * Design constraint — NO TIMEOUTS. The machine advances only on key events
 * (see oneshot_event_t); there is no timer, matrix-scan, or deferred-exec
 * transition. So a tapped one-shot holds its modifier until released by the
 * next key event, by a second press of the same trigger (see os_trigger_down),
 * or by an explicit oneshot_cancel() — it never auto-expires. This is
 * deliberate: it keeps the FSM pure and unit-testable off-target. QMK's
 * ONESHOT_TIMEOUT does NOT apply here — this is a custom register_code16
 * module, not QMK's built-in OSM. The next keypress always releases the mod, so
 * a stray tap is invisible in normal typing.
 */
#pragma once

typedef enum {
    os_up_unqueued,  /* idle: modifier released                                   */
    os_up_queued,    /* tapped, armed for the next key; modifier held (eager)     */
    os_down_unused,  /* trigger physically held, not yet used by another key      */
    os_down_used,    /* trigger physically held, already used by another key      */
    os_up_used,      /* tapped and already applied to one key; modifier still held,
                        released on the next key event (down of the next key OR up
                        of the applied key, whichever comes first)                */
} oneshot_state_t;

typedef enum {
    os_trigger_down, /* the trigger key was pressed                               */
    os_trigger_up,   /* the trigger key was released                             */
    os_other_down,   /* a non-trigger, non-ignored key was pressed               */
    os_other_up,     /* a non-trigger, non-ignored key was released              */
    os_ignore,       /* an ignored key (other trigger / OSL): no effect          */
} oneshot_event_t;

typedef enum {
    os_fx_none,
    os_fx_register,   /* press the modifier (register_code16(triggee))            */
    os_fx_unregister, /* release the modifier (unregister_code16(triggee))        */
} oneshot_effect_t;

/* Advance the machine one event. Pure: no side effects, no QMK. */
static inline oneshot_state_t oneshot_next_state(oneshot_state_t s, oneshot_event_t e) {
    switch (e) {
        case os_trigger_down:
            /* Second press of an already-queued one-shot releases it: the mod
             * has been held (eager) since the first tap, so dropping it now —
             * with no key in between — emits a bare modifier tap to the host
             * (e.g. double-tapping the Gui combo opens the launcher) and
             * disarms. A first press (from os_up_unqueued) still arms. Purely
             * state-driven, no timer — consistent with the no-timeout design. */
            if (s == os_up_queued) return os_up_unqueued;
            return os_down_unused;

        case os_trigger_up:
            if (s == os_down_unused) return os_up_queued;
            if (s == os_down_used)   return os_up_unqueued;
            return s;

        case os_other_down:
            /* Tapped path only. A held trigger (os_down_*) keeps shifting every
             * key, so it ignores other keydowns. */
            if (s == os_up_queued) return os_up_used;     /* 1st key: keep the mod */
            if (s == os_up_used)   return os_up_unqueued; /* rolled 2nd key: drop the
                                                             mod BEFORE it is emitted */
            return s;

        case os_other_up:
            if (s == os_down_unused) return os_down_used;
            if (s == os_up_queued)   return os_up_unqueued; /* slow tap: key released
                                                               before any roll        */
            if (s == os_up_used)     return os_up_unqueued; /* applied key released    */
            return s;

        case os_ignore:
            return s;
    }
    return s;
}

/* The effect is a function of the state just entered, matching the original
 * "register on os_down_unused, unregister on os_up_unqueued" rule. os_up_used
 * keeps the existing registration (no effect), which is what makes the first
 * rolled key keep the modifier while the second drops it. */
static inline oneshot_effect_t oneshot_effect(oneshot_state_t entered) {
    if (entered == os_down_unused) return os_fx_register;
    if (entered == os_up_unqueued) return os_fx_unregister;
    return os_fx_none;
}

/* True while this one-shot physically holds its modifier. Russian-layer
 * suspension is OR-reduced over the mod one-shots from this predicate, so it is
 * a pure function of state and cannot leak the way a +1/-1 depth counter did
 * when a mod one-shot was re-tapped while already queued. */
static inline int oneshot_mod_held(oneshot_state_t s) {
    return s != os_up_unqueued;
}
