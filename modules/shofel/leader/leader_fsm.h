/* leader_fsm.h — the pure fire-on-unique-match matcher for a timeoutless leader.
 *
 * Stock QMK terminates a leader sequence on a timeout. This matcher removes the
 * clock: after each key it classifies the growing buffer against the sequence
 * table and fires the instant the buffer is a *unique complete* match, aborting
 * the instant it matches nothing. That only works if the sequence set is a
 * prefix code (no sequence a prefix of another) — see tools/check_leader_prefix.py,
 * which guards that invariant; the AMBIGUOUS result below is what a violation
 * would produce, and the matcher refuses to fire it.
 *
 * Pure and header-only (like oneshot_fsm.h), so it unit-tests off-target with no
 * QMK deps — see tools/test_leader_fsm.c / `make test-leader-fsm`.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Longest leader sequence, in keys. Two today (e.g. leader,M,L); one spare. */
#ifndef LEADER_SEQ_MAX
#define LEADER_SEQ_MAX 3
#endif

typedef struct {
    uint8_t  len;
    uint16_t keys[LEADER_SEQ_MAX];
} leader_seq_keys_t;

typedef enum {
    LEADER_MATCH_NONE,       /* buffer prefixes no sequence -> abort now         */
    LEADER_MATCH_PARTIAL,    /* buffer is a proper prefix of >=1 seq -> wait     */
    LEADER_MATCH_UNIQUE,     /* buffer == one seq and prefixes no other -> fire  */
    LEADER_MATCH_AMBIGUOUS,  /* exact match that ALSO prefixes another seq: only
                              * a non-prefix-free set makes this; never fire it. */
} leader_match_t;

/* Classify `buf` (buflen keys) against `table` (n sequences). On UNIQUE, and if
 * `idx` is non-NULL, `*idx` receives the matched sequence's index. */
static inline leader_match_t leader_match(const leader_seq_keys_t *table, size_t n,
                                          const uint16_t *buf, uint8_t buflen,
                                          size_t *idx) {
    int  exact_idx  = -1;   /* index of a sequence exactly equal to the buffer */
    bool any_longer = false;/* buffer is a proper prefix of some sequence      */

    for (size_t i = 0; i < n; i++) {
        const leader_seq_keys_t *s = &table[i];
        if (buflen > s->len) {
            continue; /* buffer already outruns this sequence */
        }
        bool is_prefix = true;
        for (uint8_t k = 0; k < buflen; k++) {
            if (buf[k] != s->keys[k]) {
                is_prefix = false;
                break;
            }
        }
        if (!is_prefix) {
            continue;
        }
        if (buflen == s->len) {
            if (exact_idx < 0) {
                exact_idx = (int)i; /* first exact match wins; dups are a lint bug */
            }
        } else {
            any_longer = true;
        }
    }

    if (exact_idx >= 0) {
        if (any_longer) {
            return LEADER_MATCH_AMBIGUOUS;
        }
        if (idx) {
            *idx = (size_t)exact_idx;
        }
        return LEADER_MATCH_UNIQUE;
    }
    return any_longer ? LEADER_MATCH_PARTIAL : LEADER_MATCH_NONE;
}

/* Stateful capture: a growing key buffer + the matcher, driving a timeoutless
 * leader. `begin` arms it; `feed` appends one key and re-classifies. On any
 * non-PARTIAL result the capture deactivates (fired on UNIQUE, dropped on
 * NONE/AMBIGUOUS), so the caller checks `active` to know when it is over. Kept
 * here (not in the keymap) so it unit-tests off-target with the matcher. */
typedef struct {
    uint16_t buf[LEADER_SEQ_MAX];
    uint8_t  len;
    bool     active;
} leader_capture_t;

static inline void leader_capture_begin(leader_capture_t *c) {
    c->len    = 0;
    c->active = true;
}

/* Append `kc` and classify. Returns the match result; on non-PARTIAL, clears
 * `active`. On UNIQUE, `*idx` (if non-NULL) is the matched sequence index. */
static inline leader_match_t leader_capture_feed(leader_capture_t *c, uint16_t kc,
                                                 const leader_seq_keys_t *table,
                                                 size_t n, size_t *idx) {
    if (c->len >= LEADER_SEQ_MAX) {   /* buffer full and still no match -> give up */
        c->active = false;
        return LEADER_MATCH_NONE;
    }
    c->buf[c->len++] = kc;
    leader_match_t r = leader_match(table, n, c->buf, c->len, idx);
    if (r != LEADER_MATCH_PARTIAL) {
        c->active = false;
    }
    return r;
}
