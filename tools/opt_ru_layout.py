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
    [0, 1, 4, 5, 8, 3,   3, 8, 5, 4, 1, 1],
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

# Column 0 — the leftmost column of the left half — is never pressed. ЙЦУКЕН,
# the layout actually typed on this board for years, puts no letter on (1,0) or
# (2,0), and the user has confirmed the whole column is out. v1 ignored this:
# the comfort map scored those keys only 1/0/0, a soft penalty the annealer was
# happy to pay to park ш/х/ъ there. A score cannot express "never"; an exclusion
# can.
DEAD_COLS = {0}

# The 33rd slot is not a key. It is the v+g vertical combo — left inner column,
# top+home — which types `"` on the Latin layers and a Russian letter while
# L_RUSSIAN is live. Row 3 is a sentinel (there is no fourth physical row);
# column 5 keeps it on the correct finger, so it forms same-finger bigrams with
# the index and inner columns exactly as the two real keys under it do.
COMBO_SLOT = (3, 5)
# Scored below every real key on purpose. A chord is not a key: it costs a
# COMBO_TERM window before it resolves and it cannot be rolled into or out of,
# neither of which the single-key comfort map can express. Scoring it by its
# keys' comfort (2) made it *better* than the pinky-bottom keys, so the annealer
# rewarded it with `э` — the letter in это/этот/эти. The combo is here only
# because the alphabet is one letter longer than the grid, so it is the last
# resort and must take the letter that is almost never typed.
COMBO_COMFORT = 0

# 36 keys − 3 dead (column 0) − 1 for `.` = 32, plus the combo = 33: exactly the
# 33 letters of the alphabet. An exact fit means no key the user does press is
# left empty, which is how (0,6)/(2,6) — ЙЦУКЕН's н and т — come back into use.
SLOTS = [(r, c) for r in range(ROWS) for c in range(COLS)
         if (r, c) != DOT_SLOT and c not in DEAD_COLS] + [COMBO_SLOT]

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

# --- balanced-objective weights (folded into bigram_cost) --------------------
# The old objective was effort + SFB only: a pure effort model. "Balanced" adds
# rolls (rewarded), lateral stretch and scissors (penalised). SFB stays the
# heaviest term by a wide margin — one same-finger bigram must never be tradeable
# for one roll — so the "досадные SFB" of a roll-first layout cannot sneak back
# in. Rewards are deliberately gentle, so this is balanced, not roll-first:
# effort and SFB still lead, rolls only break ties toward better rhythm.
ROLL_IN = 4.0           # reward: an inward roll (toward the index finger)
ROLL_OUT = 2.0          # reward: an outward roll (toward the pinky)
LSB_PENALTY = 10.0      # adjacent fingers splayed >=2 columns (inner-column stretch)
SCISSOR_PENALTY = 12.0  # adjacent fingers two rows apart (a top<->bottom pinch)


def finger_of(col):
    return _FINGERS[col]


def hand_of(col):
    return "L" if col < 6 else "R"


def comfort_of(slot):
    if slot == COMBO_SLOT:
        return COMBO_COMFORT
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


def _sfb_row(slot):
    """Row charged for SFB row-jump distance. The combo has no row of its own —
    it is chorded across rows 0+1 — so charge it as the home row it rests on
    rather than letting the row-3 sentinel invent a three-row leap."""
    return 1 if slot == COMBO_SLOT else slot[0]


def _sfb_weight(a, b):
    return 1.0 + SFB_ROW_JUMP * abs(_sfb_row(a) - _sfb_row(b))


_FINGER_RANK = {"pinky": 0, "ring": 1, "mid": 2, "index": 3}


def _rank_in_hand(col):
    """0=pinky .. 3=index within a hand. Higher = further toward the centre, so a
    rising rank across a bigram is an inward roll."""
    return _FINGER_RANK[finger_of(col).split("-", 1)[1]]


def _adjacent_fingers(a, b):
    """Same hand, neighbouring fingers. Same finger (rank delta 0) is an SFB, not
    a roll, so it is excluded here."""
    return (hand_of(a[1]) == hand_of(b[1])
            and abs(_rank_in_hand(a[1]) - _rank_in_hand(b[1])) == 1)


def is_roll(a, b):
    """Two different adjacent fingers on one hand — the motion that feels fast."""
    return a != b and _adjacent_fingers(a, b)


