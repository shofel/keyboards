# Generated Keymap Reference Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Generate `docs/reference.md` and the in-`LAYOUT` ASCII schemes from `keymap.c` (single source of truth), enforce freshness in `make test` and CI, and make the leader sequences data-driven so they generate too.

**Architecture:** One stdlib-only Python tool `tools/gen_layer_schemes.py` (extractor + renderer, `--check`/`--write` modes), mirroring the existing `tools/gen_unicode_compose.py` pattern. Phase 1: layers. Phase 2: leader-table refactor in C, then combos/leader/emoji sections. CI: GitHub Actions — host tests + QMK userspace build.

**Tech Stack:** Python 3 stdlib, plain-assert tests (`python3 tools/test_*.py`), GNU Make, QMK C, GitHub Actions.

**Spec:** `docs/superpowers/specs/2026-08-04-generated-keymap-reference-design.md` — read it first.

**Conventions:** Commit style is conventional-commits lowercase (`feat(tools): …`, `docs(readme): …`). Strict TDD: every Python behavior gets a failing test first; run `python3 tools/test_gen_layer_schemes.py` and watch it fail before implementing. All paths relative to repo root (the worktree). Every commit ends with `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>`.

---

## Task 1: Extractor (enum, LAYOUT bodies, tokens)

**Files:**
- Create: `tools/test_gen_layer_schemes.py`
- Create: `tools/gen_layer_schemes.py`

- [ ] **Step 1: Write failing tests**

Create `tools/test_gen_layer_schemes.py`:

```python
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

if __name__ == "__main__":
    test_layer_names(); test_extract_layers(); test_wrong_token_count_fatal()
    print("ok")
```

- [ ] **Step 2: Run to verify failure**

Run: `python3 tools/test_gen_layer_schemes.py`
Expected: `ModuleNotFoundError: No module named 'gen_layer_schemes'`

- [ ] **Step 3: Implement extractor**

Create `tools/gen_layer_schemes.py`:

