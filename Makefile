.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(shell realpath "$(lastword $(MAKEFILE_LIST))")))
ifeq ($(QMK_USERSPACE),)
    QMK_USERSPACE := $(shell pwd)
endif

QMK_FIRMWARE_ROOT = $(shell qmk config -ro user.qmk_home 2>/dev/null | cut -d= -f2 | sed -e 's@^None$$@@g')

# Convenience targets — build and flash the Cantor keymap.
.PHONY: build flash test test-bisect test-oneshot test-compose test-schemes gen-docs

build:
	qmk compile -kb cantor -km shofel

# Flash one half. The Cantor is a split: whichever half is on USB is the one
# flashed, so run this once per half, connected directly rather than over TRRS.
#
# Enter the bootloader with the boot combo — the two outer bottom keys on that
# half (boot_combo_left / boot_combo_right in keymap.c). A half with no firmware
# can't run the combo; hold BOOT0 while plugging in USB instead.
#
# Start this first and enter the bootloader after: it polls (FLASH_TIMEOUT)
# instead of needing the device up front, which used to mean racing the reset
# against a ~40s compile. Retries cover the STM32 occasionally enumerating with
# a broken descriptor set. A retry rewrites the whole image, so a half-written
# flash from an interrupted attempt is recovered rather than compounded.
FLASH_TIMEOUT ?= 120

flash: build
	@test -n "$(QMK_FIRMWARE_ROOT)" || { echo "qmk_firmware not configured: qmk config user.qmk_home" >&2; exit 1; }
	echo "Enter the bootloader on the half to flash — boot combo, or BOOT0 while plugging in. Waiting $(FLASH_TIMEOUT)s..."
	for i in $$(seq 1 $(FLASH_TIMEOUT)); do \
	  if dfu-util -l 2>/dev/null | grep -q 0483:df11; then \
	    sleep 1; \
	    for try in 1 2 3; do \
	      dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D $(QMK_FIRMWARE_ROOT)/cantor_shofel.bin && exit 0; \
	      if test $$try -lt 3; then echo "dfu-util attempt $$try failed — retrying in 2s" >&2; sleep 2; fi; \
	    done; \
	    echo "dfu-util failed 3x. Unplug/replug the Cantor, then run make flash again." >&2; \
	    exit 1; \
	  fi; \
	  sleep 1; \
	done; \
	echo "No DFU device (0483:df11) after $(FLASH_TIMEOUT)s — was the Cantor reset?" >&2; \
	exit 1

# All off-target host tests (pure logic; no QMK, no hardware).
test: test-bisect test-oneshot test-compose test-schemes

# Off-target unit test for bisect_geom.h (pure host math; no QMK, no hardware).
test-bisect:
	gcc -Wall -Wextra -Ilayouts/shofel/split_3x6_3/shofel -o /tmp/test_bisect_geom tools/test_bisect_geom.c -lm && /tmp/test_bisect_geom

# Off-target unit test for oneshot_fsm.h (the eager one-shot state machine).
test-oneshot:
	gcc -Wall -Wextra -Imodules/shofel/oneshot -o /tmp/test_oneshot_fsm tools/test_oneshot_fsm.c && /tmp/test_oneshot_fsm

test-compose:
	python3 tools/test_gen_compose.py

test-schemes:
	python3 tools/gen_layer_schemes.py --check

# Regenerate docs/reference.md + the in-LAYOUT scheme comments from keymap.c.
gen-docs:
	python3 tools/gen_layer_schemes.py --write

%:
	@test -n "$(QMK_FIRMWARE_ROOT)" || { echo "qmk_firmware not configured: qmk config user.qmk_home" >&2; exit 1; }
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)

