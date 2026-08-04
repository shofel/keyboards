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


if __name__ == "__main__":
    main()
