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
# Flash: put the keyboard in bootloader mode (double-tap reset) before running.
.PHONY: build flash test test-bisect test-oneshot test-compose

build:
	qmk compile -kb cantor -km shofel

flash: build
	dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D $(QMK_FIRMWARE_ROOT)/cantor_shofel.bin

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

