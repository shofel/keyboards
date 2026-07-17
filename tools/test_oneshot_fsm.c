/* Off-target unit test for oneshot_fsm.h — the pure state machine behind the
 * eager one-shot modifiers. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/oneshot \
 *       -o /tmp/test_oneshot_fsm tools/test_oneshot_fsm.c && /tmp/test_oneshot_fsm
 * or:  make test-oneshot
 *
 * Covers two shipped bugs:
 *   - "88 bug": a fast roll `os_sft c a` must yield `Ca`, not `CA` — the one-shot
 *     shift must drop off the SECOND key even when it goes down before the first
 *     comes up.
 *   - RU-activation leak: Russian-layer suspension is derived from state, so
 *     re-tapping a mod one-shot while it is queued must not leave suspension
 *     stuck on (the old +1/-1 depth counter leaked here).
 */
#include <stdio.h>
#include "oneshot_fsm.h"

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

/* A one-shot plus the physical modifier registration it drives. */
typedef struct {
    oneshot_state_t state;
    int             mod_held; /* 1 while register_code16(triggee) is outstanding */
} sim_t;

static void sim_reset(sim_t *m) {
    m->state    = os_up_unqueued;
    m->mod_held = 0;
}

/* Feed one event: advance the state, run the register/unregister effect, and
 * assert the core invariant — the state-derived "mod held" always matches the
 * real registration. That invariant is what lets the keymap derive Russian
 * suspension from state instead of a leak-prone counter. */
static void sim_ev(sim_t *m, oneshot_event_t e) {
    oneshot_state_t entered = oneshot_next_state(m->state, e);
    switch (oneshot_effect(entered)) {
        case os_fx_register:
            m->mod_held = 1;
            break;
        case os_fx_unregister:
            m->mod_held = 0;
            break;
        case os_fx_none:
            break;
    }
    m->state = entered;
    if (oneshot_mod_held(m->state) != m->mod_held) {
        printf("FAIL invariant: state=%d derived_held=%d actual=%d\n",
               m->state, oneshot_mod_held(m->state), m->mod_held);
        failures++;
    }
}

int main(void) {
    sim_t m;

    /* --- Bug "88": tapped one-shot shift, fast roll `c a` (a down before c up).
     * The mod must apply to c only. --- */
    sim_reset(&m);
    sim_ev(&m, os_trigger_down);
    sim_ev(&m, os_trigger_up);
    sim_ev(&m, os_other_down);  int roll_k1 = m.mod_held; /* c down */
    sim_ev(&m, os_other_down);  int roll_k2 = m.mod_held; /* a down (rolled) */
    sim_ev(&m, os_other_up);                              /* c up */
    sim_ev(&m, os_other_up);                              /* a up */
    CHECK(roll_k1 == 1, "fast roll: 1st key keeps the mod  -> C");
    CHECK(roll_k2 == 0, "fast roll: 2nd key drops the mod  -> a (not A)");
    CHECK(m.state == os_up_unqueued && m.mod_held == 0, "fast roll: settles released");

    /* --- Tapped one-shot, slow (no overlap): still exactly one shifted key. --- */
    sim_reset(&m);
    sim_ev(&m, os_trigger_down);
    sim_ev(&m, os_trigger_up);
    sim_ev(&m, os_other_down);  int slow_k1 = m.mod_held; /* c down */
    sim_ev(&m, os_other_up);                              /* c up */
    sim_ev(&m, os_other_down);  int slow_k2 = m.mod_held; /* a down */
    sim_ev(&m, os_other_up);                              /* a up */
    CHECK(slow_k1 == 1, "slow tap: 1st key shifted");
    CHECK(slow_k2 == 0, "slow tap: 2nd key not shifted");

    /* --- Held trigger (physically holding shift): every key is shifted. --- */
    sim_reset(&m);
    sim_ev(&m, os_trigger_down);
    sim_ev(&m, os_other_down);  int held_k1 = m.mod_held; /* c down */
    sim_ev(&m, os_other_up);                              /* c up */
    sim_ev(&m, os_other_down);  int held_k2 = m.mod_held; /* a down */
    sim_ev(&m, os_other_up);                              /* a up */
    sim_ev(&m, os_trigger_up);                            /* release shift */
    CHECK(held_k1 == 1 && held_k2 == 1, "held trigger: both keys shifted");
    CHECK(m.state == os_up_unqueued && m.mod_held == 0, "held trigger: settles released");

    /* --- Bug RU-activation: re-tap a mod one-shot while it is queued, then use
     * it once. The legacy +1/-1 depth counter leaks (>0, Russian stuck
     * suspended); the state-derived predicate stays correct. --- */
    {
        int             depth = 0; /* mirrors old keymap mod_ru_suspend_depth */
        oneshot_state_t prev;
        sim_reset(&m);

#define STEP(ev)                                                        \
        do {                                                            \
            prev = m.state;                                             \
            sim_ev(&m, ev);                                             \
            if (m.state == os_down_unused && prev != os_down_unused) depth++; \
            if (m.state == os_up_unqueued && prev != os_up_unqueued) depth--; \
        } while (0)

        STEP(os_trigger_down); /* arm            */
        STEP(os_trigger_up);   /* queued         */
        STEP(os_trigger_down); /* RE-TAP whilst queued: legacy double-counts */
        STEP(os_trigger_up);   /* queued again   */
        STEP(os_other_down);   /* used by 1 key  */
        STEP(os_other_up);     /* key up         */
#undef STEP

        CHECK(depth == 1, "double-tap: legacy depth counter LEAKS to 1 (bug 1 root cause)");
        CHECK(oneshot_mod_held(m.state) == 0,
              "double-tap: state-derived suspension is CLEAR (bug 1 fixed)");
    }

    if (failures) {
        printf("\n%d FAILURE(S)\n", failures);
        return 1;
    }
    printf("\nAll oneshot_fsm tests passed.\n");
    return 0;
}
