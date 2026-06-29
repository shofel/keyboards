/**
 * Russian Unicode Support Community Module
 *
 * Provides Russian Cyrillic character support via unicode_map (see
 * introspection.c), plus an alternative "compose mode" emission backend.
 *
 * Compose mode: instead of the modal ibus hex handshake (Ctrl+Shift+U <hex>),
 * which corrupts under rolling because its commit boundary depends on modifier
 * release/terminator timing, each glyph is emitted as ONE self-delimiting X
 * Compose sequence: Compose + a private 2-char code. Compose is Scroll Lock,
 * bound to Multi_key on the host via the xkb `compose:sclk` option. Sequences
 * commit on prefix-tree match (no held modifier across the boundary, no
 * terminator), so rolling Russian behaves like rolling Latin.
 */

#include QMK_KEYBOARD_H
#include "introspection.h"
#include "unicodemap.h"
#include "ru_compose_table.h"  // generated: ru_compose_code[] (tools/gen_unicode_compose.py)

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

bool ru_compose_mode = false;

/* Emit one entry (by `enum unicode_names` index) as a Compose sequence. */
static void ru_compose_emit_index(uint8_t idx) {
    /* Strip shift (held + one-shot) so the code's keysyms are unambiguous: the
     * glyph's case is already encoded in the code (q.. vs Q..), not in a live
     * shift. Restore the user's mods afterwards. */
    uint8_t held = get_mods();
    clear_oneshot_mods();
    del_mods(MOD_MASK_SHIFT);

    if (idx == U_DOT) {
        send_string(".");
    } else if (idx == U_COMMA) {
        send_string(",");
    } else if (idx < (sizeof(ru_compose_code) / sizeof(ru_compose_code[0])) &&
               ru_compose_code[idx] != NULL) {
        tap_code(KC_SCRL);  // Compose (Scroll Lock -> Multi_key via compose:sclk)
        send_string(ru_compose_code[idx]);
    }

    set_mods(held);
}

/* When compose mode is active, intercept unicode_map keycodes (the RU_* / U_*
 * keys) and emit via Compose instead of the default hex path. Returns true if
 * handled, in which case the caller must stop further processing. */
bool ru_compose_process(uint16_t keycode, keyrecord_t *record) {
    if (!ru_compose_mode || !record->event.pressed) {
        return false;
    }
    if (keycode >= QK_UNICODEMAP && keycode <= QK_UNICODEMAP_PAIR_MAX) {
        ru_compose_emit_index(unicodemap_index(keycode));
        return true;
    }
    return false;
}
