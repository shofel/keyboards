#!/usr/bin/env python3
"""Off-target unit tests for check_leader_prefix.py.

Tests the prefix-freeness logic and the keymap parser on synthetic inputs, so
the check itself is proven correct independently of the live keymap (whose set
is intentionally *not* prefix-free until the timeoutless-leader cutover).
"""

import check_leader_prefix as m


def test_no_collision_distinct_singles():
    assert m.prefix_collisions([("R",), ("V",), ("E",)]) == []


def test_single_is_prefix_of_two_key():
    # M is a proper prefix of (M, L) -> a collision (the mouse-vs-currency case).
    got = m.prefix_collisions([("M",), ("M", "L")])
    assert got == [(("M",), ("M", "L"))], got


def test_two_singles_and_shared_prefix_two_keys_ok():
    # A is only ever a prefix (no bare A), so (A, T) and (A, R) do not collide.
    assert m.prefix_collisions([("A", "T"), ("A", "R")]) == []


def test_delete_family_collision():
    # D bare would collide with (D, A)/(D, U)/(D, W); no bare D -> no collision.
    assert m.prefix_collisions([("D", "A"), ("D", "U"), ("D", "W")]) == []
    got = m.prefix_collisions([("D",), ("D", "A")])
    assert got == [(("D",), ("D", "A"))], got


def test_identical_sequences_not_reported_as_prefix():
    # Mirror pairs emit the SAME sequence keys is impossible (keys differ), but a
    # duplicate tuple must not be reported as a prefix of itself.
    assert m.prefix_collisions([("R",), ("R",)]) == []


def test_parse_extracts_single_and_two_key():
    src = """
    static const leader_seq_t leader_seqs[] = {
      {KC_R,     KC_NO, lead_ru,   "ru"},
      {KC_M,     KC_NO, lead_mouse,"mouse"},
      {KC_M,     KC_L,  lead_lira, "lira"},
    };
    """
    seqs = m.parse_leader_sequences(src)
    assert ("R",) in seqs, seqs
    assert ("M",) in seqs, seqs
    assert ("M", "L") in seqs, seqs


def test_parse_expands_emoji_a_and_i():
    src = """
    static const leader_seq_t leader_seqs[] = {
      {KC_K, KC_NO, lead_kitty, "kitty"},
    };
    static const struct { uint16_t sel; const char *code; } emoji_seqs[] = {
      {KC_T, "@t"},
      {KC_R, "@r"},
    };
    """
    seqs = m.parse_leader_sequences(src)
    # emoji are leader,{a|i},<sel> -> both A- and I-prefixed forms exist
    assert ("A", "T") in seqs, seqs
    assert ("I", "T") in seqs, seqs
    assert ("A", "R") in seqs, seqs
    assert ("I", "R") in seqs, seqs


def test_real_keymap_mouse_currency_collision_is_detected():
    # The live keymap is knowingly NOT prefix-free (bare M/. mouse vs M,L/M,R/M,E
    # currencies). The lint must catch exactly that until the cutover fixes it.
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    keymap = os.path.join(here, "..", "layouts", "split_3x6_3", "shofel", "keymap.c")
    seqs = m.parse_leader_sequences(open(keymap, encoding="utf-8").read())
    cols = m.prefix_collisions(seqs)
    shorts = {c[0] for c in cols}
    assert ("M",) in shorts, f"expected bare-M collision, got {cols}"
    assert ("DOT",) in shorts, f"expected bare-. (DOT) collision, got {cols}"


def _run():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    for t in tests:
        t()
        print(f"ok  {t.__name__}")
    print(f"\nAll {len(tests)} prefix-lint tests passed.")


if __name__ == "__main__":
    _run()
