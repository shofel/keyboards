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
    "__": "__", "XX": "·",
    "KC_QUOT": "'", "KC_COMM": ",", "KC_DOT": ".", "KC_SLASH": "/", "KC_MINUS": "-",
    "QK_LEAD": "LEAD", "KK_LEAD_L": "LEAD", "KK_LEAD_R": "LEAD",
    "KK_SHIFT": "sft", "KK_SYMBO": "SYM",
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
    "PB_1": "nois",
    "KK_MM_POLAR": "pol",
    "OM_U": "fwd", "OM_D": "bwd", "OM_L": "←", "OM_R": "→",
    "OM_W_U": "w↑", "OM_W_D": "w↓", "OM_SLOW": "slo", "OM_FAST": "fst",
    "OM_BTN1": "b1", "OM_BTN2": "b2", "OM_BTN3": "b3",
}


LAYER_TITLES = {
    "L_BOO": "Base (BOO)",
    "L_RUSSIAN": "Russian",
    "L_SYMBOLS": "Symbols",
    "L_NUM_NAV": "Numbers & Navigation",
    "L_FKEYS_SYS": "F-keys & System",
    "L_MOUSE": "Mouse — Polar",
}


def layer_title(name):
    if name in LAYER_TITLES:
        return LAYER_TITLES[name]
    die(f"no title for layer {name!r} — add it to LAYER_TITLES")


LEGEND = [
    ("sft", "one-shot Shift"),
    ("SYM", "Symbol layer — left thumb (tap = one-shot, hold = momentary)"),
    ("ret", "Enter (tap) / Symbol layer (hold)"),
    ("spc", "Space"),
    ("LEAD", "leader key — starts a leader sequence"),
    ("⌫", "Backspace"), ("⌦", "Delete"), ("⏎", "Enter"), ("⮀", "Tab"),
    ("⇤", "Home"), ("⇥", "End"), ("pg↑", "Page Up"), ("pg↓", "Page Down"),
    ("pol", "mouse: polar mode"),
    ("fwd", "mouse: move forward"), ("bwd", "mouse: move backward"),
    ("w↑", "mouse: wheel up"), ("w↓", "mouse: wheel down"),
    ("slo", "mouse: slower"), ("fst", "mouse: faster"),
    ("b1", "mouse: button 1"), ("b2", "mouse: button 2"), ("b3", "mouse: button 3"),
    ("br↑", "brightness up"), ("br↓", "brightness down"),
    ("vl↑", "volume up"), ("vl↓", "volume down"), ("mute", "mute"),
    ("nois", "toggle focus audio (brown noise) — programmable button 1"),
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
    "KK_RIGHT_ARROW": "->", "KK_FAT_RIGHT_ARROW": "=>",
    "KK_FAT_LEFT_ARROW": "<=", "KK_LEFT_ARROW": "<-",
    "KK_RU_VIM": "Russian — vim backend (leader-armed only)",
    "KK_RU_WIN": "Russian — Windows backend (leader-armed only)",
    "KC_LBRC": "[", "KC_RBRC": "]", "KC_LPRN": "(", "KC_RPRN": ")",
    "KC_LCBR": "{", "KC_RCBR": "}",
    "KK_LANGLE": "`<` / `«`", "KK_RANGLE": "`>` / `»`",
    "OS_CTL": "one-shot Ctrl", "OS_ALT": "one-shot Alt", "OS_GUI": "one-shot Gui",
    "OSL(L_NUM_NAV)": "one-shot NUM_NAV", "OSL(L_FKEYS_SYS)": "one-shot FKEYS_SYS",
    "KC_DQUO": '"',
}

