/* Off-target unit test for oneshot_fsm.h — the pure state machine behind the
 * eager one-shot modifiers. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Imodules/shofel/oneshot \
 *       -o /tmp/test_oneshot_fsm tools/test_oneshot_fsm.c && /tmp/test_oneshot_fsm
 * or:  make test-oneshot
 *
 * Covers:
 *   - "88 bug": a fast roll `os_sft c a` must yield `Ca`, not `CA` — the one-shot
 *     shift must drop off the SECOND key even when it goes down before the first
 *     comes up.
 *   - Second press releases: re-pressing an armed one-shot drops the modifier
 *     (a bare tap to the host — e.g. double-tapping Gui opens the launcher)
 *     instead of re-arming it, and state-derived Russian-layer suspension
 *     clears with no leak (the old +1/-1 depth counter leaked here).
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

    /* --- Retap releases on the second RELEASE, not on its press. The mod is
     * held eagerly from the first tap; re-pressing the trigger parks in
     * os_down_unqueued (still physically held, no longer armed for a next key)
     * and it is the *release* that drops the mod — a bare modifier tap to the
     * host, timed to the key-up rather than the key-down (double-tapping the
     * Gui combo opens the launcher when your fingers lift). State-derived
     * Russian suspension must clear afterwards, no leak. --- */
    sim_reset(&m);
    sim_ev(&m, os_trigger_down);
    sim_ev(&m, os_trigger_up);   int rt_armed = m.mod_held;    /* tap 1: armed, held  */
    sim_ev(&m, os_trigger_down); int rt_press = m.mod_held;    /* tap 2 down: STILL held */
    CHECK(rt_armed == 1, "retap: first tap arms (mod held)");
    CHECK(rt_press == 1, "retap: the second PRESS keeps the mod held");
    CHECK(m.state == os_down_unqueued, "retap: second press parks in os_down_unqueued");
    sim_ev(&m, os_trigger_up);   int rt_rel = m.mod_held;      /* tap 2 up: released  */
    CHECK(rt_rel == 0, "retap: the second RELEASE drops the mod (bare tap)");
    CHECK(m.state == os_up_unqueued, "retap: settles released");
    CHECK(oneshot_mod_held(m.state) == 0, "retap: RU suspension clears (no leak)");
    sim_ev(&m, os_other_down);   int rt_later = m.mod_held;    /* later key unmodified */
    sim_ev(&m, os_other_up);
    CHECK(rt_later == 0, "retap: a later key is unmodified");

    /* --- Chording off a retap: hold the re-pressed trigger and hit a key. The
     * mod must apply to that key, and the eventual trigger release must NOT
     * also emit a bare tap — the one-shot was spent on the chord. --- */
    sim_reset(&m);
    sim_ev(&m, os_trigger_down);
    sim_ev(&m, os_trigger_up);                                 /* tap 1: armed        */
    sim_ev(&m, os_trigger_down);                               /* tap 2: held         */
    sim_ev(&m, os_other_down);   int rc_key = m.mod_held;      /* k down: modified    */
    sim_ev(&m, os_other_up);                                   /* k up                */
    CHECK(rc_key == 1, "retap chord: the key is modified");
    CHECK(m.state == os_down_used, "retap chord: a used key moves to os_down_used");
    sim_ev(&m, os_trigger_up);
    CHECK(m.state == os_up_unqueued && m.mod_held == 0, "retap chord: settles released");

    if (failures) {
        printf("\n%d FAILURE(S)\n", failures);
        return 1;
    }
    printf("\nAll oneshot_fsm tests passed.\n");
    return 0;
}
