# Flashing the keyboard:
# make flash

.DEFAULT_GOAL := flash

GIT_ROOT := $(shell git rev-parse --show-cdup)
qmk_dir = ${GIT_ROOT}/qmk_firmware

keyboard = handwired/dactyl_manuform/5x6_5
keymap = KEYMAP
target = avrdude

keymap_path = ${qmk_dir}/keyboards/${keyboard}/keymaps/${keymap}/
build_cmd = sudo util/docker_build.sh ${keyboard}:${keymap}:${target}

keymap_clean:
	rm -rf ${keymap_path}

keymap_copy: keymap_clean
	mkdir ${keymap_path}
	cp ./* ${keymap_path}

qmk_flash:
	cd ${qmk_dir} && ${build_cmd}

flash:
	@make keymap_copy
	@make qmk_flash
	@make keymap_clean
