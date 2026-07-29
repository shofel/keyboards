.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(shell realpath "$(lastword $(MAKEFILE_LIST))")))
ifeq ($(QMK_USERSPACE),)
    QMK_USERSPACE := $(shell pwd)
endif

QMK_FIRMWARE_ROOT = $(shell qmk config -ro user.qmk_home | cut -d= -f2 | sed -e 's@^None$$@@g')
ifeq ($(QMK_FIRMWARE_ROOT),)
    $(error Cannot determine qmk_firmware location. `qmk config -ro user.qmk_home` is not set)
endif

# Convenience targets — build and flash the Cantor keymap.
.PHONY: build flash test test-bisect test-oneshot test-compose

build:
	qmk compile -kb cantor -km shofel

# Flash ONE half. The Cantor is a split: the half you plug into USB is the half
# that gets flashed, so run this twice — once per half, each connected directly
# (not over TRRS). Both halves must carry the same firmware.
#
# Getting a half into the bootloader (it is NOT "double-tap reset" — that is a
# Pro Micro / RP2040 idiom and does not apply to the Blackpill STM32F401):
#   - Normal case: the QMK boot combo, i.e. the two outer bottom keys on that
#     half (see boot_combo_left / boot_combo_right in keymap.c). Needs working
#     firmware.
#   - Blank/bricked half: hold BOOT0, plug in USB while holding, release after
#     ~2s. (Or: hold BOOT0, tap NRST, release BOOT0.) The STM32 ROM bootloader
#     lives in mask ROM and cannot be erased, so a half with no firmware is
#     always recoverable this way — the boot combo is not, since there is no
#     firmware left to run it.
#
# Run this first, then trigger the bootloader: it waits for the device to appear
# rather than requiring it to already be there. (dfu-util alone exits at once
# with "No DFU capable USB device available", which meant racing the reset
# against a ~40s compile.) Already-in-bootloader still works: the first poll
# hits straight away. Override the wait with FLASH_TIMEOUT=n.
#
# The 1s settle + 3 attempts are not paranoia: the STM32 bootloader sometimes
# enumerates with a half-baked descriptor set right after a double-tap, and
# dfu-util then dies with "Broken LANGID string descriptor" / get_status
# LIBUSB_ERROR_OVERFLOW (observed 2026-07-29). Retrying clears it, and a failed
# attempt writes nothing, so retrying is safe.
FLASH_TIMEOUT ?= 120

flash: build
	echo "Put ONE half in the bootloader (boot combo = 2 outer bottom keys; blank half = hold BOOT0 while plugging in USB)."
	echo "Waiting up to $(FLASH_TIMEOUT)s..."
	for i in $$(seq 1 $(FLASH_TIMEOUT)); do \
	  if dfu-util -l 2>/dev/null | grep -q 0483:df11; then \
	    sleep 1; \
	    for try in 1 2 3; do \
	      dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D $(QMK_FIRMWARE_ROOT)/cantor_shofel.bin && exit 0; \
	      echo "dfu-util attempt $$try failed — retrying in 2s" >&2; \
	      sleep 2; \
	    done; \
	    echo "dfu-util failed 3x. Unplug/replug the Cantor, then run make flash again." >&2; \
	    exit 1; \
	  fi; \
	  sleep 1; \
	done; \
	echo "No DFU device (0483:df11) after $(FLASH_TIMEOUT)s — was the Cantor reset?" >&2; \
	exit 1

# All off-target host tests (pure logic; no QMK, no hardware).
test: test-bisect test-oneshot test-compose

# Off-target unit test for bisect_geom.h (pure host math; no QMK, no hardware).
test-bisect:
	gcc -Wall -Wextra -Ilayouts/shofel/split_3x6_3/shofel -o /tmp/test_bisect_geom tools/test_bisect_geom.c -lm && /tmp/test_bisect_geom

# Off-target unit test for oneshot_fsm.h (the eager one-shot state machine).
test-oneshot:
	gcc -Wall -Wextra -Imodules/shofel/oneshot -o /tmp/test_oneshot_fsm tools/test_oneshot_fsm.c && /tmp/test_oneshot_fsm

test-compose:
	python3 tools/test_gen_compose.py

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)

