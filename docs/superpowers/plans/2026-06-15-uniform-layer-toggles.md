# Uniform Layer Toggles Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the bespoke Russian-layer state with one uniform state machine that drives every sticky toggle layer (Russian, F-keys, mouse, num/nav) the same way.

**Architecture:** A single `active_toggle` variable (one layer at a time, or none) plus a `toggle_suspend_depth` counter and an `applied_layer` tracker. `toggle_apply()` reconciles intent against what is physically on, touching only the one overlay it owns — so native QMK `OSL()` momentary layers and the base layer are never disturbed (no `layer_move`). Russian is the only layer that suspends while a mod is held.

**Tech Stack:** QMK firmware (C), Cantor keyboard, `shofel` keymap. Build via `make build` (`qmk compile -kb cantor -km shofel`).

**Testing note:** This keymap has no unit-test harness; keymap C logic is not unit-testable in isolation. Per-task verification is therefore a clean `make build` (compile success), plus a one-time on-device manual checklist in the final task. The whole refactor is one atomic code task because the new `toggle_*` functions and the old `ru_*` call sites must change together to keep the file compiling.

**Spec:** `docs/superpowers/specs/2026-06-15-uniform-layer-toggles-design.md`

---

## File Structure

All changes are confined to one file:

- Modify: `layouts/shofel/split_3x6_3/shofel/keymap.c`
  - Lines 55-99: Russian state block → uniform toggle state machine.
  - Lines 260-272: `oneshot_process_event` → call `toggle_suspend`/`toggle_resume`.
  - Lines 276-332: `leader_start_user`/`leader_end_user` → suspend/resume rename, per-layer enable seqs, retire `leader,e/j/_`, `leader,space` → reset.
  - Lines 360-366: `process_record_user` `KC_ESC` → `toggle_disable()`.

No changes to `config.h`, `rules.mk`, combos, key overrides, the keymaps themselves, or any module. `oneshot_cancel(void)` is already declared in `modules/shofel/oneshot/introspection.h` (included at the top of keymap.c), so `toggle_reset()` can call it.

---

## Task 1: Replace Russian state with uniform toggle state machine

**Files:**
- Modify: `layouts/shofel/split_3x6_3/shofel/keymap.c`

- [ ] **Step 1: Replace the Russian state block (lines 55-99)**

Find this block (the `/* Switch language */` comment through the end of `ru_exit`):

```c
/* Switch language */

/*
 * Russian state is two independent concerns:
 *   ru_enabled        — whether Russian is logically on
 *   ru_suspend_depth  — how many nested mod/leader holds are masking it
 * The layer is on iff enabled and nothing is suspending it. Keeping these
 * separate makes the counter impossible to desync from the on/off intent.
 */
static bool    ru_enabled       = false;
static uint8_t ru_suspend_depth = 0;

static void ru_apply(void) {
    if (ru_enabled && ru_suspend_depth == 0) {
        layer_on(L_RUSSIAN);
    } else {
        layer_off(L_RUSSIAN);
    }
}

void ru_suspend(void) {
    ru_suspend_depth += 1;
    ru_apply();
}

void ru_resume(void) {
    if (ru_suspend_depth > 0) {
        ru_suspend_depth -= 1;
    }
    ru_apply();
}

void ru_enter(void) {
    ru_enabled = true;
    ru_suspend_depth = 0;
    layer_move(L_BOO);
    ru_apply();
}

void ru_exit(void) {
    ru_enabled = false;
    ru_suspend_depth = 0;
    layer_move(L_BOO);
    ru_apply();
}
```

Replace it with:

