/* smart_quote.h — what the quote combo (g+v) produces, by layer.
 *
 * Pure logic, no QMK deps, so tools/test_smart_quote.c can exercise it on the
 * host. keymap.c owns the emission (the guillemet pair and the cursor move);
 * this owns only the layer-scoped choice.
 *
 * The ASCII double quote is a code character and stays on the Latin layers. On a
 * Russian layer the same combo is a smart guillemet: it emits « » and leaves the
 * cursor between them, so Russian prose quotes with one press and types inside.
 */
#pragma once

#include <stdbool.h>

typedef enum {
    QUOTE_ASCII,           /* emit the ASCII double quote  "         */
    QUOTE_GUILLEMET_PAIR,  /* emit « », cursor left between them     */
} smart_quote_kind_t;

static inline smart_quote_kind_t smart_quote_kind(bool russian_active) {
    return russian_active ? QUOTE_GUILLEMET_PAIR : QUOTE_ASCII;
}
