#!/usr/bin/env python3
"""Optimise L_RUSSIAN for the Cantor, against this keyboard's own typing.

`L_RUSSIAN` is stock ЙЦУКЕН — the only layer on this board nobody designed. It
inherits GOST 6431-52 (1953), a standard whose real constraint was keeping
typewriter typebars from jamming. The cost shows up in ordinary words: `какие`
is five keystrokes and all five are the left index finger.

This places the 33 Cyrillic letters by annealing against two inputs that are
specific to THIS board and THIS user:

  - the comfort map already published in keymap.c (the same weights that drive
    symbol placement, so the Russian layer is scored the way every other layer
    is), and
  - real letter/bigram frequencies from `tools/typing_corpus.py --script
    cyrillic`, i.e. text actually typed here rather than someone else's corpus.

That second point is the whole reason not to simply adopt Вестник or Kharlamak
off the shelf: they are excellent, but they are optimised for a different corpus
on a different board. They are used here as baselines to beat, not as answers.

The one modelling detail that matters most: the index column and the inner
column are the SAME FINGER. Treating them as separate is the mistake that makes
ЙЦУКЕН look tolerable on paper while `какие` remains unusable.
"""

import argparse
import collections
import math
import os
import random
import sys

# Comfort weights, verbatim from the `### Key comfort scores` block in keymap.c.
# Higher = easier, 0-9. Combos resolve from layer 0, so one map serves every layer.
#    pinky2 pinky  ring   mid  index  inner | inner  index   mid   ring  pinky pinky2
COMFORT = [
    [1, 4, 6, 8, 6, 2,   2, 6, 8, 6, 4, 3],
    [0, 5, 7, 9, 9, 3,   3, 9, 9, 7, 5, 3],
    [0, 1, 4, 5, 6, 3,   3, 6, 5, 4, 1, 1],
]

ROWS, COLS = 3, 12

# Column -> finger. The index and inner columns share a finger, as do the pinky
# and outer columns; this is the crux of the whole exercise.
_FINGERS = {
    0: "L-pinky", 1: "L-pinky", 2: "L-ring", 3: "L-mid", 4: "L-index", 5: "L-index",
    6: "R-index", 7: "R-index", 8: "R-mid", 9: "R-ring", 10: "R-pinky", 11: "R-pinky",
}

ALPHABET = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"

# `.` keeps its current home; only the letters are optimised.
DOT_SLOT = (2, 10)
SLOTS = [(r, c) for r in range(ROWS) for c in range(COLS) if (r, c) != DOT_SLOT]

# ЙЦУКЕН exactly as it sits in keymap.c today — the control to beat.
JCUKEN = {}
for _r, _row in enumerate([
    "ёйцукенгшщзх",
    " фывапролджэ",
    " ячсмитьбю ъ",
]):
    for _c, _ch in enumerate(_row):
        if _ch != " ":
            JCUKEN[_ch] = (_r, _c)

# An SFB that also jumps a row is worse than one on adjacent rows.
SFB_ROW_JUMP = 1.0
# Scales the same-finger rate into the same units as effort. At 30, one point of
# SFB percentage costs about as much as 0.3 of average per-key comfort cost.
SFB_PENALTY = 30.0


def finger_of(col):
    return _FINGERS[col]


def hand_of(col):
    return "L" if col < 6 else "R"


def comfort_of(slot):
    return COMFORT[slot[0]][slot[1]]


def cost_of(slot):
    """Effort of a slot: 0 (easiest) to 9 (hardest)."""
    return 9 - comfort_of(slot)


def best_slot():
    return max(SLOTS, key=comfort_of)


def worst_slot():
    return min(SLOTS, key=comfort_of)


def is_sfb(a, b):
    """Same finger, different key. A repeated key is not an SFB — it is fast."""
    if a == b:
        return False
    return finger_of(a[1]) == finger_of(b[1])


def _sfb_weight(a, b):
    return 1.0 + SFB_ROW_JUMP * abs(a[0] - b[0])


def effort(layout, unigrams):
    """Frequency-weighted mean keystroke cost."""
    total = sum(unigrams.get(ch, 0) for ch in layout)
    if not total:
        return 0.0
    return sum(unigrams.get(ch, 0) * cost_of(slot) for ch, slot in layout.items()) / total