# Short labels for the combo board — each must fit a border cell (<= CW-2 chars).
COMBO_SHORT = {
    "KC_ESC": "Esc",
    "OS_CTL": "Ctl", "OS_ALT": "Alt", "OS_GUI": "Gui",
    "OSL(L_NUM_NAV)": "Nav", "OSL(L_FKEYS_SYS)": "Fky",
    "KC_LBRC": "[", "KC_RBRC": "]", "KC_LPRN": "(", "KC_RPRN": ")",
    "KC_LCBR": "{", "KC_RCBR": "}",
    "KK_LANGLE": "<", "KK_RANGLE": ">", "KC_DQUO": '"',
    "KK_RIGHT_ARROW": "->", "KK_FAT_RIGHT_ARROW": "=>",
    "KK_FAT_LEFT_ARROW": "<=", "KK_LEFT_ARROW": "<-",
    "KK_RU_VIM": "RUv", "KK_RU_WIN": "RUw",
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
    """[( [1-2 keycodes], act, doc )] from the leader_seqs[] table. `act` (the
    handler function name) identifies mirror pairs — the two rows sharing an
    action."""
    body_m = re.search(r"leader_seq_t leader_seqs\[\]\s*=\s*\{(.*?)\n\};", src, re.S)
    if not body_m:
        die("leader_seqs[] not found")
    seqs = []
    for m in re.finditer(r'\{\s*(\w+),\s*(\w+),\s*(\w+),\s*"([^"]*)"\s*\}',
                         strip_comments(body_m.group(1))):
        k1, k2, act, doc = m.groups()
        seqs.append(([k1] if k2 == "KC_NO" else [k1, k2], act, doc))
    if not seqs:
        die("leader_seqs[] parsed empty")
    return seqs


def group_leader_seqs(seqs):
    """Collapse mirror pairs (rows sharing an action) into one entry, preserving
    first-appearance order. Returns [(key_sequences, doc)] with the (mirror ...)
    annotation stripped from the doc. Fail loud if rows in a group describe
    different things (drift): they must share a base once the trailing
    parenthetical qualifier — (default), (mirror of r), (mirror) — is removed."""
    groups, order = {}, []
    for keys, act, doc in seqs:
        if act not in groups:
            groups[act] = {"seqs": [], "docs": []}
            order.append(act)
        groups[act]["seqs"].append(keys)
        groups[act]["docs"].append(doc)
    def base(d):
        return re.sub(r"\s*\([^)]*\)\s*$", "", d).strip()
    out = []
    for act in order:
        docs = groups[act]["docs"]
        if len({base(d) for d in docs}) > 1:
            die(f"leader group {act} has divergent docs: {docs}")
        doc = re.sub(r"\s*\(mirror[^)]*\)", "", docs[0]).strip()
        out.append((groups[act]["seqs"], doc))
    return out


def bold_selector(word, sel):
    """`word` with the first occurrence of the selector char bolded for markdown
    (e.g. bold_selector("think", "k") -> "thin**k**"). The char is not always
    word-initial; fail loud if the selector isn't in the mnemonic at all."""
    i = word.find(sel)
    if i < 0:
        die(f"emoji selector {sel!r} not found in mnemonic {word!r}")
    return f"{word[:i]}**{word[i]}**{word[i + 1:]}"


def extract_emoji(src):
    """[(selector char, glyph, mnemonic)] joining keymap.c emoji_seqs with the
    compose table (the single source for the glyph + mnemonic word)."""
    import gen_unicode_compose as compose
    info_by_sel = {sel: (gl, word) for _cp, sel, gl, word in compose.EMOJI}
    body_m = re.search(r"\}\s*emoji_seqs\[\]\s*=\s*\{(.*?)\n\s*\};", src, re.S)
    if not body_m:
        die("emoji_seqs[] not found")
    emo = []
    for m in re.finditer(r'\{KC_(\w),\s*"%(\w)"\}', strip_comments(body_m.group(1))):
        key, sel = m.group(1).lower(), m.group(2)
        if key != sel:
            die(f"emoji selector mismatch: KC_{m.group(1)} vs %{sel}")
        if sel not in info_by_sel:
            die(f"emoji %{sel} missing from gen_unicode_compose.EMOJI")
        gl, word = info_by_sel[sel]
        emo.append((sel, gl, word))
    if not emo:
        die("emoji_seqs[] parsed empty")
    return emo


def extract_design(src):
    """The layout-rationale markdown between the @design-begin / @design-end
    markers in keymap.c, with the C comment gutter (' * ') stripped from each
    line. The source comment is authored as markdown, so this is a copy, not a
    translation."""
    m = re.search(r"@design-begin(.*?)@design-end", src, re.S)
    if not m:
        die("@design-begin/@design-end markers not found in keymap.c")
    lines = [re.sub(r"^\s?\*\s?", "", ln) for ln in m.group(1).splitlines()]
    return "\n".join(lines).strip("\n")


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


def _is_vertical(pos, keys):
    """True if the two keys are the same column, adjacent rows (a border combo)."""
    ps = [pos.get(k) for k in keys]
    return (len(keys) == 2 and all(ps)
            and ps[0][1] == ps[1][1] and abs(ps[0][0] - ps[1][0]) == 1)


def nonadjacent_combos(base_toks, combos):
    """Combos whose two keys are not vertically adjacent (thumbs, bottom-row
    rolls, corners) — rendered as position diagrams rather than board labels."""
    pos = _grid_positions(base_toks)
    return [(keys, out) for keys, out in combos if not _is_vertical(pos, keys)]


def render_combo_board(base_toks, combos):
    """Keymap-drawer-style ASCII: a boxed base grid with each vertical (same-column,
    adjacent) combo's output on the border between its two keys — the upper border
    is home+above, the lower home+below. Non-adjacent combos are diagrammed
    separately (see nonadjacent_combos)."""
    pos = _grid_positions(base_toks)
    upper, lower = [""] * 12, [""] * 12
    for keys, out in combos:
        if not _is_vertical(pos, keys):
            continue
        ps = [pos[k] for k in keys]
        col, band = ps[0][1], {ps[0][0], ps[1][0]}
        label = COMBO_SHORT.get(out)
        if label is None:
            die(f"no COMBO_SHORT label for vertical combo output {out!r}")
        (upper if band == {0, 1} else lower)[col] = label
    # Keep only the 8 home-row anchors (a o e s / n t r i) as position markers;
    # blank the rest so the board reads as a combo map, not a full keycap grid.
    anchor_cols = {1, 2, 3, 4, 7, 8, 9, 10}
    grid = [[glyph(base_toks[r * 12 + c]) if r == 1 and c in anchor_cols else ""
             for c in range(12)] for r in range(3)]
    left = _board_hand([grid[r][0:6] for r in range(3)], upper[0:6], lower[0:6])
    right = _board_hand([grid[r][6:12] for r in range(3)], upper[6:12], lower[6:12])
    return "\n".join(l + "    " + r for l, r in zip(left, right))


# --- position-only diagrams (shared by leader sequences and non-adjacent combos)

# Thumb token (36-41) -> column index in the rendered thumb row. The cluster is
# shifted one column inboard, toward the MCU, to match where the thumbs sit on
# the board: the middle thumb (37 sft / 40 spc) under the inner-index column
# (z left / p right), the outer thumb (36 / 41 LEAD) one further out (w / h),
# and the inner thumb (38 SYM / 39 ret) in the centre gap under the MCU. The
# inter-half gap is widened to 7 (see render_position_diagram) so both inner
# thumbs get a full slot with air around them instead of colliding at centre.
_THUMB_COL = {36: 8, 37: 10, 38: 12, 39: 16, 40: 18, 41: 20}


def _hand(i):
    """'L' or 'R' for a base token index (0-41)."""
    if i >= 36:
        return "L" if i < 39 else "R"
    return "L" if i % 12 < 6 else "R"


def resolve_positions(base_toks, keys):
    """Base-layer token indices for a combo/leader key list. A key that occurs
    once resolves directly; a key present on both hands is disambiguated to the
    hand of the unambiguous keys it is chorded with. (No current combo/leader
    has a two-handed key — the two outer thumbs carry distinct leader keycodes,
    KK_LEAD_L/KK_LEAD_R, precisely so the reset chord stays unambiguous — but the
    logic is kept so the generator survives one being added. A chord of only
    two-handed keys, with no unambiguous mate to pick the hand, would die.)"""
    occ = {k: [i for i, t in enumerate(base_toks) if t == k] for k in set(keys)}
    hands = {_hand(occ[k][0]) for k in keys if len(occ[k]) == 1}
    out = []
    for k in keys:
        idxs = occ[k]
        if not idxs:
            die(f"key {k!r} not found in the base layer")
        if len(idxs) == 1:
            out.append(idxs[0])
            continue
        cands = [i for i in idxs if _hand(i) in hands]
        if len(cands) != 1:
            die(f"cannot resolve ambiguous key {k!r} to one position")
        out.append(cands[0])
    return out


def render_position_diagram(marks, dot="·", hit="●", labels=None):
    """A blanked base-layout diagram (3×12 main split 6+6, plus a 6-key thumb
    row) with `marks` (base token indices) emphasized. Combos pass a `marks` set
    and every hit is an anonymous `●` (the two keys fire simultaneously). Leaders
    pass a `labels` dict {index: str} that numbers the presses in order (LEAD=0,
    then 1, 2, …); a labelled position shows its digit instead of `●`."""
    labels = labels or {}
    def cell(i):
        if i in labels:
            return labels[i]
        return hit if i in marks else dot
    lines = []
    for r in range(3):
        left = " ".join(cell(r * 12 + c) for c in range(6))
        right = " ".join(cell(r * 12 + c) for c in range(6, 12))
        lines.append(left + "       " + right)
    row = [" "] * 29
    for tok, col in _THUMB_COL.items():
        row[col] = cell(tok)
    lines.append("".join(row).rstrip())
    return "\n".join(lines)


LEADER_TOKENS = ("QK_LEAD", "KK_LEAD_L", "KK_LEAD_R")  # keycodes that arm the leader


def leader_labels(base_toks, key_seqs):
    """Position -> step-number label for a leader group's diagram. `LEAD` (both
    outer thumbs) is step 0; the keys pressed after it are 1, 2, …. A mirror pair
    shares step numbers across hands — both members of step n get the same digit,
    so the diagram reads 'press LEAD, then key 1 (either hand)'. Fail loud if a
    group assigns one position two different steps (source drift), rather than
    silently mislabel — matching group_leader_seqs and the rest of the tool."""
    labels = {i: "0" for i, t in enumerate(base_toks) if t in LEADER_TOKENS}
    for ks in key_seqs:
        for step, idx in enumerate(resolve_positions(base_toks, ks), start=1):
            if labels.get(idx, str(step)) != str(step):
                die(f"leader diagram: position {idx} is both step "
                    f"{labels[idx]} and {step}")
            labels[idx] = str(step)
    return labels


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


# GitHub heading-anchor blocklist (github-slugger): these chars are dropped
# from the slug. Space -> hyphen; underscore and hyphen survive.
_GH_DROP = set(map(chr, [92, 39, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44,
                         46, 47, 58, 59, 60, 61, 62, 63, 64, 91, 93, 94, 96,
                         123, 124, 125, 126])) | {
    chr(c) for c in list(range(0x2000, 0x2070)) + list(range(0x2E00, 0x2E80))
}


def gh_anchor(heading):
    """GitHub fragment id for a `##`-heading. Mirrors github-slugger: strip
    inline HTML (GitHub renders tags away), lowercase, drop the punctuation
    blocklist, spaces -> hyphens. The <sub>`enum`</sub> wrapper vanishes but
    the enum text stays in the slug."""
    text = re.sub(r"<[^>]*>", "", heading).lower()
    return "".join("-" if ch == " " else ch for ch in text if ch not in _GH_DROP)


def gen_doc(src):
    out = [
        "<!-- GENERATED by tools/gen_layer_schemes.py — do not edit; run `make gen-docs`. -->",
        "",
        "# Keymap reference",
        "",
        f"Source of truth: `{KEYMAP.relative_to(ROOT)}`.",
        "`__` = transparent, `·` = no-op.",
        "",
    ]
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
    out.append("Why the same-column pairs are the safe ones — the column stagger — is "
               "explained under [Modifiers](#modifiers) in Design.")
    out.append("")
    base = dict(extract_layers(src))["L_BOO"]
    combos = extract_combos(src)
    out.append("```")
    out.append(render_combo_board(base, combos))
    out.append("```")
    out.append("")
    angle = [(ks, o) for ks, o in combos if o in ("KK_LANGLE", "KK_RANGLE")]
    if angle:
        notes = "; ".join(
            f"{' + '.join(glyph(k) for k in ks)} → {combo_out(o)}" for ks, o in angle)
        out.append(f"The angle combos are shift-aware — {notes}. A held (or "
                   "one-shot) Shift picks the second glyph of each pair, so the "
                   "Latin layers give `<` `>` unshifted and the guillemets "
                   "shifted. The Russian layer inverts that: `«` `»` come "
                   "unshifted there, since Russian prose quotes with them, and "
                   "`<` `>` take the Shift.")
        out.append("")
    nonadj = nonadjacent_combos(base, combos)
    if nonadj:
        out.append("The rest fire on keys that aren't vertically adjacent — thumbs, "
                   "bottom-row rolls, and the outer corners. Each diagram marks the two "
                   "trigger positions:")
        out.append("")
        for keys, o in nonadj:
            out.append(" + ".join(glyph(k) for k in keys) + " → " + combo_out(o))
            out.append("")
            out.append("```")
            out.append(render_position_diagram(set(resolve_positions(base, keys))))
            out.append("```")
            out.append("")
    out.append("## Leader sequences")
    out.append("")
    out.append("Tap `LEAD`, then the keys in order. Mirror pairs (either hand) share "
               "one entry; the diagram numbers the presses — `0` is `LEAD` (either "
               "outer thumb), then `1`, `2` for the keys after it.")
    out.append("")
    for key_seqs, doc in group_leader_seqs(extract_leader_seqs(src)):
        triggers = " / ".join(
            "`LEAD, " + ", ".join(glyph(k) for k in ks) + "`" for ks in key_seqs)
        out.append(f"{triggers} — {doc}")
        out.append("")
        out.append("```")
        out.append(render_position_diagram(set(), labels=leader_labels(base, key_seqs)))
        out.append("```")
        out.append("")
    out.append("## Emoji")
    out.append("")
    out.append("`LEAD, a, <sel>` or `LEAD, i, <sel>` (mirror pair), via the Compose backend.")
    out.append("")
    out.append("| sel | mnemonic | emoji |")
    out.append("|-----|----------|-------|")
    for sel, gl, word in extract_emoji(src):
        out.append(f"| `{sel}` | {bold_selector(word, sel)} | {gl} |")
    out.append("")
    out.append("## Design")
    out.append("")
    out.append(extract_design(src))
    out.append("")
    out.append("## Legend")
    out.append("")
    for gl, meaning in LEGEND:
        out.append(f"- `{gl}` — {meaning}")
    out.append("")
    out.append(
        "Arrows (`←` `→` `↑` `↓`) are directional per layer: navigation on "
        "Numbers & Navigation, steering on Mouse — Polar.")
    out.append("")
    return "\n".join(insert_toc(out))


def insert_toc(out):
    """Insert a Contents list of every `## ` section before the first one.
    Link text drops the <sub>`enum`</sub> tail; the anchor keeps it (GitHub
    slugs the whole rendered heading)."""
    first = next((i for i, l in enumerate(out) if l.startswith("## ")), None)
    if first is None:
        die("no ## sections found for the ToC")
    toc = ["## Contents", ""]
    for l in out[first:]:
        if l.startswith("## "):
            heading = l[3:]
            title = re.sub(r"\s*<sub>.*?</sub>", "", heading)
            toc.append(f"- [{title}](#{gh_anchor(heading)})")
    toc.append("")
    return out[:first] + toc + out[first:]


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
