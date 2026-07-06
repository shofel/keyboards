/* Pure geometry for bisect mouse mode: a normalized bounding box in digitizer
 * coordinates (0..1, origin top-left). Header-only and QMK-free so it can be
 * unit-tested off-target (see tools/test_bisect_geom.c).
 *
 * Each halving discards the far half along one axis; the caller then moves the
 * absolute pointer to (bisect_cx, bisect_cy) — the center of the surviving box.
 */
#pragma once

typedef struct {
    float x0, x1, y0, y1;
} bisect_box_t;

static inline void bisect_reset(bisect_box_t *b) {
    b->x0 = 0.0f;
    b->x1 = 1.0f;
    b->y0 = 0.0f;
    b->y1 = 1.0f;
}

static inline void bisect_left(bisect_box_t *b)  { b->x1 = (b->x0 + b->x1) * 0.5f; }
static inline void bisect_right(bisect_box_t *b) { b->x0 = (b->x0 + b->x1) * 0.5f; }
/* Origin is top-left: "up" keeps the smaller-y (top) half, "down" the larger-y. */
static inline void bisect_up(bisect_box_t *b)    { b->y1 = (b->y0 + b->y1) * 0.5f; }
static inline void bisect_down(bisect_box_t *b)  { b->y0 = (b->y0 + b->y1) * 0.5f; }

static inline float bisect_cx(const bisect_box_t *b) { return (b->x0 + b->x1) * 0.5f; }
static inline float bisect_cy(const bisect_box_t *b) { return (b->y0 + b->y1) * 0.5f; }