```python
#!/usr/bin/env python3
"""
Layer schemes generated from keymap.c — the single source of truth.

Outputs (both from the same extracted model):
  docs/reference.md                      — whole file is generated
  keymap.c in-LAYOUT comment blocks      — the comment right after `LAYOUT_split_3x6_3(`

Modes:
  --check   exit 1 with a diff when either output is stale (wired into `make test`)
  --write   regenerate both outputs in place (`make gen-docs`)

Strictness is the point: unknown keycode, wrong token count, or unparseable
structure is a hard error — that is what keeps the source generatable.
"""
import argparse
import difflib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
KEYMAP = ROOT / "layouts/shofel/split_3x6_3/shofel/keymap.c"
DOC = ROOT / "docs/reference.md"

N_TOKENS = 42  # 3 rows of 12 + 6 thumbs


def die(msg):
    sys.exit(f"gen_layer_schemes: {msg}")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


def layer_names(src):
    m = re.search(r"enum my_layer_names \{(.*?)\};", src, re.S)
    if not m:
        die("enum my_layer_names not found")
    return [t.strip() for t in strip_comments(m.group(1)).split(",") if t.strip()]


def split_tokens(body):
    """Split on commas at paren depth 0; body must be comment-free."""
    toks, cur, depth = [], [], 0
    for ch in body:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            toks.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    toks.append("".join(cur))
    toks = [re.sub(r"\s+", " ", t).strip() for t in toks]
    return [t for t in toks if t]


def layout_spans(src):
    """Yield (name, body_start, body_end) per LAYOUT initializer, comment-aware."""
    for m in re.finditer(r"\[(\w+)\]\s*=\s*LAYOUT_split_3x6_3\(", src):
        i, depth = m.end(), 1
        while i < len(src) and depth:
            if src.startswith("/*", i):
                i = src.index("*/", i) + 2
                continue
            if src.startswith("//", i):
                i = src.index("\n", i) + 1
                continue
            if src[i] == "(":
                depth += 1
            elif src[i] == ")":
                depth -= 1
            i += 1
        yield m.group(1), m.end(), i - 1


def extract_layers(src):
    """[(name, [42 tokens])] in source order; hard error on count mismatch."""
    names = layer_names(src)
    layers = []
    for name, start, end in layout_spans(src):
        toks = split_tokens(strip_comments(src[start:end]))
        if len(toks) != N_TOKENS:
            die(f"{name}: expected {N_TOKENS} tokens, got {len(toks)}")
        layers.append((name, toks))
    if [n for n, _ in layers] != names:
        die(f"layer order mismatch: enum {names} vs initializers {[n for n, _ in layers]}")
    return layers
```

(Plus a `main()` stub at the bottom so the file is runnable; filled in Task 4:)

```python
def main():
    pass


if __name__ == "__main__":
    main()
```

- [ ] **Step 4: Run tests, verify pass**

Run: `python3 tools/test_gen_layer_schemes.py`
Expected: `ok`

- [ ] **Step 5: Commit**

```bash
git add tools/gen_layer_schemes.py tools/test_gen_layer_schemes.py
git commit -m "feat(tools): layer-scheme extractor for keymap.c"
```

---

## Task 2: Glyph map

**Files:**
- Modify: `tools/gen_layer_schemes.py`
- Modify: `tools/test_gen_layer_schemes.py`

- [ ] **Step 1: Write failing tests** (append to test file, add calls under `__main__`)

```python
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
```

- [ ] **Step 2: Run to verify failure** — `python3 tools/test_gen_layer_schemes.py` fails: `AttributeError: ... no attribute 'glyph'`.

- [ ] **Step 3: Implement.** Insert above `def main()`:

```python
CYR = {
    "A": "а", "B": "б", "V": "в", "G": "г", "D": "д", "E": "е", "YO": "ё",
    "ZH": "ж", "Z": "з", "I": "и", "Y": "й", "K": "к", "L": "л", "M": "м",
    "N": "н", "O": "о", "P": "п", "R": "р", "S": "с", "T": "т", "U": "у",
    "F": "ф", "H": "х", "TS": "ц", "CH": "ч", "SH": "ш", "SHCH": "щ",
    "YERU": "ы", "SOFT": "ь", "HARD": "ъ", "EE": "э", "YU": "ю", "YA": "я",
}

GLYPHS = {
    "__": "__", "XX": "·", "KK_NOOP": "np",
    "KC_QUOT": "'", "KC_COMM": ",", "KC_DOT": ".", "KC_SLASH": "/", "KC_MINUS": "-",
    "QK_LEAD": "LEAD", "KK_SHIFT": "sft", "KK_SYMBO": "SYM",
    "KK_RET": "ret", "LT(L_SYMBOLS, KC_ENTER)": "ret", "KC_SPACE": "spc",
    "RU_DOT": ".", "RU_MDASH": "—", "RU_NUM": "№", "RU_SECT": "§",
    "KC_GRV": "`", "KC_DLR": "$", "KC_CIRC": "^", "KC_EXLM": "!", "KC_ASTR": "*",
    "KC_PIPE": "|", "KC_AMPR": "&", "KC_TILD": "~", "KC_AT": "@", "KC_HASH": "#",
    "KC_COLN": ":", "KC_QUES": "?", "KC_PLUS": "+", "KC_PERC": "%",
    "KC_BSLS": "\\", "KC_DEL": "⌦", "KC_BSPC": "⌫", "KC_EQL": "=", "KC_SCLN": ";",
    "KC_PGUP": "pg↑", "KC_PGDN": "pg↓", "KC_UP": "↑", "KC_DOWN": "↓",
    "KC_LEFT": "←", "KC_RGHT": "→", "KC_HOME": "⇤", "KC_END": "⇥",
    "KC_ENTER": "⏎", "KC_TAB": "⮀",
    "KC_BRIU": "br↑", "KC_BRID": "br↓", "KC_VOLU": "vl↑", "KC_VOLD": "vl↓",
    "KC_MUTE": "mute", "DB_TOGG": "DBG", "QK_BOOT": "boot",
    "KK_MM_POLAR": "pol", "KK_MM_BISECT": "bis",
    "OM_U": "fwd", "OM_D": "bwd", "OM_L": "←", "OM_R": "→",
    "OM_W_U": "w↑", "OM_W_D": "w↓", "OM_SLOW": "slo", "OM_FAST": "fst",
    "OM_BTN1": "b1", "OM_BTN2": "b2", "OM_BTN3": "b3",
    "KK_BI_L": "←", "KK_BI_R": "→", "KK_BI_U": "↑", "KK_BI_D": "↓",
    "KK_BI_CLICK": "clk", "KK_BI_RESET": "rst",
}


def glyph(token):
    if token in GLYPHS:
        return GLYPHS[token]
    m = re.fullmatch(r"KC_([A-Z])$", token)
    if m:
        return m.group(1).lower()
    if re.fullmatch(r"KC_\d", token):
        return token[3:]
    if re.fullmatch(r"KC_F\d{1,2}", token):
        return token[3:]
    m = re.fullmatch(r"RU_(\w+)", token)
    if m and m.group(1) in CYR:
        return CYR[m.group(1)]
    die(f"no glyph for keycode {token!r} — add it to GLYPHS")
```

Note: `KK_RET` never appears in the arrays (the macro expands nowhere — arrays use `KK_RET` literally), but `LT(L_SYMBOLS, KC_ENTER)` is listed defensively; keep both entries.

- [ ] **Step 4: Run tests, verify pass** — expected `ok`. If `test_real_keymap_fully_covered` dies naming a keycode, add that keycode to `GLYPHS` (that is the designed failure mode).

- [ ] **Step 5: Commit** — `feat(tools): keycode→glyph map with hard error on unknowns`

---

## Task 3: Grid renderer

**Files:** same two files.

- [ ] **Step 1: Write failing test** (append; golden output is exact — three rows 12 wide, 8-space half gap, left thumbs right-aligned to the left half's edge, right thumbs at the right half's start):

```python
def test_render_grid_golden():
    toks = (
        ["KC_Q", "KC_W", "KC_E", "KC_R", "KC_T", "KC_Y"] + ["KC_U", "KC_I", "KC_O", "KC_P", "KC_A", "KC_S"]
        + ["XX", "KC_PGUP", "KC_D", "KC_F", "KC_G", "KC_H"] + ["KC_J", "KC_K", "KC_L", "KC_Z", "KC_X", "KC_C"]
        + ["__"] * 12
        + ["QK_LEAD", "KK_SHIFT", "KK_SYMBO", "KK_RET", "KC_SPACE", "QK_LEAD"]
    )
    expected = "\n".join([
        "q   w     e  r  t  y        u  i  o  p  a  s",
        "·   pg↑   d  f  g  h        j  k  l  z  x  c",
        "__  __    __ __ __ __       __ __ __ __ __ __",
        "        LEAD  sft  SYM        ret  spc  LEAD",
    ])
    assert g.render_grid(toks) == expected
