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


# --- slot set: ЙЦУКЕН's occupied keys are the reference -----------------------
#
# v1 shipped letters onto column 0 — keys that carry no letter even in ЙЦУКЕН,
# which is the empirical record of what this user actually presses. The comfort
# map scored them 1/0/0, a soft penalty the annealer was happy to pay. Column 0
# is now a hard exclusion, and the 33rd slot is the v+g vertical combo.

def test_column_zero_is_never_a_slot():
    assert all(c != 0 for (_r, c) in m.SLOTS)


def test_dot_keeps_its_home_and_is_not_a_letter_slot():
    assert m.DOT_SLOT == (2, 10)
    assert m.DOT_SLOT not in m.SLOTS


def test_slot_count_exactly_fits_the_alphabet():
    """33 letters, 33 slots: 32 grid keys (36 − 3 dead − 1 dot) + the v+g combo.
    An exact fit means every allowed key gets a letter, so no key the user does
    press is left empty."""
    assert len(m.ALPHABET) == 33
    assert len(m.SLOTS) == 33


def test_vg_combo_is_a_slot_on_the_left_index_finger():
    assert m.COMBO_SLOT in m.SLOTS
    assert m.finger_of(m.COMBO_SLOT[1]) == "L-index"
    assert m.hand_of(m.COMBO_SLOT[1]) == "L"


def test_vg_combo_has_its_own_comfort_score():
    assert m.comfort_of(m.COMBO_SLOT) == m.COMBO_COMFORT


def test_combo_carries_the_rarest_letter():
    """A chord is a last resort, not a good key: it costs a COMBO_TERM window
    and cannot be rolled, neither of which the single-key comfort map can say.
    The combo exists only because the alphabet is one slot longer than the
    grid, so whatever lands on it must be a letter that is almost never typed.
    (Scoring it at 2 put `э` there — and `э` carries это/этот/эти.)"""
    uni, bi = m.synthetic_russian()
    # The shared synthetic fixture cannot see this bug: its tail is too flat.
    # Use the REAL corpus tail, where `э` outnumbers `ъ` twentyfold, so the test
    # actually discriminates — with the combo scored as a merely-mediocre key
    # the annealer parks `э` on the chord and leaves `ъ` on a real one.
    uni = dict(uni)
    uni.update({"ъ": 18, "ё": 228, "ф": 252, "щ": 280, "ц": 317, "э": 389})
    lay = m.optimize(uni, bi, seed=7, iters=60000, restarts=3)
    combo_letter = next(ch for ch, s in lay.items() if s == m.COMBO_SLOT)
    assert combo_letter == "ъ", (
        f"combo got {combo_letter!r} (freq {uni[combo_letter]}) "
        f"instead of the rarest letter ъ (freq {uni['ъ']})")


def test_bottom_row_index_columns_score_eight():
    """User's correction: promote the lower index 6 -> 8."""
    assert m.COMFORT[2][4] == 8   # left index, bottom row
    assert m.COMFORT[2][7] == 8   # right index, bottom row


def test_optimize_fills_every_slot():
    """Exact fit -> the layout is a bijection letters <-> slots."""
    uni, bi = m.synthetic_russian()
    lay = m.optimize(uni, bi, seed=5, iters=500)
    assert set(lay.values()) == set(m.SLOTS)


def test_optimize_uses_the_right_inner_column_top_and_bottom():
    """The v1 defect, inverted: it freed (0,6)/(2,6) — where ЙЦУКЕН puts н and
    т — while taking column 0. With an exact fit these can never be empty."""
    uni, bi = m.synthetic_russian()
    lay = m.optimize(uni, bi, seed=6, iters=500)
    assert (0, 6) in lay.values()
    assert (2, 6) in lay.values()


def test_shipped_russian_layer_is_jcuken():
    """The shipped L_RUSSIAN is stock ЙЦУКЕН, by explicit choice.

    The optimiser in this module scores ЙЦУКЕН far worse than the layouts it
    finds, and that verdict still stands — see test_optimize_beats_jcuken_by_a_
    clear_margin. It was traded away deliberately: a better layout you have to
    relearn is worth less than a worse one you already type without thinking.
    The tool stays; the board keeps the familiar keys.

    Pinned position-by-position because a transcription slip in a layout typed
    from muscle memory is close to invisible — you would feel it as your own
    error long before you suspected the keymap."""
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    src = os.path.join(here, "..", "layouts", "split_3x6_3", "shofel", "keymap.c")
    with open(src, encoding="utf-8") as fh:
        text = fh.read()
    block = text.split("[L_RUSSIAN] = LAYOUT", 1)[1].split("*/", 1)[0]
    rows = [t for t in (ln.split() for ln in block.splitlines())
            if len(t) == 12 and all(len(x) == 1 for x in t)]
    assert len(rows) == 3, f"expected 3 rows of 12 glyphs, got {len(rows)}"
    got = {tok: (r, c)
           for r, row in enumerate(rows)
           for c, tok in enumerate(row)
           if tok not in ("·", ".")}
    assert got == m.JCUKEN, (
        "L_RUSSIAN differs from ЙЦУКЕН at: "
        f"{sorted(set(got.items()) ^ set(m.JCUKEN.items()))}")


def test_no_leftover_adaptation_layer():
    """The reverted board carries one Russian layer, not two. A stray second
    layer is a live surface on a keyboard about to be flashed."""
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    src = os.path.join(here, "..", "layouts", "split_3x6_3", "shofel", "keymap.c")
    with open(src, encoding="utf-8") as fh:
        text = fh.read()
    assert "L_RU_JCUKEN" not in text
    assert "KK_DQUO_RU" not in text


def test_comfort_map_matches_keymap_c():
    """The comfort map is duplicated in keymap.c (canonical) and here. The two
    drifting apart is silent and steers the whole search wrong, so pin it."""
    import os
    import re
    here = os.path.dirname(os.path.abspath(__file__))
    src = os.path.join(here, "..", "layouts", "split_3x6_3", "shofel", "keymap.c")
    with open(src, encoding="utf-8") as fh:
        text = fh.read()
    block = text.split("### Key comfort scores", 1)[1].split("```", 2)[1]
    rows = []
    for line in block.splitlines():
        nums = re.findall(r"(?<![\w.])\d(?![\w.])", line.replace("|", " "))
        if len(nums) == 12:
            rows.append([int(n) for n in nums])
    assert len(rows) == 3, f"expected 3 comfort rows in keymap.c, got {len(rows)}"
    assert rows == m.COMFORT, f"keymap.c {rows} != optimiser {m.COMFORT}"


def _run():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    for t in tests:
        t()
        print(f"ok  {t.__name__}")
    print(f"\nAll {len(tests)} ru-layout optimiser tests passed.")


if __name__ == "__main__":
    _run()
