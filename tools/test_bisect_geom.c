/* Off-target unit test for bisect_geom.h — the pure box math behind bisect
 * mouse mode. No QMK deps, so it runs on the host.
 *
 * Build & run from the repo root:
 *   gcc -Wall -Wextra -Ilayouts/split_3x6_3/shofel \
 *       -o /tmp/test_bisect_geom tools/test_bisect_geom.c -lm && /tmp/test_bisect_geom
 * or:  make test-bisect
 */
#include <math.h>
#include <stdio.h>
#include "bisect_geom.h"

static int failures = 0;

static void check(const char *name, float got, float want) {
    if (fabsf(got - want) > 1e-6f) {
        printf("FAIL %s: got %.6f want %.6f\n", name, got, want);
        failures++;
    } else {
        printf("ok   %s = %.6f\n", name, got);
    }
}

int main(void) {
    bisect_box_t b;

    /* Full screen centers the pointer. */
    bisect_reset(&b);
    check("reset cx", bisect_cx(&b), 0.5f);
    check("reset cy", bisect_cy(&b), 0.5f);

    /* One halving per axis, re-centered into the surviving half. */
    bisect_reset(&b); bisect_right(&b);
    check("right cx", bisect_cx(&b), 0.75f);
    check("right cy unchanged", bisect_cy(&b), 0.5f);

    bisect_reset(&b); bisect_left(&b);
    check("left cx", bisect_cx(&b), 0.25f);

    /* Origin is top-left, so "up" keeps the SMALLER-y (top) half. */
    bisect_reset(&b); bisect_up(&b);
    check("up cy", bisect_cy(&b), 0.25f);
    check("up cx unchanged", bisect_cx(&b), 0.5f);

    bisect_reset(&b); bisect_down(&b);
    check("down cy", bisect_cy(&b), 0.75f);

    /* Two axes drill into a quadrant. */
    bisect_reset(&b); bisect_right(&b); bisect_up(&b);
    check("right+up cx", bisect_cx(&b), 0.75f);
    check("right+up cy", bisect_cy(&b), 0.25f);

    /* Repeated halving on one axis keeps narrowing. */
    bisect_reset(&b); bisect_right(&b); bisect_right(&b);
    check("right x2 cx", bisect_cx(&b), 0.875f);

    /* Reset undoes all narrowing. */
    bisect_reset(&b); bisect_left(&b); bisect_down(&b); bisect_reset(&b);
    check("reset restores cx", bisect_cx(&b), 0.5f);
    check("reset restores cy", bisect_cy(&b), 0.5f);

    if (failures) {
        printf("\n%d FAILURE(S)\n", failures);
        return 1;
    }
    printf("\nAll bisect_geom tests passed.\n");
    return 0;
}
