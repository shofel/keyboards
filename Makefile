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
.PHONY: build flash

build:
	qmk compile -kb cantor -km shofel

flash: build
	dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D $(QMK_FIRMWARE_ROOT)/cantor_shofel.bin

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)

