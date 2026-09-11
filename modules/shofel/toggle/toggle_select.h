/* toggle_select.h — cancel-on-reselect for the leader-activated toggle layers.
 *
 * Pure logic, no QMK deps, so tools/test_toggle_select.c can exercise it on the
 * host. keymap.c owns the layer/backend state and the register/unregister
 * effects; this owns only the decision.
 *
 * The rule mirrors a one-shot's second-tap cancel: re-selecting the switcher
 * that is already active turns it off. A switcher's identity is its layer — and,
 * for the Russian layers, also its backend (compose/vim/windows), so that
 * switching vim -> compose while Russian is live still switches rather than
 * cancelling. For the non-Russian layers the backend is irrelevant and ignored.
 */
#pragma once

#include <stdbool.h>

/* True -> the selection cancels the active layer (turn it off). False -> it
 * activates the target (a fresh layer, or a Russian backend switch). */
static inline bool toggle_reselect_cancels(bool same_layer, bool layer_is_ru,
                                           bool same_backend) {
    return same_layer && (!layer_is_ru || same_backend);
}
