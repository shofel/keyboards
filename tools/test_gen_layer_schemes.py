#!/usr/bin/env python3
"""Off-target tests for gen_layer_schemes.py (layer-scheme generator)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_layer_schemes as g

# 42 tokens: rows of 12/12/12 + 6 thumbs. Paren-nested token included on purpose.
TOKENS = (
    ["KC_Q"] * 11 + ["LT(L_SYMBOLS, KC_ENTER)"]
    + ["KC_A"] * 12
    + ["KC_Z"] * 12
    + ["KC_SPACE"] * 6
)

SNIPPET = (
    "enum my_layer_names {\n  L_ONE,\n  L_TWO,\n};\n"
    "const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {\n"
    "  [L_ONE] = LAYOUT_split_3x6_3(/* old scheme\n   q q q */\n    "
    + ", ".join(TOKENS)
    + "\n  ),\n"
    "  [L_TWO] = LAYOUT_split_3x6_3( // line comment\n    "
    + ", ".join(TOKENS)
    + "\n  ),\n"
    "};\n"
)

def test_layer_names():
    assert g.layer_names(SNIPPET) == ["L_ONE", "L_TWO"]

def test_extract_layers():
    layers = g.extract_layers(SNIPPET)
    assert [name for name, _ in layers] == ["L_ONE", "L_TWO"]
    for _, toks in layers:
        assert len(toks) == 42
        assert toks[11] == "LT(L_SYMBOLS, KC_ENTER)"  # comma inside parens kept
        assert toks[0] == "KC_Q" and toks[41] == "KC_SPACE"

def test_wrong_token_count_fatal():
    bad = SNIPPET.replace(", ".join(TOKENS), ", ".join(TOKENS[:41]), 1)
    try:
        g.extract_layers(bad)
    except SystemExit:
        pass
    else:
        raise AssertionError("41 tokens should be fatal")

def test_glyph_rules():
    assert g.glyph("KC_A") == "a"
    assert g.glyph("KC_7") == "7"
    assert g.glyph("KC_F12") == "F12"
    assert g.glyph("RU_YO") == "ё"
    assert g.glyph("__") == "__"
    assert g.glyph("XX") == "·"
    assert g.glyph("KC_BSPC") == "⌫"
    assert g.glyph("LT(L_SYMBOLS, KC_ENTER)") == "ret"

def test_unknown_keycode_fatal():
    try:
        g.glyph("KC_TOTALLY_NEW")
    except SystemExit:
        pass
    else:
        raise AssertionError("unknown keycode should be fatal")

def test_real_keymap_fully_covered():
    src = g.KEYMAP.read_text(encoding="utf-8")
    layers = g.extract_layers(src)
    assert len(layers) == 7
    for name, toks in layers:
        for t in toks:
            g.glyph(t)  # SystemExit here = missing glyph for a real keycode

if __name__ == "__main__":
    test_layer_names(); test_extract_layers(); test_wrong_token_count_fatal()
    test_glyph_rules(); test_unknown_keycode_fatal(); test_real_keymap_fully_covered()
    print("ok")