def is_lsb(a, b):
    """Lateral-stretch bigram: adjacent fingers splayed two or more columns apart
    — on this board, the index reaching the inner column while its neighbour
    holds home."""
    return _adjacent_fingers(a, b) and abs(a[1] - b[1]) >= 2


def is_scissor(a, b):
    """Adjacent fingers two rows apart — a top<->bottom pinch. The combo has no
    real row, so it is charged at the home row it rests on (see _sfb_row)."""
    return _adjacent_fingers(a, b) and abs(_sfb_row(a) - _sfb_row(b)) == 2


def bigram_cost(a, b):
    """Net cost of one ordered bigram, in effort units. The SINGLE function that
    both the full score and the annealing delta route through, so the two can
    never disagree — the drift guard in optimize() enforces exactly this.
    Positive is bad, negative is good.

        same key                     0   (a fast repeat, not a conflict)
        same finger, different key   heavy positive   (SFB — the thing to avoid)
        opposite hands               0   (alternation — neither rolled nor strained)
        adjacent same-hand fingers   roll reward, less any stretch/scissor penalty
        same hand, non-adjacent      0   (a hand hurdle, but not same-finger)
    """
    if a == b:
        return 0.0
    if is_sfb(a, b):
        return SFB_PENALTY * _sfb_weight(a, b)
    if hand_of(a[1]) != hand_of(b[1]):
        return 0.0
    if not _adjacent_fingers(a, b):
        return 0.0
    inroll = _rank_in_hand(b[1]) > _rank_in_hand(a[1])
    cost = -(ROLL_IN if inroll else ROLL_OUT)
    if is_lsb(a, b):
        cost += LSB_PENALTY
    if is_scissor(a, b):
        cost += SCISSOR_PENALTY
    return cost


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


def _bigram_mass(layout, bigrams):
    """Frequency-weighted mean of bigram_cost — the balanced bigram term of the
    objective. Divides by the SAME total as sfb_rate so the units stay
    comparable, and iterates bigrams the SAME way the incremental scorer iterates
    entries, so the drift guard holds."""
    total = sum(bigrams.values())
    if not total:
        return 0.0
    acc = 0.0
    for gram, n in bigrams.items():
        if len(gram) != 2:
            continue
        a, b = layout.get(gram[0]), layout.get(gram[1])
        if a is None or b is None:
            continue
        acc += n * bigram_cost(a, b)
    return acc / total


def score(layout, unigrams, bigrams):
    """Lower is better. Effort plus the balanced bigram term — SFB, rolls,
    lateral stretch, scissors — all folded into bigram_cost."""
    return effort(layout, unigrams) + _bigram_mass(layout, bigrams)


def _bigram_share(layout, bigrams, pred):
    """Share of bigram mass matching a predicate on (slot_a, slot_b), in [0, 1] —
    the plain-percentage reporting counterpart to bigram_cost."""
    total = sum(bigrams.values())
    if not total:
        return 0.0
    hit = 0
    for gram, n in bigrams.items():
        if len(gram) != 2:
            continue
        a, b = layout.get(gram[0]), layout.get(gram[1])
        if a is None or b is None:
            continue
        if pred(a, b):
            hit += n
    return hit / total


def roll_rate(layout, bigrams):
    return _bigram_share(layout, bigrams, is_roll)


def alt_rate(layout, bigrams):
    return _bigram_share(layout, bigrams,
                         lambda a, b: hand_of(a[1]) != hand_of(b[1]))


def lsb_rate(layout, bigrams):
    return _bigram_share(layout, bigrams, is_lsb)


def scissor_rate(layout, bigrams):
    return _bigram_share(layout, bigrams, is_scissor)


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
    """Incremental bigram cost over just the touched entries. Routes through the
    SAME bigram_cost as score()/_bigram_mass — that shared function is what keeps
    the delta arithmetic honest (see the drift guard in optimize())."""
    total = 0.0
    for i in ids:
        a, b, n = entries[i]
        pa = layout.get(a)
        pb = layout.get(b)
        if pa is None or pb is None:
            continue
        total += n * bigram_cost(pa, pb)
    return total


def rarest_letter(unigrams, letters=ALPHABET):
    """The letter that goes on the chord. Ties break on alphabet order so a
    published layout stays reproducible."""
    return min(letters, key=lambda ch: (unigrams.get(ch, 0), ch))