```

**Do not hand-tweak the expected block if the implementation disagrees on whitespace** — first decide which one is wrong. The rules: per-column width = widest glyph in that column across the 3 main rows; cells joined by 2 spaces; halves joined by 8 spaces; trailing whitespace stripped per line; thumb cells joined by 2 spaces, left thumb cluster right-aligned so it ends where the left half ends, right thumb cluster starts at the right half's start column.

- [ ] **Step 2: Run to verify failure** — `AttributeError: ... 'render_grid'`.

- [ ] **Step 3: Implement:**

```python
GAP = " " * 8


def render_grid(tokens):
    gl = [glyph(t) for t in tokens]
    rows, thumbs = [gl[0:12], gl[12:24], gl[24:36]], gl[36:42]
    w = [max(len(r[c]) for r in rows) for c in range(12)]

    def half(cells, widths):
        return "  ".join(c.ljust(wd) for c, wd in zip(cells, widths))

    lines = [(half(r[:6], w[:6]) + GAP + half(r[6:], w[6:])).rstrip() for r in rows]
    left_w = len(half(["x" * wd for wd in w[:6]], w[:6]))
    lt, rt = "  ".join(thumbs[:3]), "  ".join(thumbs[3:])
    lines.append(" " * max(0, left_w - len(lt)) + lt + GAP + rt)
    return "\n".join(lines)
```

- [ ] **Step 4: Run tests.** If the golden differs, print both (`python3 -c "import sys; sys.path.insert(0,'tools'); import gen_layer_schemes as g; print(g.render_grid([...]))"`), verify the RULES above by hand, and fix the implementation (or, if the implementation follows the rules and the golden doesn't, fix the golden — but say so in the commit message).

- [ ] **Step 5: Commit** — `feat(tools): ascii grid renderer for layer schemes`

---

## Task 4: Doc + in-file outputs, --check/--write

**Files:** same two files, plus generated `docs/reference.md` later (Task 5).

- [ ] **Step 1: Write failing tests** (append):

```python
def test_doc_paragraph():
    src = (
        "/**\n * Mouse layer — POLAR mode.\n *\n * More prose that is NOT the first paragraph.\n */\n"
        "  [L_ONE] = LAYOUT_split_3x6_3(\n    " + ", ".join(TOKENS) + "\n  ),"
    )
    assert g.doc_paragraph(src, "L_ONE") == "Mouse layer — POLAR mode."

def test_gen_doc_contains_all_layers():
    doc = g.gen_doc(g.KEYMAP.read_text(encoding="utf-8"))
    for name in ["L_BOO", "L_RUSSIAN", "L_SYMBOLS", "L_NUM_NAV",
                 "L_FKEYS_SYS", "L_MOUSE", "L_MOUSE_BISECT"]:
        assert f"## {name}" in doc
    assert doc.startswith("<!-- GENERATED by tools/gen_layer_schemes.py")

def test_regen_keymap_idempotent():
    src = g.KEYMAP.read_text(encoding="utf-8")
    once = g.regen_keymap(src)
    assert g.regen_keymap(once) == once
    assert "GENERATED scheme" in once

def test_regen_keymap_marks_every_layer():
    once = g.regen_keymap(g.KEYMAP.read_text(encoding="utf-8"))
    assert once.count("GENERATED scheme") == 7
```

- [ ] **Step 2: Run to verify failure** — missing attributes.

- [ ] **Step 3: Implement** (replace the `main()` stub):

```python
def doc_paragraph(src, name):
    """First paragraph of the /** doc comment immediately above `[name] =`."""
    idx = src.find(f"[{name}]")
    if idx < 0:
        die(f"[{name}] initializer not found")
    head = src[:idx]
    end = head.rfind("*/")
    if end < 0 or head[end + 2:].strip():
        return ""  # no doc comment directly above
    start = head.rfind("/**", 0, end)
    if start < 0:
        return ""
    lines = []
    for raw in head[start + 3:end].splitlines():
        line = raw.strip().lstrip("*").strip()
        if not line and lines:
            break  # first paragraph only
        if line:
            lines.append(line)
    return " ".join(lines)


