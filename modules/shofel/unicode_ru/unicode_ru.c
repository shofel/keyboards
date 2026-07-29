/**
 * Russian Unicode Support Community Module
 *
 * Provides Russian Cyrillic character support via unicode_map (see
 * introspection.c), plus two userspace emission backends — compose mode
 * (default) and vim mode — selected by `ru_backend`.
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

/* Active emission backend. Compose is the default (rolling-safe, host-wide);
 * vim mode (leader,v) emits vim's native `i_CTRL-V U <hex>` for typing Cyrillic
 * inside vim/neovim without any host compose setup. Both are emitted entirely in
 * userspace here, so QMK's unicode input-mode machinery (and the out-of-tree
 * UNICODE_MODE_VIM) is no longer used. */
ru_backend_t ru_backend = RU_BACKEND_COMPOSE;

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

/* Vim mode: emit a codepoint as vim's native `i_CTRL-V U <8 hex>`. The capital
 * `U` form waits for exactly 8 hex digits, then commits itself — no leading key,
 * no terminator (see `:h i_CTRL-V_digit`). Mirrors the old out-of-tree
 * UNICODE_MODE_VIM emission so the firmware no longer needs the QMK fork for it.
 * Held/one-shot mods are cleared (and restored) so a pending one-shot shift
 * can't corrupt the Ctrl-V. */
void ru_vim_emit_codepoint(uint32_t cp) {
    if (cp > 0x10FFFF) {
        return;
    }
    uint8_t held = get_mods();
    clear_oneshot_mods();
    clear_mods();
    clear_weak_mods();
    tap_code16(LCTL(KC_V));  // Ctrl-V
    tap_code16(LSFT(KC_U));  // Shift-U -> i_CTRL-V_U (8 hex digits)
    char buf[9];
    for (int i = 0; i < 8; i++) {
        buf[i] = "0123456789abcdef"[(cp >> ((7 - i) * 4)) & 0xF];
    }
    buf[8] = '\0';
    send_string(buf);
    set_mods(held);
}

/* Emit a standalone glyph (e.g. « ») via the active backend: compose uses its
 * private 2-char code, vim uses the codepoint. */
void ru_emit_glyph(const char *compose_code, uint32_t cp) {
    if (ru_backend == RU_BACKEND_VIM) {
        ru_vim_emit_codepoint(cp);
    } else {
        ru_compose_emit_code(compose_code);
    }
}

/* Intercept unicode_map keycodes (the RU_* / U_* keys) and emit them via the
 * active userspace backend instead of QMK's hex input path. Returns true if
 * handled, in which case the caller must stop further processing. */
bool ru_unicode_process(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return false;
    }
    if (keycode < QK_UNICODEMAP || keycode > QK_UNICODEMAP_PAIR_MAX) {
        return false;
    }
    uint8_t idx = unicodemap_index(keycode);
    if (ru_backend == RU_BACKEND_VIM) {
        ru_vim_emit_codepoint(pgm_read_dword(&unicode_map[idx]));
    } else {
        ru_compose_emit_index(idx);
    }
    return true;
}
