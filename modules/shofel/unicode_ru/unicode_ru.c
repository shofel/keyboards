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

/* Default ON: compose is the standard Russian/symbol backend. leader,v flips
 * it off (vim mode); leader,r/c/e set it back on. */
bool ru_compose_mode = true;

/* Emit `Compose + <code>`, with shift stripped (the glyph's case is encoded in
 * the code, q.. vs Q.., not in a live shift). Exported so the keymap can emit
 * standalone glyphs like « » that aren't unicode_map keys. */
void ru_compose_emit_code(const char *code) {
    if (code == NULL || code[0] == '\0') {
        return;
    }
    uint8_t held = get_mods();
    clear_oneshot_mods();
    del_mods(MOD_MASK_SHIFT);
    tap_code(KC_SCRL);  // Compose (Scroll Lock -> Multi_key via compose:sclk)
    send_string(code);
    set_mods(held);
}

/* Emit one entry (by `enum unicode_names` index). */
static void ru_compose_emit_index(uint8_t idx) {
    /* '.' / ',' are plain ASCII — send literally (no Compose), shift stripped. */
    if (idx == U_DOT || idx == U_COMMA) {
        uint8_t held = get_mods();
        clear_oneshot_mods();
        del_mods(MOD_MASK_SHIFT);
        send_string(idx == U_DOT ? "." : ",");
        set_mods(held);
        return;
    }
    if (idx < (sizeof(ru_compose_code) / sizeof(ru_compose_code[0])) &&
        ru_compose_code[idx] != NULL) {
        ru_compose_emit_code(ru_compose_code[idx]);
    }
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
