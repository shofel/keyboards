#!/usr/bin/env python3
"""Prefix-collision lint for the leader sequence set.

A *timeoutless* leader (fire-on-unique-match) can only work if no leader
sequence is a proper prefix of another: the moment a complete match is seen it
fires, so a shorter sequence that is a prefix of a longer one makes the longer
one unreachable. This tool extracts the leader sequences from keymap.c and
reports any prefix collisions.

Until the timeoutless-leader cutover, the live set is knowingly NOT prefix-free
(bare `M`/`.` mouse toggles are prefixes of the `M,L`/`M,R`/`M,E` currencies),
so this is wired as an informational `make lint-leader`, not a blocking gate.
The cutover PR makes the set a prefix code and flips this to enforced.

Usage:
    check_leader_prefix.py [path/to/keymap.c]     # exit 1 if any collision
"""

import os
import re
import sys


def prefix_collisions(seqs):
    """Return [(shorter, longer), ...] where `shorter` is a *proper* prefix of
    `longer`. Equal-length (incl. duplicate) sequences are never reported."""
    seqs = [tuple(s) for s in seqs]
    out = []
    seen = set()
    for a in seqs:
        for b in seqs:
            if len(a) < len(b) and b[:len(a)] == a:
                pair = (a, b)
                if pair not in seen:
                    seen.add(pair)
                    out.append(pair)
    return out


def _array_body(text, name):
    """Return the initializer body of a C array declared `... name[] = { ... }`,
    i.e. the text between the initializer's opening brace and its closing `};`."""
    i = text.find(name + "[]")
    if i < 0:
        return ""
    brace = text.find("{", i)
    if brace < 0:
        return ""
    end = text.find("};", brace)
    return text[brace:end] if end >= 0 else text[brace:]


def parse_leader_sequences(text):
    """Extract every leader key-sequence from keymap.c as a tuple of key tokens
    (the `KC_` suffix). Covers both the `leader_seqs[]` table (1- and 2-key
    entries) and the emoji table, whose entries are reached as leader,{a|i},sel."""
    seqs = []

    body = _array_body(text, "leader_seqs")
    for mo in re.finditer(r"\{\s*KC_(\w+)\s*,\s*(KC_NO|KC_(\w+))\s*,", body):
        k1, k2 = mo.group(1), mo.group(2)
        seqs.append((k1,) if k2 == "KC_NO" else (k1, mo.group(3)))

    # Emoji: leader,{a|i},<sel>. Both A- and I-prefixed forms are live.
    emoji = _array_body(text, "emoji_seqs")
    for mo in re.finditer(r"\{\s*KC_(\w+)\s*,", emoji):
        sel = mo.group(1)
        seqs.append(("A", sel))
        seqs.append(("I", sel))

    return seqs


def _fmt(seq):
    return "leader," + ",".join(seq)


def main(argv):
    if len(argv) > 1:
        path = argv[1]
    else:
        here = os.path.dirname(os.path.abspath(__file__))
        path = os.path.join(here, "..", "layouts", "split_3x6_3", "shofel", "keymap.c")

    text = open(path, encoding="utf-8").read()
    seqs = parse_leader_sequences(text)
    cols = prefix_collisions(seqs)

    if cols:
        print(f"{len(cols)} leader prefix collision(s) — a timeoutless leader could not fire these:")
        for short, long in cols:
            print(f"  {_fmt(short)}   is a prefix of   {_fmt(long)}")
        print("\nMake the set a prefix code (relocate the shorter sequence or the longer family).")
        return 1

    print(f"OK: {len(seqs)} leader sequences form a prefix code.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
