# Emoji via the Compose backend (no UCIS)

**Date:** 2026-08-04
**TODO item:** #1 — "Make ucis and unicodemap work together (→ then emoji via ucis)".

## Problem

The keymap wants emoji input, but the two obvious QMK features collide:

- `unicode_ru` enables `UNICODEMAP` (used purely as a data store — its keycodes
  are intercepted in `ru_unicode_process` and emitted by userspace compose/vim
  backends, bypassing QMK's unicode input entirely).
- `ucis_emoji` enables `UCIS`.

QMK forbids both: `quantum/unicode/unicode.c:35` hard-`#error`s when more than one
of `UNICODE` / `UNICODEMAP` / `UCIS` is enabled. So the two modules cannot compile
together, which is why `keymap.c` carried a dead `/* UCIS emoji — disabled
(module conflict with unicodemap) */` marker.

Worse, even if the guard were bypassed, QMK's `ucis.c` emits through
`register_unicode()` — the ibus-hex input path this firmware **deliberately tore
out** (rolling-unsafe, needs an input mode) while working to shed the QMK fork.
So QMK's UCIS is the wrong mechanism twice over.

## Decision

**Don't use UCIS at all.** Emoji become standalone glyphs in the *existing*
compose pipeline — exactly like currency (`₺ ₽ €`) and guillemets (`« »`) already
are. `gen_unicode_compose.py` is already the single source of truth that emits
host-wide `Compose + <private-code>` sequences; emoji are just more entries.

This dissolves the conflict instead of fighting it: only `UNICODEMAP` stays
enabled (via `unicode_ru`), the `#error` never triggers, no fork patching, no
ibus hex, host-wide, works in any app (chat, editors, terminals).

## Design

### Compose codes

New private prefix **`@`** (keysym `at`), chosen for zero collisions with the
system Compose table — verified: only `q` and `$` (both already taken) are
otherwise collision-free; `@` needs Shift but that is already proven fine by the
`$` currency prefix (both go through `send_string`, which applies its own shift).

The 11 emoji from the old `ucis_emoji` table, each with a mnemonic selector:

| code | emoji | | code | emoji |
|------|-------|-|------|-------|
| `@t` | 🌷 tulip     | | `@u` | 👍 thumbup   |
| `@r` | 🌹 rose      | | `@o` | 👌 ok        |
| `@c` | 🌸 cherry    | | `@k` | 🤔 think     |
| `@h` | 🌺 hibiscus  | | `@m` | 🧐 monocle   |
| `@s` | 🌻 sunflower | | `@n` | 🤝 handshake |
| `@d` | 🌼 daisy     | |      |              |

Emoji are standalone glyphs: **no `enum unicode_names` entry, no `ru_compose_code[]`
row** (mirrors `CURRENCY`/`EXTRA`). The generator's new `EMOJI` list feeds only
`gen_xcompose()` and `gen_cheatsheet()`.

### Keyboard trigger — symmetric mirror pair

Leader keys `a` (left home) and `i` (right home) are a geometric mirror pair and
both currently unbound, so there is no one-key overload (unlike `s`, which is
Esc). Both fire the same emoji:

```
leader , a , <sel>   →  ru_compose_emit_code("@<sel>")   →  Compose @ <sel>  →  emoji
leader , i , <sel>   →  (same)
```

Always compose (host-wide); `ru_backend`/vim is irrelevant for emoji. The 22
sequences (11 selectors × 2 prefixes) are driven from one `{selector_kc, code}`
table in a loop, not 22 hand-written predicates.

### Companion change — symmetry for `m`

`m` (mouse) is a lonely leader; its geometric mirror `.` (KC_DOT) is free. Bind
`.` as a full mirror of `m`: `leader,.` → mouse layer, and `leader,.,{l,r,e}` →
`₺ ₽ €`, so the pair matches the Esc (`s`/`n`) and Ctrl+Esc (`w`/`h`) precedent.

### Removals

- Delete `modules/shofel/ucis_emoji/` — dead once UCIS is abandoned; nothing
  references it (not in `keymap.json`; only `TODO.md` mentions it).
- Replace the dead `keymap.c` marker with the real emoji block.
- Mark the TODO item done.

## Testing / verification boundary

- **TDD (test-first) on the generator** (pure Python): extend
  `tools/test_gen_compose.py` with red→green assertions for the emoji XCompose
  lines and `@`-prefix isolation. Run via `make test-compose`.
- Full `make test` (bisect + oneshot + compose) and `make build` (qmk compile
  cantor:shofel) must be green — that is the verification ceiling here.
- **Not verifiable by this change:** on-device rendering needs a firmware flash
  **and** the regenerated `~/.XCompose` loaded on the host. The repo artifact
  (`tools/XCompose.generated`, `tools/compose_cheatsheet.txt`) is regenerated;
  copying it to the live `~/.XCompose` is a host step, called out, not claimed.