def gen_doc(src):
    out = [
        "<!-- GENERATED by tools/gen_layer_schemes.py — do not edit; run `make gen-docs`. -->",
        "",
        "# Keymap reference",
        "",
        "Source of truth: `layouts/shofel/split_3x6_3/shofel/keymap.c`.",
        "`__` = transparent, `·` = no-op.",
        "",
    ]
    for name, toks in extract_layers(src):
        out.append(f"## {name}")
        out.append("")
        prose = doc_paragraph(src, name)
        if prose:
            out.append(prose)
            out.append("")
        out.append("```")
        out.append(render_grid(toks))
        out.append("```")
        out.append("")
    return "\n".join(out)


def inline_block(toks):
    grid = "\n".join("       " + l for l in render_grid(toks).splitlines())
    return ("/* GENERATED scheme — edit the array, then `make gen-docs`.\n"
            + grid + "\n  */")


def regen_keymap(src):
    """Replace the comment right after each `LAYOUT_split_3x6_3(` with the scheme."""
    spans = list(layout_spans(src))
    for name, start, _end in reversed(spans):  # right-to-left keeps indices valid
        toks_all = dict(extract_layers(src))
        m = re.compile(r"\s*/\*.*?\*/", re.S).match(src, start)
        block = inline_block(toks_all[name])
        if m:
            src = src[:start] + block + src[m.end():]
        else:
            src = src[:start] + block + src[start:]
    return src


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--write", action="store_true")
    a = ap.parse_args()
    src = KEYMAP.read_text(encoding="utf-8")
    outputs = [
        (KEYMAP, regen_keymap(src)),
        (DOC, gen_doc(src)),
    ]
    if a.write:
        for path, text in outputs:
            path.write_text(text, encoding="utf-8")
            print(f"wrote {path.relative_to(ROOT)}")
        return
    stale = False
    for path, text in outputs:
        old = path.read_text(encoding="utf-8") if path.exists() else ""
        if old != text:
            stale = True
            sys.stdout.writelines(difflib.unified_diff(
                old.splitlines(keepends=True), text.splitlines(keepends=True),
                str(path), "generated"))
    if stale:
        sys.exit("gen_layer_schemes: outputs stale — run `make gen-docs`")
    print("schemes ok")