def sfb_rate(layout, bigrams):
    """TRUE share of bigram mass landing on one finger, in [0, 1].

    Deliberately unweighted: this is the number quoted against published layouts
    (Вестник reports 0.756% SFB), so it has to mean the same thing they mean. The
    row-jump weighting that the optimiser cares about lives in `sfb_cost`, not
    here — mixing the two would produce a "rate" above 1.0 and silently break
    every comparison."""
    total = sum(bigrams.values())
    if not total:
        return 0.0
    bad = 0
    for gram, n in bigrams.items():
        if len(gram) != 2:
            continue
        a, b = layout.get(gram[0]), layout.get(gram[1])
        if a is None or b is None:
            continue
        if is_sfb(a, b):
            bad += n
    return bad / total


def sfb_cost(layout, bigrams):
    """Same-finger mass weighted by row jump — what the optimiser minimises.

    A same-finger bigram that also spans two rows is a bigger reach than one on
    neighbouring rows, so it should cost more; but that weighting is an
    optimisation preference, not a measurement."""
    total = sum(bigrams.values())
    if not total:
        return 0.0
    bad = 0.0
    for gram, n in bigrams.items():
        if len(gram) != 2:
            continue
        a, b = layout.get(gram[0]), layout.get(gram[1])
        if a is None or b is None:
            continue
        if is_sfb(a, b):
            bad += n * _sfb_weight(a, b)
    return bad / total


def score(layout, unigrams, bigrams):
    """Lower is better."""
    return effort(layout, unigrams) + SFB_PENALTY * sfb_cost(layout, bigrams)


def _bigram_index(bigrams, letters):
    """Flatten bigrams to (a, b, n) plus a letter -> entry-ids index.

    A full rescore is O(all bigrams) and makes annealing hopelessly slow in
    Python. A swap only changes the bigrams that touch the two moved letters, so
    the index turns each proposal into a handful of lookups."""
    alphabet = set(letters)
    entries = []
    for gram, n in bigrams.items():
        if len(gram) == 2 and gram[0] in alphabet and gram[1] in alphabet:
            entries.append((gram[0], gram[1], n))
    index = collections.defaultdict(list)
    for i, (a, b, _n) in enumerate(entries):
        index[a].append(i)
        if b != a:
            index[b].append(i)
    return entries, index


def _entries_cost(entries, ids, layout):
    total = 0.0
    for i in ids:
        a, b, n = entries[i]
        pa = layout.get(a)
        pb = layout.get(b)
        if pa is None or pb is None:
            continue
        if is_sfb(pa, pb):
            total += n * _sfb_weight(pa, pb)
    return total


def optimize(unigrams, bigrams, seed=0, iters=200000, restarts=1, letters=ALPHABET):
    """Simulated annealing over letter->slot assignments.

    Deterministic for a given seed so a published layout can be reproduced.
    Scoring is incremental — see `_bigram_index`."""
    rng = random.Random(seed)
    entries, index = _bigram_index(bigrams, letters)
    uni_total = sum(unigrams.get(ch, 0) for ch in letters) or 1
    bi_total = sum(n for _a, _b, n in entries) or 1
    k_sfb = SFB_PENALTY / bi_total

    best_layout, best_cost = None, float("inf")

    for _restart in range(restarts):
        slots = list(SLOTS)
        rng.shuffle(slots)
        layout = {ch: slots[i] for i, ch in enumerate(letters)}
        free = slots[len(letters):]
        chars = list(layout)
        cur = score(layout, unigrams, bigrams)

        for i in range(iters):
            temp = max(1e-4, 1.2 * (1.0 - i / iters))
            a = rng.choice(chars)

            if free and rng.random() < 0.15:
                b, j = None, rng.randrange(len(free))
                touched = index[a]
                old_a, new_a = layout[a], free[j]
            else:
                b = rng.choice(chars)
                if a == b:
                    continue
                touched = set(index[a]) | set(index[b])
                old_a, new_a = layout[a], layout[b]

            before_sfb = _entries_cost(entries, touched, layout)
            before_uni = unigrams.get(a, 0) * cost_of(layout[a])
            if b is not None:
                before_uni += unigrams.get(b, 0) * cost_of(layout[b])

            if b is None:
                layout[a] = new_a
            else:
                layout[a], layout[b] = layout[b], layout[a]

            after_sfb = _entries_cost(entries, touched, layout)
            after_uni = unigrams.get(a, 0) * cost_of(layout[a])
            if b is not None:
                after_uni += unigrams.get(b, 0) * cost_of(layout[b])

            delta = (after_uni - before_uni) / uni_total + k_sfb * (after_sfb - before_sfb)

            if delta <= 0 or rng.random() < math.exp(-delta / temp):
                cur += delta
                if b is None:
                    free[j] = old_a
            else:
                if b is None:
                    layout[a] = old_a
                else:
                    layout[a], layout[b] = layout[b], layout[a]

        # Drift guard. The search is steered entirely by deltas; if the delta
        # arithmetic disagrees with a full rescore then every accept/reject
        # decision was made on a wrong number and the "optimised" layout is
        # noise that merely looks authoritative. Fail loudly instead.
        full = score(layout, unigrams, bigrams)
        if abs(full - cur) > 1e-6 * max(1.0, abs(full)):
            raise AssertionError(
                f"incremental score drifted from full rescore: {cur!r} vs {full!r}")
        if full < best_cost:
            best_cost, best_layout = full, dict(layout)

    return best_layout