```c
/* Sticky toggle layers */

/*
 * Russian, F-keys, mouse and num/nav are all driven the same way: exactly one
 * toggle layer is active at a time, enabled by its own leader sequence and
 * disabled by leader,space. State is two concerns:
 *   active_toggle        — which toggle layer the user wants on (or TOGGLE_NONE)
 *   toggle_suspend_depth — how many nested mod/leader holds are masking it
 * applied_layer is the overlay we physically turned on. The reconcile only ever
 * touches our own layer, so it never disturbs a native OSL momentary layer or
 * the base layer (no layer_move). Russian is the only layer that suspends while
 * a mod is held — Ctrl/Alt/Gui combos then fall through to the Latin base.
 */
#define TOGGLE_NONE 0xFF

static uint8_t active_toggle        = TOGGLE_NONE;
static uint8_t applied_layer        = TOGGLE_NONE;
static uint8_t toggle_suspend_depth = 0;

static void toggle_apply(void) {
    uint8_t want = active_toggle;
    if (active_toggle == L_RUSSIAN && toggle_suspend_depth > 0) {
        want = TOGGLE_NONE;
    }
    if (applied_layer == want) {
        return;
    }
    if (applied_layer != TOGGLE_NONE) {
        layer_off(applied_layer);
    }
    if (want != TOGGLE_NONE) {
        layer_on(want);
    }
    applied_layer = want;
}

static void toggle_enable(uint8_t layer) {
    active_toggle = layer;
    toggle_apply();
}

static void toggle_disable(void) {
    active_toggle = TOGGLE_NONE;
    toggle_suspend_depth = 0;
    toggle_apply();
}

static void toggle_reset(void) {
    oneshot_cancel();
    toggle_disable();
}

void toggle_suspend(void) {
    toggle_suspend_depth += 1;
    toggle_apply();
}

void toggle_resume(void) {
    if (toggle_suspend_depth > 0) {
        toggle_suspend_depth -= 1;
    }
    toggle_apply();
}
```

- [ ] **Step 2: Update `oneshot_process_event` (the Russian suspend/resume hook)**

Find:

```c
    switch (oneshot->state) {
      case os_down_unused: ru_suspend(); break;
      case os_down_used: break;
      case os_up_queued: break;
      case os_up_unqueued: ru_resume(); break;
    }
```

Replace with:

```c
    switch (oneshot->state) {
      case os_down_unused: toggle_suspend(); break;
      case os_down_used: break;
      case os_up_queued: break;
      case os_up_unqueued: toggle_resume(); break;
    }
```

- [ ] **Step 3: Update `leader_start_user`**

Find:

```c
void leader_start_user(void) {
  ru_suspend();
}
```

Replace with:

```c
void leader_start_user(void) {
  toggle_suspend();
}
```

- [ ] **Step 4: Update the Russian + resume part of `leader_end_user`**

Find:

```c
void leader_end_user(void) {
  ru_resume();

  /* Ru */
  if (leader_sequence_one_key(KC_R)) {
    ru_enter();
  }
  if (leader_sequence_one_key(KC_E)) {
    ru_exit();
  }
  if (leader_sequence_one_key(KC_L)) {
    set_unicode_input_mode(UNICODE_MODE_LINUX);
    ru_enter();
  }
  if (leader_sequence_one_key(KC_V)) {
    set_unicode_input_mode(UNICODE_MODE_VIM);
    ru_enter();
  }

  /* Layers */
  if (leader_sequence_one_key(KC_SPACE)) {
    layer_move(L_BOO);
  }
```

Replace with:

```c
void leader_end_user(void) {
  toggle_resume();

  /* Ru */
  if (leader_sequence_one_key(KC_R)) {
    toggle_enable(L_RUSSIAN);
  }
  if (leader_sequence_one_key(KC_L)) {
    set_unicode_input_mode(UNICODE_MODE_LINUX);
    toggle_enable(L_RUSSIAN);
  }
  if (leader_sequence_one_key(KC_V)) {
    set_unicode_input_mode(UNICODE_MODE_VIM);
    toggle_enable(L_RUSSIAN);
  }

  /* Disable any active toggle layer (one seq for all) */
  if (leader_sequence_one_key(KC_SPACE)) {
    toggle_reset();
  }
```