```

Note the subtlety in `regen_keymap`: after one replacement the other spans' indices shift, so it re-extracts nothing — it processes spans **right-to-left** using positions from the ORIGINAL scan, which stay valid for the not-yet-touched earlier part of the file. `extract_layers` inside the loop re-reads `src` each iteration only for tokens (name-keyed), which is cheap and safe.

- [ ] **Step 4: Run tests, verify pass** — `python3 tools/test_gen_layer_schemes.py` → `ok`.

- [ ] **Step 5: Commit** — `feat(tools): doc + in-file scheme generation with check/write modes`

---

## Task 5: One-time keymap.c prep + first regeneration

**Files:**
- Modify: `layouts/shofel/split_3x6_3/shofel/keymap.c`
- Create (generated): `docs/reference.md`

- [ ] **Step 1: Manual prep in keymap.c** (hand edits, before running the generator):
  1. Base layer: it has no doc comment. Add one directly above `[L_BOO] = LAYOUT_split_3x6_3(`:
     ```c
       /**
        * Base layer — the [BOO layout](https://ballerboo.github.io/boolayout/):
        * Dvorak modified for more rollover.
        */
     ```
  2. Symbol layer doc comment: delete ONLY the 4 lines of the embedded position table (the lines starting `*         pinky2 pinky ring` through `*  bot      ·     ·    №`) — it duplicates the generated grid. Keep every "Notes / mnemonics" bullet.
  3. No other manual edits — the inline `/* ... */` blocks inside the LAYOUT calls will be overwritten by the generator.

- [ ] **Step 2: Regenerate:** `python3 tools/gen_layer_schemes.py --write`
Expected output: `wrote layouts/.../keymap.c`, `wrote docs/reference.md`.

- [ ] **Step 3: Review the diff by eye:** `git diff` — check: (a) only comment blocks changed in keymap.c, zero keycode-array changes; (b) the base-layer scheme now shows `LEAD` on the left thumb (this fixes the known drift where the old comment showed `__`); (c) `docs/reference.md` has 7 layers with grids.

- [ ] **Step 4: Verify:** `make test` → all four suites pass (`test-schemes` prints `schemes ok`). Then `direnv exec . make build` → firmware compiles (proves the comment surgery didn't touch code). This takes ~1 min.

- [ ] **Step 5: Commit**

```bash
git add layouts/shofel/split_3x6_3/shofel/keymap.c docs/reference.md
git commit -m "docs(reference): generate layer schemes from keymap.c"
```

(If Makefile isn't updated yet — Task 6 — run the check directly: `python3 tools/gen_layer_schemes.py --check`.)

---

## Task 6: Makefile wiring (+ lazy qmk guard for CI)

**Files:**
- Modify: `Makefile`

- [ ] **Step 1:** Add `test-schemes` to the `test` target and a `gen-docs` target:

```make
# All off-target host tests (pure logic; no QMK, no hardware).
test: test-bisect test-oneshot test-compose test-schemes

test-schemes:
	python3 tools/gen_layer_schemes.py --check

# Regenerate docs/reference.md + the in-LAYOUT scheme comments from keymap.c.
gen-docs:
	python3 tools/gen_layer_schemes.py --write
```

Add `test-schemes gen-docs` to the `.PHONY` line.

- [ ] **Step 2: Make `make test` runnable without qmk** (CI has no qmk CLI in the test job). Replace the eager error:

```make
QMK_FIRMWARE_ROOT = $(shell qmk config -ro user.qmk_home 2>/dev/null | cut -d= -f2 | sed -e 's@^None$$@@g')
ifeq ($(QMK_FIRMWARE_ROOT),)
    $(error Cannot determine qmk_firmware location. `qmk config -ro user.qmk_home` is not set)
endif
```

with a lazy guard — delete the `ifeq/$(error)/endif` block, keep the assignment, and prepend this line to the `flash:` recipe and to the `%:` catch-all recipe:

```make
	@test -n "$(QMK_FIRMWARE_ROOT)" || { echo "qmk_firmware not configured: qmk config user.qmk_home" >&2; exit 1; }
```

(`build:` needs no guard — it calls `qmk compile`, which fails with its own message.)

- [ ] **Step 3: Verify:** `make test` passes in the devenv; and `env PATH=/usr/bin:/bin make test` (a PATH without qmk) still runs the four suites. Run the test tools' own suite too: `python3 tools/test_gen_layer_schemes.py`.

- [ ] **Step 4: Commit** — `build(make): test-schemes drift gate, gen-docs target, lazy qmk guard`

---

## Task 7: CI workflow

**Files:**
- Create: `.github/workflows/ci.yml`

- [ ] **Step 1: Pick the qmk_firmware pin.** Run `git -C /home/slava/workspaces-one/forks/qmk_firmware describe --tags` and `git -C /home/slava/workspaces-one/forks/qmk_firmware log --oneline -1` to see what the local fork tracks. Pin `qmk_ref` to the nearest upstream breakpoint tag (e.g. `0.30.4` — verify the tag exists: `git ls-remote --tags https://github.com/qmk/qmk_firmware.git | grep -o 'refs/tags/0\.[0-9.]*$' | sort -V | tail -5`).

- [ ] **Step 2: Write `.github/workflows/ci.yml`:**

```yaml
name: CI

on:
  push:
    branches: [main]
  pull_request:
  workflow_dispatch:

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Host tests (bisect, oneshot, compose, scheme drift)
        run: make test

  build:
    uses: qmk/.github/.github/workflows/qmk_userspace_build.yml@main
    with:
      qmk_repo: qmk/qmk_firmware
      qmk_ref: <PINNED_TAG_FROM_STEP_1>
```

The reusable workflow builds every target in `qmk.json` (both `cantor:shofel` and `handwired/dactyl_manuform/5x6_5:shofel` are already listed there).

- [ ] **Step 3: Verify what can be verified locally:** `python3 -c "import yaml,sys; yaml.safe_load(open('.github/workflows/ci.yml'))"` if PyYAML is available, else `nix run nixpkgs#yq-go -- e . .github/workflows/ci.yml` (yq-go is installed — just `yq e . .github/workflows/ci.yml`). Note in the final report: the build job's pin is validated only by the first CI run on GitHub; spec names the fallback (pin the user's fork) if upstream fails.

- [ ] **Step 4: Commit** — `ci: host tests + qmk userspace build on push/pr`

---

## Task 8: README + TODO (phase 1 close-out)

**Files:**
- Modify: `README.md`
- Modify: `TODO.md`

- [ ] **Step 1: README.** In `### Conventions`, replace the bullet
"Keep each layer's ASCII comment in sync with its `LAYOUT` array. When a PR changes a layer, resync that layer's comment block in the same PR." with:

```markdown
- Layer schemes — the in-`LAYOUT` comments in `keymap.c` and
  [docs/reference.md](docs/reference.md) — are GENERATED from the `LAYOUT`
  arrays by `tools/gen_layer_schemes.py`. After changing a layer, run
  `make gen-docs` in the same PR; `make test` (and CI) fail on drift.
```

Also add a one-line pointer near the top of the README (end of the `## Structure` list): `- docs/reference.md - generated keymap reference (layers; combos+leader in phase 2)`.

- [ ] **Step 2: TODO.md.** Remove task #1's row from the ranked table and renumber (result: absolute-mouse #1, docs-reference #2, shared-layout #3, readme #4, draw-by-hand #5, case #6, animations #7 — recompute rank numbers, keep scores). Remove the schemes bullet from the `## Dactyl` section ("generate clean schemes from layer definitions..."). Update the ranked-table date header to `2026-08-04`.

- [ ] **Step 3: Commit** — `docs(todo,readme): schemes are generated now; convention updated`

---

## Task 9 (phase 2): Data-driven leader sequences in C

**Files:**
- Modify: `layouts/shofel/split_3x6_3/shofel/keymap.c` (function `leader_end_user`, currently ~lines 348–460)

- [ ] **Step 1:** Replace the if-chain in `leader_end_user` with a table + dispatch. Keep `leader_resume()` first, keep the emoji block (`emoji_seqs` + its for-loop) exactly as-is at the end. Insert ABOVE `leader_end_user`:

```c
/* Leader sequences — data so tools/gen_layer_schemes.py can extract them into
 * docs/reference.md. k2 == KC_NO marks a one-key sequence; doc strings appear
 * verbatim in the generated reference. Mirror pairs (s·n, w·h, m·.) are two
 * rows sharing an action, so either hand can trigger them. */
typedef struct {
  uint16_t k1, k2;
  void (*act)(void);
  const char *doc;
} leader_seq_t;

/* Ru compose is the default backend; see the unicode_ru module. */
static void lead_ru(void)       { ru_backend = RU_BACKEND_COMPOSE; toggle_enable(L_RUSSIAN); }
static void lead_vim(void)      { ru_backend = RU_BACKEND_VIM; toggle_enable(L_RUSSIAN); }
static void lead_en(void)       { ru_backend = RU_BACKEND_COMPOSE; toggle_disable(); }
static void lead_reset(void)    { toggle_reset(); }
/* Esc mirrors the thumb esc combo: exits the toggle layer AND sends Esc.
 * tap_code alone would not re-enter process_record, so the KC_ESC ->
 * toggle_disable path would never fire. */
static void lead_esc(void)      { toggle_disable(); tap_code(KC_ESC); }
static void lead_ctl_esc(void)  { tap_code16(LCTL(KC_ESC)); }
static void lead_fkeys(void)    { toggle_enable(L_FKEYS_SYS); }
static void lead_mouse(void)    { toggle_enable(L_MOUSE); }
static void lead_num(void)      { toggle_enable(L_NUM_NAV); }
static void lead_lira(void)     { ru_emit_glyph("$l", 0x20BA); }
static void lead_rub(void)      { ru_emit_glyph("$r", 0x20BD); }
static void lead_eur(void)      { ru_emit_glyph("$e", 0x20AC); }
static void lead_del_all(void)  { tap_code16(LCTL(KC_A)); tap_code16(KC_DEL); }
static void lead_del_line(void) { tap_code16(LSFT(KC_HOME)); tap_code16(KC_DEL); }
static void lead_del_word(void) { tap_code16(LCTL(KC_BSPC)); }
static void lead_kitty(void)    { tap_code16(LGUI(KC_T)); }
static void lead_pscr(void)     { tap_code(KC_PSCR); }

static const leader_seq_t leader_seqs[] = {
  {KC_R,     KC_NO, lead_ru,       "Russian — compose backend (default)"},
  {KC_C,     KC_NO, lead_ru,       "Russian — compose backend (mirror of r)"},
  {KC_V,     KC_NO, lead_vim,      "Russian — vim backend (vim-native unicode)"},
  {KC_E,     KC_NO, lead_en,       "back to English (drop the toggle layer)"},
  {KC_SPACE, KC_NO, lead_reset,    "disable any toggle layer, cancel one-shots"},
  {KC_S,     KC_NO, lead_esc,      "Esc + exit toggle layer (mirror pair s·n)"},
  {KC_N,     KC_NO, lead_esc,      "Esc + exit toggle layer (mirror pair s·n)"},
  {KC_W,     KC_NO, lead_ctl_esc,  "Ctrl+Esc (mirror pair w·h)"},
  {KC_H,     KC_NO, lead_ctl_esc,  "Ctrl+Esc (mirror pair w·h)"},
  {KC_F,     KC_NO, lead_fkeys,    "F-keys / system layer (sticky)"},
  {KC_M,     KC_NO, lead_mouse,    "mouse layer, polar mode (mirror pair m·.)"},
  {KC_DOT,   KC_NO, lead_mouse,    "mouse layer, polar mode (mirror pair m·.)"},
  {KC_T,     KC_NO, lead_num,      "num/nav layer (sticky)"},
  {KC_M,     KC_L,  lead_lira,     "₺ lira"},
  {KC_DOT,   KC_L,  lead_lira,     "₺ lira (mirror)"},
  {KC_M,     KC_R,  lead_rub,      "₽ ruble"},
  {KC_DOT,   KC_R,  lead_rub,      "₽ ruble (mirror)"},
  {KC_M,     KC_E,  lead_eur,      "€ euro"},
  {KC_DOT,   KC_E,  lead_eur,      "€ euro (mirror)"},
  {KC_D,     KC_A,  lead_del_all,  "delete all (Ctrl+A, Del)"},
  {KC_D,     KC_U,  lead_del_line, "delete to line start (like Ctrl-U)"},
  {KC_D,     KC_W,  lead_del_word, "delete word (Ctrl+Backspace)"},
  {KC_K,     KC_NO, lead_kitty,    "kitty terminal (Gui+T)"},
  {KC_P,     KC_NO, lead_pscr,     "Print Screen"},
};
```

and the new function body (everything between `leader_resume();` and the emoji comment block is REPLACED by the loop):

```c
void leader_end_user(void) {
  leader_resume();

  for (size_t i = 0; i < sizeof(leader_seqs) / sizeof(leader_seqs[0]); i++) {
    const leader_seq_t *e = &leader_seqs[i];
    bool hit = (e->k2 == KC_NO) ? leader_sequence_one_key(e->k1)
                                : leader_sequence_two_keys(e->k1, e->k2);
    if (hit) { e->act(); }
  }

  /* Emoji: ... (existing block, unchanged) */
  ...
}
```

