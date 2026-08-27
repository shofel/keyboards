# Keylog — key-level typing instrument

Measures what the character corpus cannot.

`tools/typing_corpus.py` counts characters that *survived* into a prompt. It is
structurally blind to everything that happens on the way there: backspace,
modifiers, layer switches, combo usage, thumb load, and the type-then-correct
rate — several of which are plausibly among the most-pressed keys on this board
and none of which reach the text. The number-entry study stalled for want of
exactly these numbers.

## Counts, never sequences

This is a keylogger in mechanism and deliberately not one in effect. The state
is counters plus **one** slot of history (`last_key`, the minimum needed to
attribute a backspace to the key it deleted). No two-key sequence can be
recovered from it, and `tools/test_keylog_stats.c` pins that by asserting two
different typing orders leave *byte-identical* state.

Nothing is written to flash and nothing leaves the board unless you ask for it
with the dump command. The dump is a table of totals.

## Use

    leader,s,d    dump the counters to the HID console
    leader,s,c    clear them (start a fresh measurement window)

Read them with the console:

    qmk console

Output is greppable:

    keylog total=48213 backspace=3120 combo=884
    keylog layer 0 41022
    keylog layer 3 5170
    keylog key 15 r2 c3 n=1902 corrected=61 ppm=32071
    keylog end

`ppm` is the parts-per-million of that key's presses that were immediately
undone by backspace — the type-then-correct rate, per key. A key with a high
rate is a placement problem, and it is invisible in any text corpus because the
evidence was deleted before it got there.

## Known gaps

- **No per-combo breakdown.** `combo` is an exact total, not a table. QMK
  exposes no fire-time hook carrying the combo index: `combo_should_trigger` is
  called once per candidate key of every candidate combo on every press, so
  counting there overcounts wildly, and `IS_COMBOEVENT` is exact but anonymous.
  An exact total beats a plausible-looking wrong breakdown; the breakdown needs
  an upstream hook and is left undone rather than faked.
- **Counters saturate at 65535** rather than wrapping. Wrapping would turn the
  most-pressed key into the least-pressed one. Clear between windows.
- **Right half.** Only the half on USB reports; this board flashes the left.
