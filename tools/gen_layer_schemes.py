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
KEYMAP = ROOT / "layouts/split_3x6_3/shofel/keymap.c"
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


LAYER_TITLES = {
    "L_BOO": "Base (BOO)",
    "L_RUSSIAN": "Russian",
    "L_SYMBOLS": "Symbols",
    "L_NUM_NAV": "Numbers & Navigation",
    "L_FKEYS_SYS": "F-keys & System",
    "L_MOUSE": "Mouse — Polar",
    "L_MOUSE_BISECT": "Mouse — Bisect",
}


def layer_title(name):
    if name in LAYER_TITLES:
        return LAYER_TITLES[name]
    die(f"no title for layer {name!r} — add it to LAYER_TITLES")


LEGEND = [
    ("np", "no-op filler key"),
    ("sft", "one-shot Shift"),
    ("SYM", "Symbol layer — left thumb (tap = one-shot, hold = momentary)"),
    ("ret", "Enter (tap) / Symbol layer (hold)"),
    ("spc", "Space"),
    ("LEAD", "leader key — starts a leader sequence"),
    ("⌫", "Backspace"), ("⌦", "Delete"), ("⏎", "Enter"), ("⮀", "Tab"),
    ("⇤", "Home"), ("⇥", "End"), ("pg↑", "Page Up"), ("pg↓", "Page Down"),
    ("pol", "mouse: polar mode"), ("bis", "mouse: bisect mode"),
    ("fwd", "mouse: move forward"), ("bwd", "mouse: move backward"),
    ("w↑", "mouse: wheel up"), ("w↓", "mouse: wheel down"),
    ("slo", "mouse: slower"), ("fst", "mouse: faster"),
    ("b1", "mouse: button 1"), ("b2", "mouse: button 2"), ("b3", "mouse: button 3"),
    ("clk", "mouse (bisect): click / hold to drag"),
    ("rst", "mouse (bisect): reset to full screen"),
    ("br↑", "brightness up"), ("br↓", "brightness down"),
    ("vl↑", "volume up"), ("vl↓", "volume down"), ("mute", "mute"),
    ("DBG", "toggle debug logging"), ("boot", "bootloader (for flashing)"),
]


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


COMBO_OUT = {  # combo outputs that aren't plain layer glyphs
    "KC_ESC": "Esc", "QK_BOOT": "bootloader", "QK_REBOOT": "reboot",
    "KK_FAT_RIGHT_ARROW": "=>", "KK_RIGHT_ARROW": "->",
    "KC_LBRC": "[", "KC_RBRC": "]", "KC_LPRN": "(", "KC_RPRN": ")",
    "KK_LANGLE": "< (« when shifted)", "KK_RANGLE": "> (» when shifted)",
    "OS_CTL": "one-shot Ctrl", "OS_ALT": "one-shot Alt", "OS_GUI": "one-shot Gui",
    "OSL(L_NUM_NAV)": "one-shot NUM_NAV", "OSL(L_FKEYS_SYS)": "one-shot FKEYS_SYS",
    "KC_DQUO": '"',
}

