# Flashing the keyboard:
# make flash

.DEFAULT_GOAL := flash

keyboard = cantor
keymap = keymap
target = flash

keymap_src = .
keymap_dest = ${QMK_FIRMWARE}/keyboards/${keyboard}/keymaps/${keymap}/

clean:
	rm -rf ${keymap_dest}

copy: clean
	cp -r ${keymap_src} ${keymap_dest}

qmk_flash:
	qmk flash -kb ${keyboard} -km ${keymap}

flash:
	@make copy
	@make qmk_flash || true
	@make clean