def render(layout, dot=True):
    """3 rows of 12 tokens; `·` marks an unused key."""
    grid = [["·"] * COLS for _ in range(ROWS)]
    for ch, (r, c) in layout.items():
        grid[r][c] = ch
    if dot:
        grid[DOT_SLOT[0]][DOT_SLOT[1]] = "."
    return "\n".join(" ".join(row) for row in grid)


def load_freqs(path):
    """Read a `count<TAB>gram` frequency file written by typing_corpus.py."""
    out = {}
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line:
                continue
            count, _, gram = line.partition("\t")
            if not gram:
                continue
            try:
                out[gram] = int(count)
            except ValueError:
                continue
    return out


def synthetic_russian():
    """Small deterministic stand-in for the real corpus, for unit tests."""
    uni = {
        "о": 1098, "е": 845, "а": 801, "и": 735, "н": 670, "т": 626, "с": 547,
        "р": 473, "в": 454, "л": 440, "к": 349, "м": 321, "д": 298, "п": 281,
        "у": 262, "я": 201, "ы": 190, "ь": 174, "г": 170, "з": 165, "б": 159,
        "ч": 144, "й": 121, "х": 97, "ж": 94, "ш": 73, "ю": 64, "ц": 48,
        "щ": 36, "э": 32, "ф": 26, "ъ": 4, "ё": 4,
    }
    bi = {
        "ст": 100, "то": 95, "на": 90, "ен": 85, "ов": 80, "ни": 78, "ра": 75,
        "во": 70, "ко": 68, "по": 66, "но": 64, "от": 60, "ер": 58, "пр": 56,
        "ль": 54, "ро": 52, "ка": 50, "ет": 48, "ва": 46, "ал": 44, "ре": 42,
        "ор": 40, "ли": 38, "ти": 36, "ес": 34, "ла": 32, "ел": 30, "ит": 28,
        "ак": 26, "ан": 24, "ей": 22, "ог": 20, "ой": 18, "ом": 16, "ат": 14,
    }
    return uni, bi


# --- reporting ---------------------------------------------------------------

_BASELINES = {
    # Вестник (Ivan Koz, 2024) — natively 3x10, so it maps onto cols 1..10.
    "Вестник": ["цдргхфпаяэ", "стнкбьвоеи", "шзлмчжйыую"],
    # Kharlamak (Kharlee) — ANSI 12/11/10; first 10 of each row form its core.
    "Kharlamak": ["эьукюшпрдг", "иаоеямтнсв", "ъйыхёзблчж"],
}


def baseline_layout(rows):
    """Map a 3x10 published layout onto the Cantor's inner 10 columns."""
    lay = {}
    for r, row in enumerate(rows):
        for i, ch in enumerate(row):
            lay[ch] = (r, i + 1)
    return lay


