#!/usr/bin/env python3
"""Off-target unit tests for opt_ru_layout.py.

The optimiser decides where 33 Cyrillic letters go on the Cantor. Two things
have to be right or the answer is confidently wrong:

  1. The FINGER map. `какие` is a disaster on ЙЦУКЕН precisely because the index
     column and the inner column are the SAME finger — if the model treats them
     as different fingers it will happily reproduce the bug it was built to fix.
  2. The SCORE has to actually rank known-good above known-bad. ЙЦУКЕН is the
     control: any scoring function that does not rate it badly is broken.

Frequencies come from the real corpus at runtime, so the tests use synthetic
counts to stay deterministic.
"""

import opt_ru_layout as m


# --- geometry ----------------------------------------------------------------

def test_index_and_inner_columns_are_one_finger():
    # THE bug that makes `какие` five presses of one finger.
    assert m.finger_of(4) == m.finger_of(5)      # left index + left inner
    assert m.finger_of(6) == m.finger_of(7)      # right inner + right index


def test_outer_and_pinky_columns_are_one_finger():
    assert m.finger_of(0) == m.finger_of(1)
    assert m.finger_of(10) == m.finger_of(11)


def test_hands_are_distinct():
    assert m.finger_of(5) != m.finger_of(6)
    assert m.hand_of(5) == "L" and m.hand_of(6) == "R"


def test_middle_and_ring_are_their_own_fingers():
    assert len({m.finger_of(c) for c in (1, 2, 3, 4)}) == 4


# --- same-finger bigrams -----------------------------------------------------

def test_sfb_same_finger_different_key():
    assert m.is_sfb((0, 4), (1, 5)) is True      # both left index, different keys


def test_same_key_repeat_is_not_an_sfb():
    # Typing a doubled letter is fast, not a same-finger conflict.
    assert m.is_sfb((1, 4), (1, 4)) is False


def test_different_hands_is_not_an_sfb():
    assert m.is_sfb((1, 4), (1, 7)) is False


# --- the control: ЙЦУКЕН must score badly ------------------------------------

def test_jcuken_kakie_is_four_sfbs():
    """`какие` on ЙЦУКЕН — the word that started this. All five keystrokes are
    the left index finger, so every one of the four transitions is an SFB."""
    lay = m.JCUKEN
    positions = [lay[c] for c in "какие"]
    sfbs = sum(1 for a, b in zip(positions, positions[1:]) if m.is_sfb(a, b))
    assert sfbs == 4, sfbs


def test_jcuken_kakie_is_all_one_finger():
    fingers = {m.finger_of(m.JCUKEN[c][1]) for c in "какие"}
    assert len(fingers) == 1


def test_jcuken_has_all_33_letters():
    assert len(m.JCUKEN) == 33
    assert len(set(m.JCUKEN.values())) == 33


# --- scoring -----------------------------------------------------------------

def test_effort_prefers_comfortable_positions():
    uni = {"а": 100}
    comfy = {"а": m.best_slot()}
    awful = {"а": m.worst_slot()}
    assert m.effort(comfy, uni) < m.effort(awful, uni)


def test_effort_weights_by_frequency():
    # Putting the COMMON letter on the good key must beat the reverse.
    uni = {"а": 100, "ъ": 1}
    good = {"а": m.best_slot(), "ъ": m.worst_slot()}
    bad = {"а": m.worst_slot(), "ъ": m.best_slot()}
    assert m.effort(good, uni) < m.effort(bad, uni)


def test_sfb_rate_counts_only_same_finger_pairs():
    lay = {"а": (0, 4), "б": (1, 5), "в": (1, 7)}
    assert m.sfb_rate(lay, {"аб": 10}) == 1.0     # same finger
    assert m.sfb_rate(lay, {"ав": 10}) == 0.0     # opposite hands


def test_sfb_rate_ignores_letters_not_on_the_layout():
    lay = {"а": (0, 4), "б": (1, 5)}
    assert m.sfb_rate(lay, {"аж": 10}) == 0.0


def test_score_rates_jcuken_worse_than_an_optimised_layout():
    """The end-to-end sanity check. If this fails the objective is not measuring
    what it claims to."""
    uni, bi = m.synthetic_russian()
    better = m.optimize(uni, bi, seed=1, iters=4000)
    assert m.score(better, uni, bi) < m.score(m.JCUKEN, uni, bi)


# --- the optimiser must return something legal -------------------------------

def test_optimize_places_every_letter_exactly_once():
    uni, bi = m.synthetic_russian()
    lay = m.optimize(uni, bi, seed=2, iters=500)
    assert set(lay) == set(m.ALPHABET)
    assert len(set(lay.values())) == len(m.ALPHABET)


def test_optimize_only_uses_allowed_slots():
    uni, bi = m.synthetic_russian()
    lay = m.optimize(uni, bi, seed=3, iters=500)
    assert set(lay.values()) <= set(m.SLOTS)


def test_incremental_scoring_agrees_with_full_rescore():
    """The annealer steers entirely by deltas. If the delta arithmetic drifts
    from a full rescore, every accept/reject was decided on a wrong number and
    the result is noise wearing a confident face. optimize() raises on drift, so
    a clean run over many iterations IS the assertion."""
    uni, bi = m.synthetic_russian()
    for seed in (11, 12, 13):
        m.optimize(uni, bi, seed=seed, iters=3000)


def test_optimize_beats_jcuken_by_a_clear_margin():
    uni, bi = m.synthetic_russian()
    lay = m.optimize(uni, bi, seed=5, iters=20000)
    assert m.sfb_rate(lay, bi) < m.sfb_rate(m.JCUKEN, bi)
    assert m.effort(lay, uni) < m.effort(m.JCUKEN, uni)


def test_optimize_is_deterministic_for_a_seed():
    uni, bi = m.synthetic_russian()
    a = m.optimize(uni, bi, seed=7, iters=800)
    b = m.optimize(uni, bi, seed=7, iters=800)
    assert a == b


# --- io ----------------------------------------------------------------------

def test_load_freqs_parses_count_tab_gram(tmp=None):
    import tempfile, os
    fd, path = tempfile.mkstemp()
    with os.fdopen(fd, "w", encoding="utf-8") as fh:
        fh.write("500\tо\n300\tа\n")
    got = m.load_freqs(path)
    os.unlink(path)
    assert got == {"о": 500, "а": 300}


def test_render_produces_three_rows_of_twelve():
    rows = m.render(m.JCUKEN).splitlines()
    assert len(rows) == 3
    for r in rows:
        assert len(r.split()) == 12


def _run():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    for t in tests:
        t()
        print(f"ok  {t.__name__}")
    print(f"\nAll {len(tests)} ru-layout optimiser tests passed.")


if __name__ == "__main__":
    _run()