Preserve the prose comments that carried "why" (compose-default note, Esc semantics note) as shown; the per-sequence what-it-does now lives in the doc strings.

**Behavior invariants to check by eye:** the original code fires EVERY matching `if` independently — the loop preserves that (no `break`). One-key `m` vs two-key `m,l` never both match (leader matchers are exact). `leader,e` also resets the backend to compose — preserved in `lead_en`.

- [ ] **Step 2: Verify:** `direnv exec . make build` — firmware compiles. `make test` — host suites still green, but `test-schemes` will FAIL now (keymap.c changed → in-file schemes still match, doc unchanged… actually arrays didn't change, so it should still pass; if it fails, run `make gen-docs` and inspect why). Confirm `git diff` touches only `leader_end_user` and above-it additions.

- [ ] **Step 3: Commit** — `refactor(keymap): data-driven leader sequences (extractable table)`

---

## Task 10 (phase 2): Extract combos, leader table, emoji → new doc sections

**Files:**
- Modify: `tools/gen_layer_schemes.py`
- Modify: `tools/test_gen_layer_schemes.py`
- Regenerate: `docs/reference.md`

- [ ] **Step 1: Write failing tests** (append):

```python
def test_extract_combos_real():
    combos = g.extract_combos(g.KEYMAP.read_text(encoding="utf-8"))
    by_keys = {tuple(ks): act for ks, act in combos}
    assert by_keys[("KC_S", "KC_W")] == "KC_LBRC"
    assert by_keys[("KK_SHIFT", "KC_SPACE")] == "KC_ESC"
    assert len(combos) == 23

def test_extract_leader_seqs_real():
    seqs = g.extract_leader_seqs(g.KEYMAP.read_text(encoding="utf-8"))
    assert (["KC_R"], "Russian — compose backend (default)") in [(k, d) for k, d in seqs]
    assert (["KC_D", "KC_W"], "delete word (Ctrl+Backspace)") in [(k, d) for k, d in seqs]
    assert len(seqs) == 24

def test_extract_emoji_real():
    emo = g.extract_emoji(g.KEYMAP.read_text(encoding="utf-8"))
    assert ("t", "🌷") in emo and len(emo) == 11

def test_gen_doc_has_phase2_sections():
    doc = g.gen_doc(g.KEYMAP.read_text(encoding="utf-8"))
    for h in ["## Combos", "## Leader sequences", "## Emoji"]:
        assert h in doc
```

(23 combos = count the `COMBO(...)` entries in `key_combos[]`; recount when writing the test and use the real number. 24 = rows of `leader_seqs[]` from Task 9.)

- [ ] **Step 2: Run to verify failure.**

- [ ] **Step 3: Implement.** Add to `gen_layer_schemes.py`:

```python
COMBO_OUT = {  # combo outputs that aren't plain layer glyphs
    "KC_ESC": "Esc", "QK_BOOT": "bootloader", "QK_REBOOT": "reboot",
    "KK_FAT_RIGHT_ARROW": "=>", "KK_RIGHT_ARROW": "->",
    "KC_LBRC": "[", "KC_RBRC": "]", "KC_LPRN": "(", "KC_RPRN": ")",
    "KK_LANGLE": "< (« when shifted)", "KK_RANGLE": "> (» when shifted)",
    "OS_CTL": "one-shot Ctrl", "OS_ALT": "one-shot Alt", "OS_GUI": "one-shot Gui",
    "OSL(L_NUM_NAV)": "one-shot NUM_NAV", "OSL(L_FKEYS_SYS)": "one-shot FKEYS_SYS",
    "KC_DQUO": '"',
}


def combo_out(token):
    if token in COMBO_OUT:
        return COMBO_OUT[token]
    if token in GLYPHS or re.fullmatch(r"KC_([A-Z]|\d|F\d{1,2})", token):
        return glyph(token)
    die(f"no description for combo output {token!r} — add it to COMBO_OUT")


def extract_combos(src):
    """[( [trigger keycodes], output token )] in key_combos[] order."""
    arrays = {}
    for m in re.finditer(
            r"const uint16_t PROGMEM (\w+)\[\]\s*=\s*\{([^}]*)\};", src):
        toks = split_tokens(strip_comments(m.group(2)))
        if toks and toks[-1] == "COMBO_END":
            arrays[m.group(1)] = toks[:-1]
    body_m = re.search(r"combo_t key_combos\[\]\s*=\s*\{(.*?)\n\};", src, re.S)
    if not body_m:
        die("key_combos[] not found")
    combos = []
    for m in re.finditer(r"COMBO\((\w+),\s*([^)]+(?:\([^)]*\))?)\)",
                         strip_comments(body_m.group(1))):
        name, out = m.group(1), m.group(2).strip()
        if name not in arrays:
            die(f"combo key array {name!r} not found")
        combos.append((arrays[name], out))
    return combos


def extract_leader_seqs(src):
    """[( [1-2 keycodes], doc )] from the leader_seqs[] table."""
    body_m = re.search(r"leader_seq_t leader_seqs\[\]\s*=\s*\{(.*?)\n\};", src, re.S)
    if not body_m:
        die("leader_seqs[] not found")
    seqs = []
    for m in re.finditer(r'\{\s*(\w+),\s*(\w+),\s*\w+,\s*"([^"]*)"\s*\}',
                         strip_comments(body_m.group(1))):
        k1, k2, doc = m.groups()
        seqs.append(([k1] if k2 == "KC_NO" else [k1, k2], doc))
    if not seqs:
        die("leader_seqs[] parsed empty")
    return seqs


def extract_emoji(src):
    """[(selector char, glyph)] joining keymap.c emoji_seqs with the compose table."""
    import gen_unicode_compose as compose
    glyph_by_sel = {sel: gl for _cp, sel, gl in compose.EMOJI}
    body_m = re.search(r"\}\s*emoji_seqs\[\]\s*=\s*\{(.*?)\n\s*\};", src, re.S)
    if not body_m:
        die("emoji_seqs[] not found")
    emo = []
    for m in re.finditer(r'\{KC_(\w),\s*"@(\w)"\}', strip_comments(body_m.group(1))):
        key, sel = m.group(1).lower(), m.group(2)
        if key != sel:
            die(f"emoji selector mismatch: KC_{m.group(1)} vs @{sel}")
        if sel not in glyph_by_sel:
            die(f"emoji @{sel} missing from gen_unicode_compose.EMOJI")
        emo.append((sel, glyph_by_sel[sel]))
    if not emo:
        die("emoji_seqs[] parsed empty")
    return emo
```

Extend `gen_doc` — after the layers loop, append:

```python
    out.append("## Combos")
    out.append("")
    out.append("Chord both keys at once (positions are base-layer keys).")
    out.append("")
    for keys, action in extract_combos(src):
        trig = " + ".join(f"`{glyph(k)}`" for k in keys)
        out.append(f"- {trig} → {combo_out(action)}")
    out.append("")
    out.append("## Leader sequences")
    out.append("")
    out.append("Tap `LEAD`, then the keys in order.")
    out.append("")
    for keys, doc in extract_leader_seqs(src):
        seq = ", ".join(glyph(k) for k in keys)
        out.append(f"- `LEAD, {seq}` — {doc}")
    out.append("")
    out.append("## Emoji")
    out.append("")
    out.append("`LEAD, a, <sel>` or `LEAD, i, <sel>` (mirror pair), via the Compose backend.")
    out.append("")
    out.append("| sel | emoji |")
    out.append("|-----|-------|")
    for sel, gl in extract_emoji(src):
        out.append(f"| `{sel}` | {gl} |")
    out.append("")
    return "\n".join(out)
```

(Fold this into `gen_doc` as a single return path — i.e. move the existing `return` down. `glyph()` covers trigger keys `KC_S`, `KK_SHIFT`, `KC_SPACE`, `KK_NOOP`, `KK_SYMBO`, `KK_RET`, `KC_SLASH`, `KC_MINUS` etc.; anything missing dies loudly — add it to `GLYPHS`.)

- [ ] **Step 4: Run tests → green.** Then `make gen-docs` (regenerates `docs/reference.md` with the three new sections), `make test` (drift gate green again), eyeball `docs/reference.md`.

- [ ] **Step 5: Commit** — `feat(tools): combos, leader and emoji sections in the generated reference` (include the regenerated `docs/reference.md`).

---

## Task 11 (phase 2): Close-out — TODO + README pointer

**Files:**
- Modify: `TODO.md`, `README.md`

- [ ] **Step 1: TODO.md** — remove the docs-reference task row (old #3) from the ranked table, renumber, update date; delete the whole `## Docs` section body (the scattered-reference paragraph — it is done). 

- [ ] **Step 2: README.md** — in the Russian-unicode section, add after the emoji bullet: `All leader sequences, combos and layer schemes: see [docs/reference.md](docs/reference.md) (generated).` Update the phase-1 pointer text from "(layers; combos+leader in phase 2)" to just "generated keymap reference".

- [ ] **Step 3: Final full gate:** `make test` AND `direnv exec . make build` — paste both outputs in the report. Also `git log --oneline main..HEAD` to list the branch's commits.

- [ ] **Step 4: Commit** — `docs(todo,readme): reference doc replaces scattered leader/emoji docs`

---

## Self-review notes (already applied)

- Spec coverage: input contract → Tasks 1–2; grid → Task 3; outputs/modes → Task 4; comment prep + drift fix → Task 5; Makefile/CI → Tasks 6–7; conventions/TODO → Task 8; phase 2 source prep → Task 9; phase 2 sections → Task 10; close-out → Task 11.
- The `--check`-in-CI depends on Task 6's lazy-qmk Makefile change; Task 7's test job would otherwise die at Makefile parse time on the missing qmk CLI.
- Counts in Task 10 tests (23 combos, 24 leader rows) must be re-verified against the real file when writing those tests — the numbers here were counted from the current source and Task 9's table.