# Short labels for the combo board — each must fit a border cell (<= CW-2 chars).
COMBO_SHORT = {
    "OS_CTL": "Ctl", "OS_ALT": "Alt", "OS_GUI": "Gui",
    "OSL(L_NUM_NAV)": "Nav", "OSL(L_FKEYS_SYS)": "Fky",
    "KC_LBRC": "[", "KC_RBRC": "]", "KC_LPRN": "(", "KC_RPRN": ")",
    "KK_LANGLE": "<", "KK_RANGLE": ">", "KC_DQUO": '"',
}
CW = 5  # combo-board cell width


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
    for m in re.finditer(r"COMBO\((\w+)\s*,\s*([^)]+?(?:\([^)]*\))?)\)",
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


def _grid_positions(base_toks):
    """base keycode token -> (row, col), only for tokens unique in the 36-key grid."""
    seen = {}
    for i, t in enumerate(base_toks[:36]):
        seen.setdefault(t, []).append((i // 12, i % 12))
    return {t: ps[0] for t, ps in seen.items() if len(ps) == 1}


def _board_hand(rows, upper, lower):
    """rows: 3 lists of 6 glyphs; upper/lower: 6 border labels ('' = plain border)."""
    def cells(vals):
        return "│" + "│".join(v.center(CW) for v in vals) + "│"

    def seg(label):
        if not label:
            return "─" * CW
        if len(label) > CW - 2:
            die(f"combo-board label {label!r} too wide for a {CW}-char cell")
        return "─" + label.center(CW - 2) + "─"

    def border(labels):
        return "├" + "┼".join(seg(l) for l in labels) + "┤"

    def edge(a, mid, b):
        return a + mid.join("─" * CW for _ in range(6)) + b

    return [edge("┌", "┬", "┐"), cells(rows[0]), border(upper),
            cells(rows[1]), border(lower), cells(rows[2]), edge("└", "┴", "┘")]


def render_combo_board(base_toks, combos):
    """Keymap-drawer-style ASCII: a boxed base grid with each vertical (same-column,
    adjacent) combo's output on the border between its two keys — the upper border
    is home+above, the lower home+below. Combos whose keys are not vertically
    adjacent (thumbs, bottom-row rolls, corners) are listed as captions instead."""
    pos = _grid_positions(base_toks)
    upper, lower = [""] * 12, [""] * 12
    captions = []
    for keys, out in combos:
        ps = [pos.get(k) for k in keys]
        if len(keys) == 2 and all(ps) and ps[0][1] == ps[1][1] and abs(ps[0][0] - ps[1][0]) == 1:
            col, band = ps[0][1], {ps[0][0], ps[1][0]}
            label = COMBO_SHORT.get(out)
            if label is None:
                die(f"no COMBO_SHORT label for vertical combo output {out!r}")
            (upper if band == {0, 1} else lower)[col] = label
        else:
            captions.append((keys, out))
    grid = [[glyph(base_toks[r * 12 + c]) for c in range(12)] for r in range(3)]
    left = _board_hand([grid[r][0:6] for r in range(3)], upper[0:6], lower[0:6])
    right = _board_hand([grid[r][6:12] for r in range(3)], upper[6:12], lower[6:12])
    lines = [l + "    " + r for l, r in zip(left, right)]
    lines.append("")
    lines.append("Other combos (keys not vertically adjacent):")
    for keys, out in captions:
        lines.append("  " + " + ".join(glyph(k) for k in keys) + " → " + combo_out(out))
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
        f"Source of truth: `{KEYMAP.relative_to(ROOT)}`.",
        "`__` = transparent, `·` = no-op.",
        "",
        "## Legend",
        "",
    ]
    for gl, meaning in LEGEND:
        out.append(f"- `{gl}` — {meaning}")
    out.append("")
    out.append(
        "Arrows (`←` `→` `↑` `↓`) are directional per layer: navigation on "
        "Numbers & Navigation, steering on Mouse — Polar, screen-halving on "
        "Mouse — Bisect.")
    out.append("")
    for name, toks in extract_layers(src):
        out.append(f"## {layer_title(name)} <sub>`{name}`</sub>")
        out.append("")
        prose = doc_paragraph(src, name)
        if prose:
            out.append(prose)
            out.append("")
        out.append("```")
        out.append(render_grid(toks))
        out.append("```")
        out.append("")
    out.append("## Combos")
    out.append("")
    out.append("Combos fire on two physical key positions (they resolve from the base "
               "layer). A label on a border is the combo of the two keys it sits between: "
               "the upper border is *home + the key above* (mods, `\"`, F-keys), the lower "
               "border is *home + the key below* (brackets — opening on the left hand, "
               "closing on the right).")
    out.append("")
    out.append("```")
    out.append(render_combo_board(dict(extract_layers(src))["L_BOO"], extract_combos(src)))
    out.append("```")
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
