/* toggle_select.h — cancel-on-reselect for the leader-activated toggle layers.
 *
 * Pure logic, no QMK deps, so tools/test_toggle_select.c can exercise it on the
 * host. keymap.c owns the layer/backend state and the register/unregister
 * effects; this owns only the decision.
 *
 * Two kinds of toggle layer, and only one of them cancels on re-select:
 *   - Overlays (num/nav, F-keys, mouse) are MODS — reversible by a second tap,
 *     mirroring a one-shot's second-tap cancel: re-selecting the active overlay
 *     turns it off.
 *   - The Russian layers are language SWITCHES — stable. Re-selecting the active
 *     Russian layer never cancels; you leave Russian with leader,e / leader,space.
 *     (Re-selecting it with a different backend switches the backend; that switch
 *     lives in keymap.c's toggle_select, not here.)
 * So the decision is simply: cancel iff the same non-Russian layer is re-selected.
 */
#pragma once

#include <stdbool.h>

/* True -> the selection cancels the active layer (turn it off). False -> it
 * activates the target (a fresh overlay, or a Russian re-assert / backend switch). */
static inline bool toggle_reselect_cancels(bool same_layer, bool layer_is_ru) {
    return same_layer && !layer_is_ru;
}