RU_KEYCODE = {
    "а": "RU_A", "б": "RU_B", "в": "RU_V", "г": "RU_G", "д": "RU_D", "е": "RU_E",
    "ё": "RU_YO", "ж": "RU_ZH", "з": "RU_Z", "и": "RU_I", "й": "RU_Y", "к": "RU_K",
    "л": "RU_L", "м": "RU_M", "н": "RU_N", "о": "RU_O", "п": "RU_P", "р": "RU_R",
    "с": "RU_S", "т": "RU_T", "у": "RU_U", "ф": "RU_F", "х": "RU_H", "ц": "RU_TS",
    "ч": "RU_CH", "ш": "RU_SH", "щ": "RU_SHCH", "ъ": "RU_HARD", "ы": "RU_YERU",
    "ь": "RU_SOFT", "э": "RU_EE", "ю": "RU_YU", "я": "RU_YA",
}


def emit_keymap(layout):
    """Render the three LAYOUT rows as QMK keycodes.

    Unused keys become XX, never `__`. Transparent would fall through to the
    BASE layer, so an unused slot at (0,6)/(2,6) would type Latin `q`/`p` in the
    middle of Russian text — a silent, maddening bug."""
    grid = [["XX"] * COLS for _ in range(ROWS)]
    for ch, (r, c) in layout.items():
        grid[r][c] = RU_KEYCODE[ch]
    grid[DOT_SLOT[0]][DOT_SLOT[1]] = "RU_DOT"
    return "\n".join("           " + ", ".join(f"{k:<8}" for k in row) + "," for row in grid)


def word_report(layout, words):
    """Per-word same-finger check — the human-legible version of the metric."""
    out = []
    for w in words:
        pos = [layout[c] for c in w if c in layout]
        sfbs = sum(1 for a, b in zip(pos, pos[1:]) if is_sfb(a, b))
        hands = "".join(hand_of(p[1]) for p in pos)
        out.append(f"    {w:<10} SFB {sfbs}/{max(0, len(pos) - 1)}   {hands}")
    return "\n".join(out)


def report_row(name, layout, uni, bi):
    return (f"  {name:<16} score {score(layout, uni, bi):7.3f}   "
            f"effort {effort(layout, uni):5.3f}   "
            f"SFB {100 * sfb_rate(layout, bi):5.2f}%   "
            f"keys {len(layout)}")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--freqs", required=True,
                    help="directory holding corpus.1 / corpus.2 (see `make corpus-ru --out-dir`)")
    ap.add_argument("--seed", type=int, default=20260827)
    ap.add_argument("--iters", type=int, default=200000)
    ap.add_argument("--restarts", type=int, default=8)
    args = ap.parse_args(argv)

    uni = load_freqs(os.path.join(args.freqs, "corpus.1"))
    bi = load_freqs(os.path.join(args.freqs, "corpus.2"))
    uni = {k: v for k, v in uni.items() if k in ALPHABET}
    bi = {k: v for k, v in bi.items() if len(k) == 2 and all(c in ALPHABET for c in k)}
    if not uni:
        print("no Cyrillic frequencies found — run `make corpus-ru` first", file=sys.stderr)
        return 1

    print(f"corpus: {sum(uni.values())} letters, {sum(bi.values())} bigrams\n")

    best = optimize(uni, bi, seed=args.seed, iters=args.iters, restarts=args.restarts)

    print("scored on THIS corpus and THIS board's comfort map (lower is better):")
    print(report_row("ЙЦУКЕН", JCUKEN, uni, bi))
    for name, rows in _BASELINES.items():
        print(report_row(name, baseline_layout(rows), uni, bi))
    print(report_row("optimised", best, uni, bi))

    print("\noptimised layout:\n")
    print(render(best))

    words = ["какие", "никаких", "который", "человек", "теперь", "сегодня", "потому"]
    print("\nsame-finger check — ЙЦУКЕН:")
    print(word_report(JCUKEN, words))
    print("\nsame-finger check — optimised:")
    print(word_report(best, words))

    print("\nkeymap.c rows (unused keys are XX, never __):\n")
    print(emit_keymap(best))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