Note: the `KC_E` (`ru_exit`) sequence is removed — `leader,space` now covers disable.

- [ ] **Step 5: Retire the `leader,j` / `leader,_` exit sequences**

Find:

```c
  if (leader_sequence_one_key(KC_J)) {
    oneshot_cancel();
    ru_exit();
  }
  if (leader_sequence_one_key(KC_UNDS)) {
    oneshot_cancel();
    ru_exit();
  }
  if (leader_sequence_one_key(KC_F)) {
    layer_on(L_FKEYS_SYS);
  }
  if (leader_sequence_one_key(KC_M)) {
    layer_on(L_MOUSE);
  }
  if (leader_sequence_one_key(KC_T)) {
    layer_on(L_NUM_NAV);
  }
```

Replace with:

```c
  if (leader_sequence_one_key(KC_F)) {
    toggle_enable(L_FKEYS_SYS);
  }
  if (leader_sequence_one_key(KC_M)) {
    toggle_enable(L_MOUSE);
  }
  if (leader_sequence_one_key(KC_T)) {
    toggle_enable(L_NUM_NAV);
  }
```

Note: `leader,j` and `leader,_` are removed — their job (cancel oneshots + exit) is now `leader,space` → `toggle_reset()`.

- [ ] **Step 6: Update `process_record_user` for `KC_ESC`**

Find:

```c
    case KC_ESC:
      if (record->event.pressed) {
        ru_exit(); // In vim: restore En in normal mode
      }
      return true;
```

Replace with:

```c
    case KC_ESC:
      if (record->event.pressed) {
        toggle_disable(); // exits any active toggle; restores En in vim normal mode
      }
      return true;
```

- [ ] **Step 7: Verify no stale `ru_*` references remain**

Run: `grep -n "ru_enabled\|ru_suspend\|ru_resume\|ru_enter\|ru_exit\|ru_apply\|layer_move" layouts/shofel/split_3x6_3/shofel/keymap.c`
Expected: no output (every old symbol and every `layer_move` call is gone).

- [ ] **Step 8: Build**

Run: `make build`
Expected: compiles clean, ends with the `cantor_shofel.bin` size report and no errors/warnings about undefined symbols. (If a pipe is used, redirect to a file — per project memory, a piped `qmk compile` can swallow the failure exit code.)

- [ ] **Step 9: Commit**

```bash
git add layouts/shofel/split_3x6_3/shofel/keymap.c
git commit -m "layout: unify sticky layer toggles into one state machine"
```

---

## Task 2: On-device verification

**Files:** none (manual hardware testing).

This task is a human/on-device checklist — there is no automated substitute. Flash with `make flash` (double-tap reset to enter bootloader first), then confirm:

- [ ] **Step 1: Russian enable** — `leader,r` enables Russian; typing produces Cyrillic.
- [ ] **Step 2: Suspend under mods** — with Russian on, hold Ctrl (oneshot) and tap a key: it acts on the Latin base (e.g. Ctrl+C copies), then Russian returns on release. Shift still types Cyrillic capitals (Shift is not in the suspend hook).
- [ ] **Step 3: One-at-a-time** — with Russian on, `leader,f` switches to F-keys (Russian off); `leader,t` switches to num/nav; etc. Only one toggle layer is ever active.
- [ ] **Step 4: Disable (full reset)** — `leader,space` from any toggle returns to base, and any queued oneshot mod is cleared.
- [ ] **Step 5: Esc disables** — Esc (via the shift+space combo) from any active toggle returns to base.
- [ ] **Step 6: RU-resurrection bug is fixed** — `leader,r`, then `leader,space`, then tap+release Ctrl: Russian stays **off** (previously it came back).
- [ ] **Step 7: OSL still works** — with Russian on, fire the num/nav OSL combo (the `E+U` / `T+D` vertical combo): numbers overlay momentarily, then it reverts to Russian. The F-keys OSL combo (`B+Q`) behaves the same.
