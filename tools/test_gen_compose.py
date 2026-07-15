#!/usr/bin/env python3
"""Off-target test: currency additions to gen_unicode_compose.py."""
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

if __name__ == "__main__":
    test_currency_in_xcompose(); test_no_duplicate_sequences(); print("ok")
