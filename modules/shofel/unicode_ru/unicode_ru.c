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
 *
 * `clear_mods()` is what actually protects the sequence: a one-shot that has
 * already fired holds its modifier in the real mod state, and a live Shift or
 * Ctrl there would turn Ctrl-V into something else. `clear_oneshot_mods()` is
 * separate — it stops a still-pending one-shot from being spent on our own
 * keystrokes. Both are restored afterwards, weak mods included.
 *
 * Caps Lock is neutralised for the same reason: it inverts letter case on the
 * host, so Shift-U would arrive as lowercase `u` and select the 4-digit form
 * instead — our trailing 4 digits would then land in the buffer as literal
 * text. Hex digits themselves are case-insensitive to vim, so only the selector
 * matters. */
void ru_vim_emit_codepoint(uint32_t cp) {
    if (cp > 0x10FFFF) {
        return;
    }
    uint8_t held = get_mods();
    uint8_t weak = get_weak_mods();
    bool    caps = host_keyboard_led_state().caps_lock;
    clear_oneshot_mods();
    clear_mods();
    clear_weak_mods();
    if (caps) {
        tap_code(KC_CAPS);
    }
    tap_code16(LCTL(KC_V));  // Ctrl-V
    tap_code16(LSFT(KC_U));  // Shift-U -> i_CTRL-V_U (8 hex digits)
    char buf[9];
    for (int i = 0; i < 8; i++) {
        buf[i] = "0123456789abcdef"[(cp >> ((7 - i) * 4)) & 0xF];
    }
    buf[8] = '\0';
    send_string(buf);
    if (caps) {
        tap_code(KC_CAPS);
    }
    set_mods(held);
    set_weak_mods(weak);
}

/* Windows mode: native Alt+numpad hex input (EnableHexNumpad). Hold Left Alt,
 * tap numpad `+`, type the 4 hex digits (0-9 on the numpad, A-F as letters),
 * release Alt. A faithful port of QMK's UNICODE_MODE_WINDOWS start/finish +
 * send_nibble_wrapper (quantum/unicode/unicode.c). BMP only (<= U+FFFF): Russian
 * and ₺/₽/€ are all BMP, and astral emoji stay on the compose backend anyway.
 * Requires the host's `HKCU\Control Panel\Input Method\EnableHexNumpad = 1`
 * (+reboot). UNVERIFIED on Linux — needs a Windows host to QA. */
void ru_windows_emit_codepoint(uint32_t cp) {
    if (cp > 0xFFFF) {
        return;  // EnableHexNumpad handles the BMP only
    }
    uint8_t held = get_mods();
    uint8_t weak = get_weak_mods();
    led_t   led  = host_keyboard_led_state();
    clear_oneshot_mods();
    clear_mods();
    clear_weak_mods();
    if (!led.num_lock) {
        tap_code(KC_NUM_LOCK);  // numpad digits require Num Lock on
    }
    register_code(KC_LEFT_ALT);
    wait_ms(UNICODE_TYPE_DELAY);
    tap_code(KC_KP_PLUS);
    for (int i = 3; i >= 0; i--) {
        uint8_t d  = (cp >> (i * 4)) & 0xF;
        uint8_t kc = (d < 10) ? (uint8_t)(KC_KP_1 + (10 + d - 1) % 10)  // 0-9 -> numpad
                              : (uint8_t)(KC_A + (d - 10));             // A-F -> letters
        tap_code(kc);
    }
    unregister_code(KC_LEFT_ALT);
    if (!led.num_lock) {
        tap_code(KC_NUM_LOCK);  // restore prior Num Lock state
    }
    set_mods(held);
    set_weak_mods(weak);
}

/* Emit a standalone glyph (e.g. « ») via the active backend: compose uses its
 * private 2-char code; vim and windows use the codepoint. */
void ru_emit_glyph(const char *compose_code, uint32_t cp) {
    switch (ru_backend) {
        case RU_BACKEND_VIM:     ru_vim_emit_codepoint(cp);          break;
        case RU_BACKEND_WINDOWS: ru_windows_emit_codepoint(cp);      break;
        default:                 ru_compose_emit_code(compose_code); break;
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
    switch (ru_backend) {
        case RU_BACKEND_VIM:
            ru_vim_emit_codepoint(pgm_read_dword(&unicode_map[idx]));
            break;
        case RU_BACKEND_WINDOWS:
            ru_windows_emit_codepoint(pgm_read_dword(&unicode_map[idx]));
            break;
        default:
            ru_compose_emit_index(idx);
            break;
    }
    return true;
}
