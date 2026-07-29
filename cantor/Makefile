# Flashing the keyboard:
# From repository root: make -C cantor flash
# Or directly: cd .. && qmk flash -kb cantor -km shofel

.DEFAULT_GOAL := flash

keyboard = cantor
keymap = shofel
repo_root = $(shell git rev-parse --show-toplevel)

# Flash using QMK CLI (must be run from repo root)
flash:
	@cd ${repo_root} && qmk flash -kb ${keyboard} -km ${keymap}

# Compile using QMK CLI (must be run from repo root)
compile:
	@cd ${repo_root} && qmk compile -kb ${keyboard} -km ${keymap}
