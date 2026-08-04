#!/usr/bin/env python3
"""Off-target test: currency + emoji additions to gen_unicode_compose.py."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_unicode_compose as g

def test_currency_in_xcompose():
    out = g.gen_xcompose()
    assert '<Multi_key> <dollar> <l> : "₺" U20BA' in out
    assert '<Multi_key> <dollar> <r> : "₽" U20BD' in out
    assert '<Multi_key> <dollar> <e> : "€" U20AC' in out

def test_no_duplicate_sequences():
    lines = [l for l in g.gen_xcompose().splitlines() if l.startswith("<Multi_key>")]
    seqs = [l.split(" : ")[0] for l in lines]
    assert len(seqs) == len(set(seqs)), "duplicate compose sequence"

# Emoji: the 11 flowers + reactions, under the private '@' (keysym 'at') prefix.
EMOJI_EXPECTED = {
    "t": ("🌷", 0x1F337),
    "r": ("🌹", 0x1F339),
    "c": ("🌸", 0x1F338),
    "h": ("🌺", 0x1F33A),
    "s": ("🌻", 0x1F33B),
    "d": ("🌼", 0x1F33C),
    "u": ("👍", 0x1F44D),
    "o": ("👌", 0x1F44C),
    "k": ("🤔", 0x1F914),
    "m": ("🧐", 0x1F9D0),
    "n": ("🤝", 0x1F91D),
}

def test_emoji_in_xcompose():
    out = g.gen_xcompose()
    for sel, (glyph, cp) in EMOJI_EXPECTED.items():
        assert f'<Multi_key> <at> <{sel}> : "{glyph}" U{cp:04X}' in out, f"missing emoji {sel}"

def test_emoji_prefix_isolated():
    # Every emoji sequence uses the <at> first-key, and exactly the 11 of them do —
    # so the private '@' prefix stays clear of the q / Q / dollar maps.
    lines = [l for l in g.gen_xcompose().splitlines() if l.startswith("<Multi_key>")]
    at_lines = [l for l in lines if l.split()[1] == "<at>"]
    assert len(at_lines) == len(EMOJI_EXPECTED), "emoji count under <at> prefix wrong"

def test_emoji_in_cheatsheet():
    out = g.gen_cheatsheet()
    for sel, (glyph, _cp) in EMOJI_EXPECTED.items():
        assert glyph in out and f"Compose @ {sel}" in out, f"missing cheatsheet {sel}"

if __name__ == "__main__":
    test_currency_in_xcompose(); test_no_duplicate_sequences()
    test_emoji_in_xcompose(); test_emoji_prefix_isolated(); test_emoji_in_cheatsheet()
    print("ok")