def optimize(unigrams, bigrams, seed=0, iters=200000, restarts=1, letters=ALPHABET,
             pinned=None):
    """Simulated annealing over letter->slot assignments.

    Deterministic for a given seed so a published layout can be reproduced.
    Scoring is incremental — see `_bigram_index`.

    The chord slot is pinned rather than optimised: WHICH letter lives on it is
    a constraint, not something to discover. A chord costs a COMBO_TERM window
    before it resolves and cannot be rolled into or out of — costs the per-key
    comfort map has no way to express — so letting the annealer trade it against
    ordinary keys is trading with a broken price. Left free it parked `ю` (487
    in the corpus, and every `любой`/`юг`) on the chord to buy back 0.06pp of
    same-finger rate. By default the rarest letter is pinned there; pass
    `pinned={}` to let the annealer have it back."""
    if pinned is None:
        pinned = {rarest_letter(unigrams, letters): COMBO_SLOT}
    pinned_slots = set(pinned.values())
    movable = [ch for ch in letters if ch not in pinned]
    rng = random.Random(seed)
    entries, index = _bigram_index(bigrams, letters)
    uni_total = sum(unigrams.get(ch, 0) for ch in letters) or 1
    bi_total = sum(n for _a, _b, n in entries) or 1
    k_bi = 1.0 / bi_total   # bigram_cost already carries every weight

    best_layout, best_cost = None, float("inf")

    for _restart in range(restarts):
        slots = [s for s in SLOTS if s not in pinned_slots]
        rng.shuffle(slots)
        layout = {ch: slots[i] for i, ch in enumerate(movable)}
        layout.update(pinned)
        free = slots[len(movable):]
        # Pinned letters are never picked as swap partners, so they stay put.
        chars = list(movable)
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

            before_bi = _entries_cost(entries, touched, layout)
            before_uni = unigrams.get(a, 0) * cost_of(layout[a])
            if b is not None:
                before_uni += unigrams.get(b, 0) * cost_of(layout[b])

            if b is None:
                layout[a] = new_a
            else:
                layout[a], layout[b] = layout[b], layout[a]

            after_bi = _entries_cost(entries, touched, layout)
            after_uni = unigrams.get(a, 0) * cost_of(layout[a])
            if b is not None:
                after_uni += unigrams.get(b, 0) * cost_of(layout[b])

            delta = (after_uni - before_uni) / uni_total + k_bi * (after_bi - before_bi)

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


def combo_keys(layout):
    """The two letters the chord physically sits under, on this layout.

    Calling it the `v+g` combo is only meaningful on the Latin layers. Someone
    typing Russian sees the letters at those positions, so name it by those."""
    at = {slot: ch for ch, slot in layout.items()}
    return at.get((0, 5)), at.get((1, 5))


def combo_name(layout):
    a, b = combo_keys(layout)
    return f"{a}+{b}" if a and b else "v+g"


def render(layout, dot=True):
    """3 rows of 12 tokens; `·` marks an unused key. The combo letter is not on
    the grid, so it trails on its own line."""
    grid = [["·"] * COLS for _ in range(ROWS)]
    combo = None
    for ch, slot in layout.items():
        if slot == COMBO_SLOT:
            combo = ch
            continue
        grid[slot[0]][slot[1]] = ch
    if dot:
        grid[DOT_SLOT[0]][DOT_SLOT[1]] = "."
    out = "\n".join(" ".join(row) for row in grid)
    if combo is not None:
        out += f"\n{combo_name(layout)} combo: {combo}"
    return out


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
    combo = None
    for ch, slot in layout.items():
        if slot == COMBO_SLOT:
            combo = ch
            continue
        grid[slot[0]][slot[1]] = RU_KEYCODE[ch]
    grid[DOT_SLOT[0]][DOT_SLOT[1]] = "RU_DOT"
    body = "\n".join("           " + ", ".join(f"{k:<8}" for k in row) + "," for row in grid)
    if combo is not None:
        body += (f"\n           /* {combo_name(layout)} combo (v+g on Latin) -> "
                 f"{RU_KEYCODE[combo]}  ({combo}) */")
    return body


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
    return (f"  {name:<12} score {score(layout, uni, bi):7.3f}  "
            f"effort {effort(layout, uni):5.3f}  "
            f"SFB {100 * sfb_rate(layout, bi):5.2f}%  "
            f"roll {100 * roll_rate(layout, bi):5.1f}%  "
            f"alt {100 * alt_rate(layout, bi):5.1f}%  "
            f"LSB {100 * lsb_rate(layout, bi):4.1f}%  "
            f"scis {100 * scissor_rate(layout, bi):4.1f}%")


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
