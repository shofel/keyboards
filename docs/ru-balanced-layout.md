# Balanced Russian layout — `L_RU_OPT`

A second Russian layer for the Cantor, an optimised alternative to stock ЙЦУКЕН.
Placed by [`tools/opt_ru_layout.py`](../tools/opt_ru_layout.py) — a balanced
multi-objective anneal (same-finger bigrams, rolls, lateral stretch, scissors) —
against this board's published comfort map and **82,357 letters of real Cyrillic
typing** (`make corpus-ru`). The goal was BOO's roll-feel *without* its annoying
(досадные) same-finger bigrams.

> There is an interactive version of this page — a rendered board with the
> comfort heatmap — in [`ru-balanced-layout.html`](ru-balanced-layout.html).
> (GitHub shows `.html` as source; open it locally, or via GitHub Pages. It is
> not inlined here because GitHub's Markdown sanitiser strips the `<script>`/
> `<style>` this page needs.)

## The layout

```
· у п я л э      ё д а м ч ж
ъ и в е н ц      ш к о т с з
· ы г ю р щ      ф б ь й . х
```

- **ъ** (the rarest letter) sits on the reclaimed **outer-pinky home key** — so
  every one of the 33 letters is a single keypress and no combo is needed.
- Six of the seven most common letters (о е т с н и) land on the home row; `а`
  sits on the top-row middle finger.
- `(0,0)` / `(2,0)` are `XX`, like L_RUSSIAN's — transparent would fall through
  to the base layer and type Latin mid-word.

## How it scores

On this corpus and comfort map (lower total score is better):

| Layout | SFB ↓ | Rolls ↑ | Alt | Effort ↓ | Score ↓ |
|--------|------:|--------:|----:|---------:|--------:|
| ЙЦУКЕН (stock GOST 1953) | 20.42% | 13.7% | 53.0% | 3.591 | 15.94 |
| Вестник (Koz 2024) | 2.11% | 16.2% | 68.9% | 2.334 | 3.45 |
| Kharlamak (Kharlee) | 2.73% | 11.3% | 71.1% | 2.246 | 3.76 |
| **Balanced (`L_RU_OPT`)** | **1.58%** | **29.9%** | 49.2% | **2.268** | **2.63** |

It beats both published Russian layouts on same-finger rate **and** rolls at
once. The lower alternation is deliberate — it converts cross-hand alternation
into same-hand rolls, without paying for it in same-finger collisions, because
SFB stays the heaviest term in the objective (one same-finger bigram is never
traded for a roll).

## The same-finger proof

Same-finger transitions per word — the stutter you feel. Every test word drops
to zero:

| word | ЙЦУКЕН | balanced |
|------|:------:|:--------:|
| какие | 4/4 | **0/4** |
| никаких | 4/6 | **0/6** |
| который | 3/6 | **0/6** |
| человек | 1/6 | **0/6** |
| теперь | 3/5 | **0/5** |
| сегодня | 1/6 | **0/6** |
| потому | 2/5 | **0/5** |

## How it ships

- **Coexists with ЙЦУКЕН.** `L_RUSSIAN` is untouched; the balanced layout is a
  separate layer, so muscle memory and the optimised layout are both available.
- **Activate with `leader → (r+n)`** — the leader key, then the `r+n` chord.
  This is the same mechanism the `r+v` (vim) / `r+w` (Windows) backend chords
  use: a `{KC_R, KC_N}` combo emits a `KK_RU_OPT` token that the armed leader
  captures, gated by `combo_should_trigger` to the armed state so it can't
  misfire in ordinary typing ("turn", "born"). Compose backend.
- **Status:** firmware compiles and `make test` is green; **not yet flashed**.

## Reproduce

```
make corpus-ru                # writes counts-only frequency files (add --out-dir)
python3 tools/opt_ru_layout.py --freqs <dir>   # seed 20260827, deterministic
```

The keymap rows (in [`keymap.c`](../layouts/split_3x6_3/shofel/keymap.c),
`[L_RU_OPT]`):

```c
XX,       RU_U,  RU_P,  RU_YA,   RU_L,  RU_EE,    RU_YO, RU_D,  RU_A,    RU_M,  RU_CH,  RU_ZH,
RU_HARD,  RU_I,  RU_V,  RU_E,    RU_N,  RU_TS,    RU_SH, RU_K,  RU_O,    RU_T,  RU_S,   RU_Z,
XX,       RU_YERU, RU_G, RU_YU,  RU_R,  RU_SHCH,  RU_F,  RU_B,  RU_SOFT, RU_Y,  RU_DOT, RU_H,
```
