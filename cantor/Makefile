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

visual = visual
json = ${visual}/keymap.json
yaml = ${visual}/keymap.yaml
yaml_old = ${visual}/keymap.yaml.old
svg = ${visual}/keymap.svg
combos = ${visual}/combos.yml

c2json:
	qmk c2json -q keymap.c -kb ${keyboard} -km ${keymap} --no-cpp > ${json}

parse:
	@make c2json
	mv ${yaml} ${yaml_old}
	keymap parse -b ${yaml_old} -q ${json} > ${yaml}
	rm ${yaml_old}

parse_and_fix:
	@make c2json
	keymap parse -q ${json} > ${yaml}
	sd XX "''" ${yaml}
	# BOO home row mods
	sd 'BH A' "{t: 'A', h: 'Super'}" ${yaml}
	sd 'BH O' "{t: 'O', h: 'Alt'}" ${yaml}
	sd 'BH E' "{t: 'E', h: 'NAV'}" ${yaml}
	sd 'BH S' "{t: 'S', h: 'Ctrl'}" ${yaml}
	sd 'BH N' "{t: 'N', h: 'Ctrl'}" ${yaml}
	sd 'BH T' "{t: 'T', h: 'NUM'}" ${yaml}
	sd 'BH R' "{t: 'R', h: 'Alt'}" ${yaml}
	sd 'BH I' "{t: 'I', h: 'Super'}" ${yaml}
	# Ru home row mods
	sd 'RU ' '' ${yaml}
	sd 'RH F' "{t: 'F', h: 'Super'}" ${yaml}
	sd 'RH Y' "{t: 'Y', h: 'Alt'}" ${yaml}
	sd 'RH V' "{t: 'V', h: 'NAV'}" ${yaml}
	sd 'RH A' "{t: 'A', h: 'Ctrl'}" ${yaml}
	sd 'RH O' "{t: 'O', h: 'Ctrl'}" ${yaml}
	sd 'RH L' "{t: 'L', h: 'NUM'}" ${yaml}
	sd 'RH D' "{t: 'D', h: 'Alt'}" ${yaml}
	sd 'RH Z' "{t: 'Z', h: 'Super'}" ${yaml}
	#
	sd 'KK[ \n]MOUSE' '{h: MOUSE}' ${yaml}
	sd 'KK[ \n]SHIFT' 'Shift' ${yaml}
	sd 'KK[ \n]SYMBO' 'SYM' ${yaml}
	sd 'KK[ \n]ENTER' '{t: Enter, h: SYM}' ${yaml}
	sd 'KK[ \n]SPACE' '{t: Space, h: NAV}' ${yaml}
	sd 'KK[ \n]RU' '{t: RU, h: FSYS}' ${yaml}
	sd 'NOOP' "''" ${yaml}
	sd 'EM THNK' "🤔" ${yaml}
	cat ${combos} >> ${yaml}

draw:
	@make parse_and_fix
	keymap draw ${yaml} > ${svg}
	@make view

view:
	firefox ${svg}
